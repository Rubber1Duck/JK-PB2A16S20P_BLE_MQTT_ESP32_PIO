#include "ble_client.h"

// strings
const char *devicename = DEVICENAME;

// status flags
boolean ble_connected = false; // Flag to track BLE connection status
boolean capturing = false; // Flag to indicate if we are currently capturing data after detecting the start sequence
boolean DI_send = false; // Flag to indicate if the initial Device Info send has been done
boolean CI_send = false; // Flag to indicate if the initial Config Info send has been done
boolean CD_running = false; // Flag to indicate if we are currently receiving cell data frames


// BLE
static NimBLEUUID serviceUUID("ffe0"); // The remote service we wish to connect to.
static NimBLEUUID charUUID("ffe1");    // The characteristic of the remote service we are interested in.
static const NimBLEAdvertisedDevice *myDevice;
static boolean doConnect = false;
static uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
static NimBLERemoteService*        pSvc = nullptr;
static NimBLERemoteCharacteristic* pChr = nullptr;
static NimBLEClient *pClient = nullptr;

// messages
const byte getDeviceInfo[20] = {0xaa, 0x55, 0x90, 0xeb, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11}; // Device Info
const byte getConfigInfo[20] = {0xaa, 0x55, 0x90, 0xeb, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10}; // Config Info
const char *getDeviceInfo_str = "AA:55:90:EB:97:00:00:00:00:00:00:00:00:00:00:00:00:00:00:11"; // hex string representation of getDeviceInfo
const char *getConfigInfo_str = "AA:55:90:EB:96:00:00:00:00:00:00:00:00:00:00:00:00:00:00:10"; // hex string representation of getConfigInfo

// Buffer
std::mutex bufferMutex;
uint8_t ble_buffer[BUFFER_SIZE];
uint16_t ble_buffer_index = 0;
const uint8_t start_sequence[4] = {0x55, 0xaa, 0xeb, 0x90}; // Start sequence to detect the beginning of a message
const uint8_t pos_of_FrameType = 4;       // position of FrameType in the message
const uint8_t pos_of_FrameCounter = 5;    // position of FrameCounter in the message
boolean all_notifications_blocked = true; // Flag to track if all notifications are currently blocked

// Time variables
time_t last_rssi_time = 0; // Variable to save the last time RSSI was checked
time_t save_millis = 0; // Variable to save the last time millis() was called
time_t time_CI_sent = 0; // Variable to save the time when the Config Info request was sent
time_t lastRcvdCDTime = 0; // Variable to save the last time a cell data frame was received

#ifdef DUALCORE
// Define the queue handle
QueueHandle_t bleQueue;
#endif

// forks into message parser by message type
void parser(void *message) {
    uint8_t *msg = static_cast<uint8_t *>(message);
    uint8_t type = msg[pos_of_FrameType]; // 4. Byte decides the frame type
    uint8_t frameCounter = msg[pos_of_FrameCounter]; // 5. Byte is the frame counter

    switch (type) {
    case 0x01:
        DEBUG_PRINTF("Received Config Info. Frame Counter: %d\n", frameCounter);
        readConfigInfoRecord(message, devicename);
        break;
    
    case 0x02:
        lastRcvdCDTime = millis();
        if (!CD_running) {
            CD_running = true; // Set the flag to indicate that we are now receiving cell data frames
            DEBUG_PRINTLN("Started receiving cell data frames.");
        }
        readCellDataRecord(message, devicename);
        break;
    
    case 0x03:
        DEBUG_PRINTF("Received Device Info. Frame Counter: %d\n", frameCounter);
        readDeviceInfoRecord(message, devicename);
        break;
    
    default:
        DEBUG_PRINTLN("Unbekannter Typ in message[4]!");
        break;
    }
}

// Convert BLE disconnect reason code to readable text
const char *getDisconnectReasonText(int reason) {
    switch (reason) {
        case 513:
            return "(0x01): BLE_ERR_UNKNOWN_HCI_CMD – Unbekannter HCI-Befehl.";
        case 514:
            return "(0x02): BLE_ERR_UNK_CONN_ID – Unbekannte Verbindungskennung.";
        case 515:
            return "(0x03): BLE_ERR_HW_FAIL – Hardware-Fehler im Bluetooth-Chip.";
        case 517:
            return "(0x05): BLE_ERR_AUTH_FAIL – Authentifizierungsfehler (falscher PIN/Passkey).";
        case 518:
            return "(0x06): BLE_ERR_PINKEY_MISSING – PIN oder Verschlüsselungs-Key fehlt (Pairing verloren).";
        case 519:
            return "(0x07): BLE_ERR_MEM_CAPACITY – Speicher des Bluetooth-Controllers ist voll.";
        case 520:
            return "(0x08): BLE_ERR_CONN_SPVN_TMO – Supervision Timeout (Verbindung verloren).";
        case 521:
            return "(0x09): BLE_ERR_CONN_LIMIT – Verbindungslimit des Geräts ist erreicht.";
        case 522:
            return "(0x0A): BLE_ERR_SYNCH_CONN_LIMIT – Limit für synchrone Verbindungen erreicht.";
        case 523:
            return "(0x0B): BLE_ERR_ACL_CONN_EXISTS – ACL-Verbindung existiert bereits.";
        case 524:
            return "(0x0C): BLE_ERR_CMD_DISALLOWED – Befehl aktuell nicht erlaubt.";
        case 525:
            return "(0x0D): BLE_ERR_CONN_REJ_RESOURCES – Verbindung wegen fehlender Ressourcen abgewiesen.";
        case 526:
            return "(0x0E): BLE_ERR_CONN_REJ_SECURITY – Verbindung wegen Sicherheitsgründen abgewiesen.";
        case 527:
            return "(0x0F): BLE_ERR_CONN_REJ_BD_ADDR – Verbindung wegen unzulässiger Bluetooth-Adresse abgelehnt.";
        case 528:
            return "(0x10): BLE_ERR_CONN_ACCEPT_TMO – Verbindungsannahme-Timeout überschritten.";
        case 530:
            return "(0x12): BLE_ERR_INVALID_HCI_PARAMS – Ungültige HCI-Befehlsparameter.";
        case 531:
            return "(0x13): BLE_ERR_REM_USER_CONN_TERM – Remote-Nutzer hat die Verbindung getrennt.";
        case 532:
            return "(0x14): BLE_ERR_RD_CONN_TERM_RESRCS – Remote-Gerät hat wegen Ressourcenmangel getrennt.";
        case 533:
            return "(0x15): BLE_ERR_RD_CONN_TERM_PWROFF – Remote-Gerät hat sich ausgeschaltet.";
        case 534:
            return "(0x16): BLE_ERR_CONN_TERM_LOCAL – Lokales Gerät hat die Verbindung beendet.";
        case 535:
            return "(0x17): BLE_ERR_REPEATED_ATTEMPTS – Zu viele Pairing-Versuche hintereinander (Sperre).";
        case 536:
            return "(0x18): BLE_ERR_PAIRING_NOT_ALLOW – Pairing auf diesem Gerät nicht erlaubt.";
        case 538:
            return "(0x1A): BLE_ERR_UNSUPPORTED_REM_FEATURE – Remote-Gerät unterstützt diese Bluetooth-Funktion nicht.";
        case 541:
            return "(0x1D): BLE_ERR_MIC_FAILURE – Message Integrity Check fehlgeschlagen (Kryptographie-Fehler).";
        case 542:
            return "(0x1E): BLE_ERR_CONN_ESTABLISHMENT_TMO – Verbindung konnte nicht rechtzeitig aufgebaut werden.";
        case 545:
            return "(0x21): BLE_ERR_LMP_PDU_NOT_ALLOW – Protokoll-Paket (LMP PDU) nicht erlaubt.";
        case 546:
            return "(0x22): BLE_ERR_LMP_LL_RESP_TMO – Keine Antwort auf Link-Layer-Ebene (Timeout).";
        case 547:
            return "(0x23): BLE_ERR_LMP_COLLISION – Kollision bei der Protokoll-Verhandlung.";
        case 556:
            return "(0x2C): BLE_ERR_UNSUPPORTED_LMP_EXT – Erweiterte LMP-Funktion wird nicht unterstützt.";
        case 560:
            return "(0x30): BLE_ERR_INVALID_LMP_PARAMS – Ungültige LMP/LL-Parameter.";
        case 564:
            return "(0x34): BLE_ERR_DIFF_TRANSACTION_COLL – Transaktions-Kollision im Link-Layer.";
        case 570:
            return "(0x3A): BLE_ERR_UNACCEPT_CONN_PARAMS – Die vorgeschlagenen Verbindungsparameter sind unzulässig.";
        case 574:
            return "(0x3E): BLE_ERR_CONN_ESTABLISHMENT – Verbindung fehlgeschlagen, kein einziges Paket empfangen.";
        default:
            return "Unknown Reason";
    }
}

class MyClientCallback : public NimBLEClientCallbacks {
    
    void onConnect(NimBLEClient *pClient) override { DEBUG_PRINTF("BLE Connected\n"); }

    void onDisconnect(NimBLEClient *pClient, int reason) override {
        DEBUG_PRINTF("%s Disconnected, reason = %d %s - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason, getDisconnectReasonText(reason));
        DEBUG_PRINTLN("Reset flags to allow sending getConfigInfo and getDeviceInfo again.");
        CI_send = false; // Reset the flag to allow sending getConfigInfo again
        DI_send = false; // Reset the flag to allow sending getDeviceInfo again
        CD_running = false; // Reset the flag to indicate that we are no longer receiving cell data frames
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} clientCallbacks;

/** Define a class to handle the callbacks when scan events are received */
class ScanCallbacks : public NimBLEScanCallbacks {
    
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
        
        DEBUG_PRINTF("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
        if (advertisedDevice->isAdvertisingService(serviceUUID) && advertisedDevice->getName() == devicename) {
            DEBUG_PRINTF("Found our server \"%s\"\n", devicename);
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
        /**
         *  Set initial connection parameters:
         *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
         */
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
    std::string macAddr = myDevice->getAddress().toString();
    char rssiVal[12];
    snprintf(rssiVal, sizeof(rssiVal), "%d", myDevice->getRSSI());
    setState("ble_device_mac", macAddr.c_str(), true);
    setState("ble_device_rssi", rssiVal, true);

    /** Now we can read/write/subscribe the characteristics of the services we are interested in */
    // Obtain a reference to the service we are after in the remote BLE server.
    pSvc = pClient->getService(serviceUUID);
    if (pSvc) {
        DEBUG_PRINTLN(" - Found our service");
        pChr = pSvc->getCharacteristic(charUUID);
        if (pChr == nullptr) {
            std::string charUuid = charUUID.toString();
            DEBUG_PRINTF("Failed to find our characteristic UUID: %s\n", charUuid.c_str());
            pClient->disconnect();
            return false;
        }
        DEBUG_PRINTLN(" - Found our characteristic");
        // Set the notification callback
        if (pChr->canNotify()) {
            if (!pChr->subscribe(true, notifyCB)) {
                DEBUG_PRINTLN("Failed to subscribe to notifications");
                pClient->disconnect();
                return false;
            }
            DEBUG_PRINTLN("Subscribed to notifications.");
            delay(200); // Small delay to ensure subscription is set up before sending the first message
        }
    } else {
        std::string svcUuid = serviceUUID.toString();
        DEBUG_PRINTF("Failed to find our service UUID: %s\n", svcUuid.c_str());
        pClient->disconnect();
        return false;
    }
    if (pChr->canWriteNoResponse()) {
        DEBUG_PRINTLN("Start the show, unblock notifications ...");
        all_notifications_blocked = false; // Unblock notifications
    } else {
        DEBUG_PRINTLN("Characteristic does not support writing with no response");
        pClient->disconnect();
        return false;
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
    // Communication Flow

    // 1. Establish BLE connection
    // 2. Register for notifications on characteristic 0xFFE1 (handle 0x05)
    // 3. Send 0x96 on characteristic 0xFFE1 (handle 0x03) → BMS responds with Frame 0x01 (config info)
    // 4. Send 0x97 on characteristic 0xFFE1 (handle 0x03) → BMS responds with Frame 0x03 (device info)
    // 5. After both commands are acknowledged, BMS automatically streams Frame 0x02 (cell info) periodically
    
    if (ble_connected) {
        // make shure the complete logic is running with the same time base, so we save the last millis() and use it for all timing calculations
        save_millis = millis(); // Save the current time for consistent timing calculations
        // Handle sending getDeviceInfo and getConfigInfo messages with timing and blocking logic
        
        // Send getConfigInfo message if not already sent
        if (!CI_send && pChr->writeValue(getConfigInfo, 20, false)) {
            DEBUG_PRINTF("Sent getConfigInfo message: %s\n.", getConfigInfo_str);
            time_CI_sent = save_millis; // Update the time when we sent the config info request
            CI_send = true; // Mark that the Config Info is sent
        }
        
        // Send getDeviceInfo message if not already sent and SEND_INTERVAL seconds after Config Info is sent
        else if (!DI_send && CI_send && ((save_millis - time_CI_sent) >= SEND_INTERVAL) && pChr->writeValue(getDeviceInfo, 20, false)) {
            DEBUG_PRINTF("Sent getDeviceInfo message: %s\n.", getDeviceInfo_str);
            DI_send = true; // Mark that the Device Info is sent
        }
        
        // After both messages have been sent, the device will send cell data frames continuously, so we will not send getDeviceInfo and getConfigInfo again
        // but we will check if we receive them within the expected interval (4-5 times a second) and if not we will send DI and CI requests again,
        // but only if the previous request has been answered
        else if (CI_send && DI_send && CD_running && save_millis - lastRcvdCDTime > MAX_TIME_BETWEEN_CELL_DATA_MESSAGES) {
            DEBUG_PRINTLN("No cell data received for more than 2 seconds, sending getDeviceInfo and getConfigInfo again.");
            // Reset the flags to allow sending getDeviceInfo and getConfigInfo again in next loop iteration
            CI_send = false; // Reset the flag to allow sending getConfigInfo again
            DI_send = false; // Reset the flag to allow sending getDeviceInfo again
            CD_running = false; // Reset the flag to indicate that we are no longer receiving cell data frames
        }
        
        if (last_rssi_time == 0 || (save_millis - last_rssi_time) >= BLE_RSSI_INTERVAL) {
            last_rssi_time = save_millis;
            char rssiVal[12];
            snprintf(rssiVal, sizeof(rssiVal), "%d", myDevice->getRSSI());
            setState("ble_device_rssi", rssiVal, true);
        }
    }
}

#ifdef DUALCORE

// Define the parser task
void parserTask(void *pvParameters) {
    uint8_t messageFromQueue[BUFFER_SIZE];
    time_t lastParserTime = 0;

    while (true) {
        // Receive data from the queue
        if (xQueueReceive(bleQueue, &messageFromQueue, portMAX_DELAY) == pdTRUE) {
            lastParserTime = millis(); // Update the last parser time when a message is received
            // Call the parser function
            parser(messageFromQueue);
        }
        while (millis() - lastParserTime < 25) {
            // Wait until 25 milliseconds have passed since the last parser call
            vTaskDelay(1); // Delay for 1 tick (1 ms)
        }
    }
}
#endif

void ble_setup() {
    
#ifdef DUALCORE
    // Create the queue
    bleQueue = xQueueCreate(20, sizeof(uint8_t[BUFFER_SIZE]));
    DEBUG_PRINTLN("BLE queue created");

    // Create the parser task on core 1
    xTaskCreatePinnedToCore(parserTask, "Parser Task", 8192, NULL, 1, NULL, 1);
    DEBUG_PRINTLN("Parser Task created");

#endif

    DEBUG_PRINTLN("Starting NimBLE Client\n");
    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);            // +9dbm
    NimBLEScan *pBLEScan = NimBLEDevice::getScan();    // Create a new scan    
    pBLEScan->setScanCallbacks(&scanCallbacks, false); // Set the callback handlers to be called when we receive a result and when the scan is complete.

    DEBUG_PRINTLN("BLE client started");

    pBLEScan->setInterval(100);
    pBLEScan->setWindow(100);
    pBLEScan->setActiveScan(true);

    /** Start scanning for advertisers */
    pBLEScan->start(scanTimeMs);

    DEBUG_PRINTF("Scan for our Server \"%s\"\n", devicename);
}