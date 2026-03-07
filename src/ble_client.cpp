#include "ble_client.h"

// strings
const char *devicename = DEVICENAME;

// status flags
bool ble_connected = false; // Flag to track BLE connection status
bool capturing = false; // Flag to indicate if we are currently capturing data after detecting the start sequence
bool initial_send_done = false; // Flag to indicate if the initial send has occurred

// BLE
static NimBLEUUID serviceUUID("ffe0"); // The remote service we wish to connect to.
static NimBLEUUID charUUID("ffe1");    // The characteristic of the remote service we are interested in.
static const NimBLEAdvertisedDevice *myDevice;
static bool doConnect = false;
static uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
static NimBLERemoteService *pRemoteService = nullptr;
static NimBLERemoteCharacteristic *pRemoteCharacteristic = nullptr;

// messages
byte getdeviceInfo[20] = {0xaa, 0x55, 0x90, 0xeb, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11}; // Device Infos
byte getInfo[20] = {0xaa, 0x55, 0x90, 0xeb, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};       // Cell Data

// Buffer
std::mutex bufferMutex;
uint8_t ble_buffer[BUFFER_SIZE];
int ble_buffer_index = 0;
const uint8_t start_sequence[4] = {0x55, 0xaa, 0xeb, 0x90}; // Start sequence to detect the beginning of a message
const uint8_t pos_of_FrameType = 4;       // position of FrameType in the message
boolean all_notifications_blocked = true; // Flag to track if all notifications are currently blocked

// Time variables
unsigned long last_sending_time = 0; // Timestamp of the last time we sent a message to the BLE device
unsigned long last_rssi_time = 0;

#ifdef DUALCORE
// Define the queue handle
QueueHandle_t bleQueue;
#endif

// forks into message parser by message type
void parser(void *message) {
    uint8_t *msg = static_cast<uint8_t *>(message);
    uint8_t type = msg[4]; // 4. Byte decides the message type

    switch (type) {
    case 0x01:
        readConfigInfoRecord(message, devicename);
        break;
    case 0x02:
        readCellDataRecord(message, devicename);
        break;
    case 0x03:
        readDeviceInfoRecord(message, devicename);
        break;
    default:
        DEBUG_PRINTLN("Unbekannter Typ in message[4]!");
        break;
    }
}

// Convert BLE disconnect reason code to readable text
String getDisconnectReasonText(int reason) {
    switch (reason) {
    case 0x08:
        return "Connection Timeout (0x08)";
    case 0x13:
        return "Remote User Terminated Connection (0x13)";
    case 0x14:
        return "Remote Device Terminated due to Low Resources (0x14)";
    case 0x15:
        return "Remote Device Terminated due to Power Off (0x15)";
    case 0x16:
        return "Connection Terminated by Local Host (0x16)";
    case 0x22:
        return "LMP Response Timeout (0x22)";
    case 0x3D:
        return "Connection Failed to be Established (0x3D)";
    case 0x3E:
        return "LMP Response Timeout (0x3E)";
    default:
        return "Unknown Reason (" + String(reason, HEX) + ")";
    }
}

class MyClientCallback : public NimBLEClientCallbacks {
    
    void onConnect(NimBLEClient *pClient) override { DEBUG_PRINTF("BLE Connected\n"); }

    void onDisconnect(NimBLEClient *pClient, int reason) override {
        DEBUG_PRINTF("%s Disconnected, reason = %d - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
        String reasonText = getDisconnectReasonText(reason);
        DEBUG_PRINTLN("BLE Disconnected. Reason: " + reasonText);
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} clientCallbacks;

/** Define a class to handle the callbacks when scan events are received */
class ScanCallbacks : public NimBLEScanCallbacks {
    
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
        
        DEBUG_PRINTF("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
        if (advertisedDevice->isAdvertisingService(serviceUUID) && advertisedDevice->getName() == devicename) {
            DEBUG_PRINTLN("Found our server \"" + String(devicename) + "\"");
            /** stop scan before connecting */
            NimBLEDevice::getScan()->stop();
            /** Save the device reference in a global for the client to use*/
            myDevice = advertisedDevice;
            /** Ready to connect now */
            doConnect = true;
        }
    }

    /** Callback to process the results of the completed scan or restart it */
    void onScanEnd(const NimBLEScanResults &results, int reason) override {
        DEBUG_PRINTF("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} scanCallbacks;

bool CRC_Check(uint8_t *data, size_t length) {
    if (length < 2)
        return false; // Not enough data for CRC

    uint16_t calculated_crc = 0;
    for (size_t i = 0; i < length - 1; i++){
        calculated_crc += data[i];
    }
    calculated_crc &= 0xFF; // Keep only the lowest byte

    uint8_t received_crc = data[length - 1];
    return calculated_crc == received_crc;
}

void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
    
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (all_notifications_blocked) return; // Ignore all notifications if the flag is set
    
    //DEBUG_PRINTLN("Notification received. Length: " + String(length) + " bytes");
    //DEBUG_PRINT("Data: ");
    //for (size_t i = 0; i < length; i++) {
    //    if (pData[i] < 16) {
    //        DEBUG_PRINT("0"); // Leading zero for single-digit hex values
    //    }
    //    DEBUG_PRINT(String(pData[i], HEX) + " ");
    //}
    //DEBUG_PRINTLN("");

    if (!capturing && length >= sizeof(start_sequence) && memcmp(pData, start_sequence, sizeof(start_sequence)) == 0) {
        
        memcpy(ble_buffer, pData, length); // copy notification to buffer
        ble_buffer_index = length;
        capturing = true; // from now on, data will be stored in buffer
    }
    else if (capturing) {
        
        // copy pData to end of buffer, but only if it doesn't exceed the buffer size
        size_t bytes_to_copy = std::min(length, static_cast<size_t>(BUFFER_SIZE - ble_buffer_index));
        memcpy(ble_buffer + ble_buffer_index, pData, bytes_to_copy);
        ble_buffer_index += bytes_to_copy;

        // if 300 bytes received and CRC_Check OK call parser
        if (ble_buffer_index >= BUFFER_SIZE && CRC_Check(ble_buffer, BUFFER_SIZE)){
            std::vector<uint8_t> message(ble_buffer, ble_buffer + BUFFER_SIZE);
            ble_buffer_index = 0;
            capturing = false; // waiting for next start sequence

            // .. and call parser or send message to queue for parser task

#ifdef DUALCORE
            // Add message to queue
            if (xQueueSend(bleQueue, message.data(), 0) != pdTRUE) DEBUG_PRINTLN("Failed to send message to queue");
#else
            parser(static_cast<void *>(message.data()));
#endif
        }
    }
}

bool connectToBLEServer() {
    NimBLEClient *pClient = nullptr;
    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount()) {
        /**
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(myDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(myDevice, false)) {
                DEBUG_PRINTF("Reconnect failed\n");
                return false;
            }
            DEBUG_PRINTF("Reconnected client\n");
        } else {
            /**
             *  We don't already have a client that knows this device,
             *  check for a client that is disconnected that we can use.
             */
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient) {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
            DEBUG_PRINTF("Max clients reached - no more connections available\n");
            return false;
        }

        pClient = NimBLEDevice::createClient();
        DEBUG_PRINTF("New client created\n");
        pClient->setClientCallbacks(&clientCallbacks, false);
        pClient->setConnectionParams(12, 12, 0, 150);
        /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
        pClient->setConnectTimeout(5 * 1000);
        if (!pClient->connect(myDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            DEBUG_PRINTF("Failed to connect, deleted client\n");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(myDevice)) {
            DEBUG_PRINTF("Failed to connect\n");
            return false;
        }
    }

    DEBUG_PRINTF("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
    String macAddr = String(myDevice->getAddress().toString().c_str());
    String rssiVal = String(myDevice->getRSSI());
    setState("ble_device_mac", macAddr, true);
    setState("ble_device_rssi", rssiVal, true);

    // Obtain a reference to the service we are after in the remote BLE server.
    pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService) {
        DEBUG_PRINTLN(" - Found our service");
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
            String charMsg = "Failed to find our characteristic UUID: " + String(charUUID.toString().c_str());
            DEBUG_PRINTLN(charMsg);
            pClient->disconnect();
            return false;
        }
        DEBUG_PRINTLN(" - Found our characteristic");
        // Set the notification callback
        if (pRemoteCharacteristic->canNotify()) {
            if (!pRemoteCharacteristic->subscribe(true, notifyCB)) {
                DEBUG_PRINTLN("Failed to subscribe to notifications");
                pClient->disconnect();
                return false;
            }
            DEBUG_PRINTLN("Subscribed to notifications");
        }
    } else {
        DEBUG_PRINTLN("Failed to find our service UUID: " + String(serviceUUID.toString().c_str()));
        pClient->disconnect();
    }
    if (pRemoteCharacteristic->canWrite()) {
        // Sending getdevice info
        if (pRemoteCharacteristic->writeValue(getdeviceInfo, 20)) {
            DEBUG_PRINTLN("Sent \"getdeviceInfo\" to the BLE device");
            initial_send_done = false;    // Reset the initial send flag for the new connection
            last_sending_time = millis(); // Update the last sending time to the current time
        } else {
            DEBUG_PRINTLN("Failed to send \"getdeviceInfo\" to the BLE device");
            pClient->disconnect();
            return false;
        }
    }
    setState("ble_connection", "connected", true);
    ble_connected = true;

#ifdef USELED
    // Send LED_BLINK_SLOW state to the LED task
    set_led(LedState::LED_BLINK_SLOW);
#endif

    return true;
}

void ble_loop() {
    /** Loop here until we find a device we want to connect to */
    delay(10);
    if (doConnect) {
        doConnect = false;
        if (connectToBLEServer()) {
            DEBUG_PRINTLN("We are now connected to the BLE Server.");
        } else {
            DEBUG_PRINTLN("Failed to connect to the BLE Server.");
        }
    }

    if (ble_connected) {
        // request cell data 5 seconds after request for device data
        if (!initial_send_done) {
            // Initial send after INITIAL_SEND_INTERVAL
            if ((millis() - last_sending_time) >= INITIAL_SEND_INTERVAL) {
                DEBUG_PRINTLN("Start the show, unblock notifications ...");
                all_notifications_blocked = false; // Unblock notifications
                DEBUG_PRINTLN("Send getInfo (initial)");
                if (pRemoteCharacteristic->writeValue(getInfo, 20)) {
                    DEBUG_PRINTLN("Sent \"getInfo\" to the BLE device");
                } else {
                    DEBUG_PRINTLN("Failed to send \"getInfo\" to the BLE device");
                }
                last_sending_time = millis(); // Update the last sending time to the current time
                initial_send_done = true;     // Set the flag to indicate the initial send is done
            }
        } else {
            // Subsequent sends every hour
            if ((millis() - last_sending_time) >= REPEAT_SEND_INTERVAL) {
                DEBUG_PRINTLN("Send getInfo (hourly)");
                pRemoteCharacteristic->writeValue(getInfo, 20);
                last_sending_time = millis(); // Update the last sending time to the current time
            }
        }

        if (last_rssi_time == 0 || (millis() - last_rssi_time) >= BLE_RSSI_INTERVAL) {
            last_rssi_time = millis();
            setState("ble_device_rssi", String(myDevice->getRSSI()).c_str(), true);
        }
    }
}

#ifdef DUALCORE

// Define the BLE client task
void bleClientTask(void *pvParameters) {
    
    DEBUG_PRINTLN("Starting NimBLE Client\n");
    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");
    NimBLEDevice::setPower(ESP_PWR_LVL_P6);            // +6dbm
    NimBLEScan *pBLEScan = NimBLEDevice::getScan();    // Create a new scan
    pBLEScan->setScanCallbacks(&scanCallbacks, false); // Set the callback handlers to be called when we receive a result and when the scan is complete.
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(100);
    pBLEScan->setActiveScan(true);

    /** Start scanning for advertisers */
    pBLEScan->start(scanTimeMs);

    DEBUG_PRINTLN("Scan for our Server \"" + String(devicename) + "\"");

    // Keep the task running
    while (true) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// Define the parser task
void parserTask(void *pvParameters) {
    uint8_t messageFromQueue[BUFFER_SIZE];

    while (true) {
        // Receive data from the queue
        if (xQueueReceive(bleQueue, &messageFromQueue, 0) == pdTRUE) {
            // Call the parser function
            parser(messageFromQueue);
        }
        vTaskDelay(25 / portTICK_PERIOD_MS); // Small delay to prevent task starvation
    }
}
#endif

void ble_setup() {

#ifdef DUALCORE
    // Create the queue
    bleQueue = xQueueCreate(20, sizeof(uint8_t[BUFFER_SIZE]));
    DEBUG_PRINTLN("BLE queue created");

    // Create the BLE client task on core 1
    xTaskCreate(bleClientTask, "BLE Client Task", 8192, NULL, 1, NULL);
    DEBUG_PRINTLN("BLE Client Task created");

    // Create the parser task on core 0
    xTaskCreate(parserTask, "Parser Task", 8192, NULL, 1, NULL);
    DEBUG_PRINTLN("Parser Task created");

#else

    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");
    NimBLEDevice::setPower(ESP_PWR_LVL_P6);            // +6dbm
    NimBLEScan *pBLEScan = NimBLEDevice::getScan();    // Create a new scan
    pBLEScan->setScanCallbacks(&scanCallbacks, false); // Set the callback handlers to be called when we receive a result and when the scan is complete.

    DEBUG_PRINTLN("BLE client started");

    pBLEScan->setInterval(100);
    pBLEScan->setWindow(100);
    pBLEScan->setActiveScan(true);

    /** Start scanning for advertisers */
    pBLEScan->start(scanTimeMs);

        DEBUG_PRINTLN("Scan for our Server \"" + String(devicename) + "\"");

#endif
}