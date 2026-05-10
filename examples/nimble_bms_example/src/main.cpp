#include <Arduino.h>
#include <NimBLEDevice.h>

// Beispiel fuer einen NimBLE-Client fuer ein BMS mit:
// - Service UUID: 0xFFE0
// - Characteristic UUID: 0xFFE1
//   - Write: Handle 0x03
//   - Notify: Handle 0x05
//
// In NimBLE arbeitest du auf Client-Seite normalerweise mit UUIDs.
// Die Handles 0x03 und 0x05 sind auf dem Geraet wichtig, im Client-Code reicht es
// aber meist, die eine Characteristic mit UUID 0xFFE1 zu finden und dort Write/Notify zu nutzen.

static const NimBLEUUID kServiceUUID("ffe0");
static const NimBLEUUID kCharacteristicUUID("ffe1");
static constexpr bool kUseDeviceNameFilter = false;
static const char *kTargetDeviceName = "Garage-01";

// Beispiel-Kommandos aus dem bestehenden Projekt.
// Diese Frames bitte an dein konkretes BMS-Protokoll anpassen, falls noetig.
static const uint8_t kCmdDeviceInfo[20] = {
    0xaa, 0x55, 0x90, 0xeb, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11
};
static const uint8_t kCmdCellData[20] = {
    0xaa, 0x55, 0x90, 0xeb, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
};

static NimBLEAdvertisedDevice *g_device = nullptr;
static NimBLERemoteCharacteristic *g_characteristic = nullptr;
static bool g_doConnect = false;
static bool g_connected = false;
static bool g_initialRequestDone = false;
static unsigned long g_lastWriteMs = 0;
static constexpr uint32_t kScanTimeMs = 5000;
static constexpr uint32_t kInitialDelayMs = 3000;
static constexpr uint32_t kRepeatDelayMs = 60000;

static bool isTargetDevice(const NimBLEAdvertisedDevice *advertisedDevice) {
    if (!advertisedDevice->isAdvertisingService(kServiceUUID)) {
        return false;
    }

    if (!kUseDeviceNameFilter) {
        return true;
    }

    return advertisedDevice->haveName() && advertisedDevice->getName() == kTargetDeviceName;
}

static void printHex(const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (data[i] < 0x10) {
            Serial.print('0');
        }
        Serial.print(data[i], HEX);
        if (i + 1 < length) {
            Serial.print(' ');
        }
    }
    Serial.println();
}

static void notifyCallback(NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
    (void)characteristic;
    (void)isNotify;

    Serial.print("Notify [");
    Serial.print(length);
    Serial.print(" bytes]: ");
    printHex(data, length);
}

class ClientCallbacks : public NimBLEClientCallbacks {
    void onDisconnect(NimBLEClient *client, int reason) override {
        Serial.print("Disconnected: ");
        Serial.println(reason);
        g_connected = false;
        g_initialRequestDone = false;
        g_characteristic = nullptr;
        NimBLEDevice::getScan()->start(kScanTimeMs, false, true);
    }
};

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
        if (!isTargetDevice(advertisedDevice)) {
            return;
        }

        Serial.print("Found BMS candidate: ");
        Serial.println(advertisedDevice->toString().c_str());
        if (kUseDeviceNameFilter) {
            Serial.print("Matched device name: ");
            Serial.println(kTargetDeviceName);
        }

        NimBLEDevice::getScan()->stop();
        g_device = const_cast<NimBLEAdvertisedDevice *>(advertisedDevice);
        g_doConnect = true;
    }

    void onScanEnd(const NimBLEScanResults &results, int reason) override {
        (void)results;
        Serial.print("Scan ended, reason: ");
        Serial.println(reason);
        if (!g_connected) {
            NimBLEDevice::getScan()->start(kScanTimeMs, false, true);
        }
    }
};

static ClientCallbacks g_clientCallbacks;
static ScanCallbacks g_scanCallbacks;

static bool sendCommand(const uint8_t *command, size_t length) {
    if (g_characteristic == nullptr) {
        return false;
    }

    bool ok = false;
    if (g_characteristic->canWriteNoResponse()) {
        ok = g_characteristic->writeValue(command, length, false);
    } else if (g_characteristic->canWrite()) {
        ok = g_characteristic->writeValue(command, length, true);
    }

    if (ok) {
        Serial.print("Sent command: ");
        printHex(command, length);
        g_lastWriteMs = millis();
    } else {
        Serial.println("Write failed");
    }

    return ok;
}

static bool connectToBms() {
    if (g_device == nullptr) {
        return false;
    }

    NimBLEClient *client = nullptr;
    if (NimBLEDevice::getCreatedClientCount()) {
        client = NimBLEDevice::getClientByPeerAddress(g_device->getAddress());
        if (client != nullptr && !client->isConnected()) {
            if (!client->connect(g_device, false)) {
                Serial.println("Reconnect failed");
                return false;
            }
        }
    }

    if (client == nullptr) {
        client = NimBLEDevice::createClient();
        client->setClientCallbacks(&g_clientCallbacks, false);
        client->setConnectionParams(12, 12, 0, 150);
        client->setConnectTimeout(5000);

        if (!client->connect(g_device)) {
            NimBLEDevice::deleteClient(client);
            Serial.println("Connect failed");
            return false;
        }
    }

    NimBLERemoteService *service = client->getService(kServiceUUID);
    if (service == nullptr) {
        Serial.println("Service 0xFFE0 not found");
        client->disconnect();
        return false;
    }

    g_characteristic = service->getCharacteristic(kCharacteristicUUID);
    if (g_characteristic == nullptr) {
        Serial.println("Characteristic 0xFFE1 not found");
        client->disconnect();
        return false;
    }

    if (g_characteristic->canNotify()) {
        if (!g_characteristic->subscribe(true, notifyCallback)) {
            Serial.println("Subscribe failed");
            client->disconnect();
            return false;
        }
        Serial.println("Notifications enabled on 0xFFE1");
    } else {
        Serial.println("Characteristic does not support notify");
    }

    g_connected = true;
    g_initialRequestDone = false;
    g_lastWriteMs = millis();

    Serial.print("Connected to ");
    Serial.println(client->getPeerAddress().toString().c_str());
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("NimBLE BMS example starting");
    NimBLEDevice::init("NimBLE-BMS-Example");
    NimBLEDevice::setPower(ESP_PWR_LVL_P6);

    NimBLEScan *scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(&g_scanCallbacks, false);
    scan->setInterval(100);
    scan->setWindow(100);
    scan->setActiveScan(true);
    scan->start(kScanTimeMs, false, true);
}

void loop() {
    delay(10);

    if (g_doConnect) {
        g_doConnect = false;
        connectToBms();
    }

    if (!g_connected || g_characteristic == nullptr) {
        return;
    }

    if (!g_initialRequestDone) {
        if (millis() - g_lastWriteMs >= kInitialDelayMs) {
            Serial.println("Send initial device info request");
            if (sendCommand(kCmdDeviceInfo, sizeof(kCmdDeviceInfo))) {
                g_initialRequestDone = true;
            }
        }
        return;
    }

    if (millis() - g_lastWriteMs >= kRepeatDelayMs) {
        Serial.println("Send cell data request");
        sendCommand(kCmdCellData, sizeof(kCmdCellData));
    }
}
