#include "ble_client.h"

// strings
const char *devicename = DEVICENAME;

// status flags
bool do_connect = false;
bool ble_connected = false;
bool capturing = false;
bool initial_send_done = false; // Flag to indicate if the initial send has occurred

// counter
int count_ble_scans = 0;

// BLE
static NimBLEUUID serviceUUID("ffe0"); // The remote service we wish to connect to.
static NimBLEUUID charUUID("ffe1");    // The characteristic of the remote service we are interested in.
NimBLEAdvertisedDevice *myDevice;
NimBLEScan *pBLEScan;
NimBLEClient *pClient;
NimBLERemoteCharacteristic *pRemoteCharacteristic;

// messages
byte getdeviceInfo[20] = {0xaa, 0x55, 0x90, 0xeb, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11}; // Device Infos
byte getInfo[20] = {0xaa, 0x55, 0x90, 0xeb, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};       // Cell Data

// Buffer
std::mutex bufferMutex;
uint8_t ble_buffer[BUFFER_SIZE];
int ble_buffer_index = 0;
const uint8_t start_sequence[4] = {0x55, 0xaa, 0xeb, 0x90};
const uint8_t pos_of_FrameType = 4;       // position of FrameType in the message
boolean all_notifications_blocked = true; // Flag to track if all notifications are currently blocked

// Time variables
unsigned long last_received_notification = 0;
unsigned long last_sending_time = 0;
unsigned long last_ble_connection_attempt_time = 0;
unsigned long last_ble_scan_time = 0;
unsigned long last_data_received_time = 0;
unsigned long last_rssi_time = 0;

#ifdef DUALCORE
// Define the queue handle
QueueHandle_t bleQueue;
#endif

// forks into message parser by message type
void parser(void *message)
{
    uint8_t *msg = static_cast<uint8_t *>(message);
    uint8_t type = msg[4]; // 4. Byte decides the message type

    switch (type)
    {
    case 0x01:
        readConfigDataRecord(message, devicename);
        break;
    case 0x02:
        readCellDataRecord(message, devicename);
        break;
    case 0x03:
        readDeviceDataRecord(message, devicename);
        break;
    default:
        DEBUG_PRINTLN("Unbekannter Typ in message[3]!");
        break;
    }
}

// Convert BLE disconnect reason code to readable text
String getDisconnectReasonText(int reason)
{
    switch (reason)
    {
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

class MyClientCallback : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient *pclient)
    {
        DEBUG_PRINTLN("BLE Connected");
        ble_connected = true;
    }

    void onDisconnect(NimBLEClient *pclient, int reason)
    {
        ble_connected = false;
        String reasonText = getDisconnectReasonText(reason);
        DEBUG_PRINTLN("BLE Disconnected. Reason: " + reasonText + ". Restarting...");
        ESP.restart();
    }
};

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
    void onResult(NimBLEAdvertisedDevice *advertisedDevice)
    {

        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID(serviceUUID)) && advertisedDevice->getName() == devicename)
        {
            String serverMsg = "Found our server " + String(devicename);
            DEBUG_PRINTLN(serverMsg);
            pBLEScan->stop();
            myDevice = advertisedDevice;
            do_connect = true;
            // pBLEScan->clearResults(); // that seems to kill the pointer to the device
        } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks

// Static callback instances to avoid memory leaks
static MyClientCallback clientCallback;
static MyAdvertisedDeviceCallbacks deviceCallback;

bool CRC_Check(uint8_t *data, size_t length)
{
    if (length < 2)
        return false; // Not enough data for CRC

    uint16_t calculated_crc = 0;
    for (size_t i = 0; i < length - 1; i++)
    {
        calculated_crc += data[i];
    }
    calculated_crc &= 0xFF; // Keep only the lowest byte

    uint8_t received_crc = data[length - 1];
    return calculated_crc == received_crc;
}

void notifyCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (all_notifications_blocked)
    {
        return; // Ignore all notifications if the flag is set
    }
    // DEBUG_PRINTLN("Notification received. Length: " + String(length) + " bytes");
    // DEBUG_PRINT("Data: ");
    // for (size_t i = 0; i < length; i++) {
    //     if (pData[i] < 16) {
    //         DEBUG_PRINT("0"); // Leading zero for single-digit hex values
    //     }
    //     DEBUG_PRINT(String(pData[i], HEX) + " ");
    // }
    // DEBUG_PRINTLN("");

    last_received_notification = millis(); // Zeitstempel aktualisieren

    if (!capturing && length >= sizeof(start_sequence) && memcmp(pData, start_sequence, sizeof(start_sequence)) == 0)
    {
        // DEBUG_PRINTLN("Start sequence detected! Message type: " + String(pData[pos_of_FrameType], HEX));
        memcpy(ble_buffer, pData, length); // copy notification to buffer
        ble_buffer_index = length;
        capturing = true; // from now on, data will be stored in buffer
    }
    else if (capturing)
    {
        // write byte to buffer
        size_t bytes_to_copy = std::min(length, static_cast<size_t>(BUFFER_SIZE - ble_buffer_index));
        memcpy(ble_buffer + ble_buffer_index, pData, bytes_to_copy);
        ble_buffer_index += bytes_to_copy;

        // if 300 bytes received and CRC_Check OK call parser
        if (ble_buffer_index >= BUFFER_SIZE && CRC_Check(ble_buffer, BUFFER_SIZE))
        {
            std::vector<uint8_t> message(ble_buffer, ble_buffer + BUFFER_SIZE);
            ble_buffer_index = 0;
            capturing = false; // waiting for next start sequence
            last_data_received_time = millis();

            // .. and call parser or send message to queue for parser task

#ifdef DUALCORE
            // Add message to queue
            if (xQueueSend(bleQueue, message.data(), 0) != pdTRUE)
            {
                DEBUG_PRINTLN("Failed to send message to queue");
            }
#else
            parser(static_cast<void *>(message.data()));
#endif
        }
    }
}

bool connectToBLEServer()
{

    pClient->setClientCallbacks(&clientCallback);

    // Connect to the remote BLE Server.
    if (pClient->connect(myDevice))
    {

        String macAddr = String(myDevice->getAddress().toString().c_str());
        String rssiVal = String(myDevice->getRSSI());
        setState("ble_device_mac", macAddr, true);
        setState("ble_device_rssi", rssiVal, true);

        DEBUG_PRINTLN(" - Connected to server");

        // Obtain a reference to the service we are after in the remote BLE server.
        NimBLERemoteService *pRemoteService = pClient->getService(NimBLEUUID(serviceUUID));
        if (pRemoteService == nullptr)
        {
            String serviceMsg = "Failed to find our service UUID: " + String(serviceUUID.toString().c_str());
            DEBUG_PRINTLN(serviceMsg);
            pClient->disconnect();
            return false;
        }
        DEBUG_PRINTLN(" - Found our service");

        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID(charUUID));
        if (pRemoteCharacteristic == nullptr)
        {
            String charMsg = "Failed to find our characteristic UUID: " + String(charUUID.toString().c_str());
            DEBUG_PRINTLN(charMsg);
            pClient->disconnect();
            return false;
        }
        DEBUG_PRINTLN(" - Found our characteristic");

        // Set the notification callback
        if (pRemoteCharacteristic->canNotify())
        {
            pRemoteCharacteristic->subscribe(true, notifyCallback);
            DEBUG_PRINTLN("Subscribed to notifications");
        }

        // Sending getdevice info
        pRemoteCharacteristic->writeValue(getdeviceInfo, 20);
        initial_send_done = false;
        last_sending_time = millis();
        DEBUG_PRINTLN("Sending \"getdeviceInfo\" to the BLE device...");

        setState("ble_connection", "connected", true);

#ifdef USELED
        // Send LED_BLINK_SLOW state to the LED task
        set_led(LedState::LED_BLINK_SLOW);
#endif

        return true;
    }
    else
    {
        DEBUG_PRINTLN(" - Failed to connect to server");
        setState("ble_connection", "failed", true);
        return false;
    }
}

void ble_loop()
{
    // BLE not connected
    if (!ble_connected && !do_connect && (millis() - last_ble_scan_time) > SCAN_REPEAT_TIME)
    {
        String scanMsg = "BLE -> Scanning ... " + String(count_ble_scans);
        DEBUG_PRINTLN(scanMsg);

        setState("ble_connection", String("scanning " + String(count_ble_scans)).c_str(), true);

        last_ble_scan_time = millis();
        if (pBLEScan != nullptr)
        {
            pBLEScan->start(5, false);

#ifdef USELED
            // Send LED_BLINK_FAST state to the LED task
            set_led(LedState::LED_BLINK_FAST);
#endif
        }
        else
        {
            DEBUG_PRINTLN("Error: pBLEScan is NULL");
        }
        count_ble_scans++;
    }

    // BLE connected but no new data since 60 seconds
    if (!do_connect && ble_connected && (millis() >= (last_data_received_time + TIMEOUT_NO_DATA)) && last_data_received_time != 0)
    {
        ble_connected = false;
        delay(200);
        setState("ble_connection", "terminated", true);
        DEBUG_PRINTLN("BLE-Disconnect/terminated");
        last_data_received_time = millis();
        if (pClient != nullptr)
        {
            pClient->disconnect();
        }
        else
        {
            DEBUG_PRINTLN("Error: pClient is NULL");
        }
    }

    // ESP restart after REBOOT_AFTER_BLE_RETRY BLE Scans without success
    if (count_ble_scans > REBOOT_AFTER_BLE_RETRY)
    {
        setState("ble_connection", "rebooting", true);
        DEBUG_PRINTLN("BLE not receiving new Data from BMS... and no BLE reconnection possible, Reboot ESP...");
        ESP.restart();
    }

    // Attempt to connect if a device is found
    if (do_connect)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - last_ble_connection_attempt_time >= BLE_RECONNECT)
        {
            last_ble_connection_attempt_time = currentMillis;
            if (connectToBLEServer())
            {
                DEBUG_PRINTLN("We are now connected to the BLE Server.");
                do_connect = false;
            }
            else
            {
                DEBUG_PRINTLN("Failed to connect to the BLE Server.");
            }
        }
    }

    if (ble_connected)
    {
        // request cell data 5 seconds after request for device data

        if (!initial_send_done)
        {
            // Initial send after 5 seconds
            if ((millis() - last_sending_time) >= INITIAL_SEND_INTERVAL)
            {
                DEBUG_PRINTLN("Start the show, unblock notifications ...");
                all_notifications_blocked = false; // Unblock notifications
                DEBUG_PRINTLN("Send getInfo (initial)");
                pRemoteCharacteristic->writeValue(getInfo, 20);
                last_sending_time = millis(); // Update the last sending time to the current time
                initial_send_done = true;     // Set the flag to indicate the initial send is done
            }
        }
        else
        {
            // Subsequent sends every hour
            if ((millis() - last_sending_time) >= REPEAT_SEND_INTERVAL)
            {
                DEBUG_PRINTLN("Send getInfo (hourly)");
                pRemoteCharacteristic->writeValue(getInfo, 20);
                last_sending_time = millis(); // Update the last sending time to the current time
            }
        }

        if (last_rssi_time == 0 || (millis() - last_rssi_time) >= RSSI_INTERVAL)
        {
            last_rssi_time = millis();
            setState("ble_device_rssi", String(myDevice->getRSSI()).c_str(), true);
        }
    }
}

#ifdef DUALCORE
// Define the BLE client task
void bleClientTask(void *pvParameters)
{
    NimBLEDevice::init("");
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(&clientCallback);
    DEBUG_PRINTLN("BLE client started");

    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(&deviceCallback);
    pBLEScan->setInterval(50);
    pBLEScan->setWindow(40);
    pBLEScan->setActiveScan(true);

    String scanForMsg = "Scan for our Server " + String(devicename);
    DEBUG_PRINTLN(scanForMsg);

    // Keep the task running
    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Define the parser task
void parserTask(void *pvParameters)
{
    uint8_t message[BUFFER_SIZE];

    while (true)
    {
        // Receive data from the queue
        if (xQueueReceive(bleQueue, &message, 0) == pdTRUE)
        {
            // Call the parser function
            parser(message);
        }
        vTaskDelay(25 / portTICK_PERIOD_MS); // Small delay to prevent task starvation
    }
}
#endif

void ble_setup()
{

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

    NimBLEDevice::init("");
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(&clientCallback);
    DEBUG_PRINTLN("BLE client started");

    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(&deviceCallback);
    pBLEScan->setInterval(50);
    pBLEScan->setWindow(40);
    pBLEScan->setActiveScan(true);

    DEBUG_PRINTLN("Scan for our Server " + String(devicename));

#endif
}