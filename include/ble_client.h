#ifndef BLE_CLIENT_H
#define BLE_CLIENT_H
#include <Arduino.h>
#include "config.h"
#include "settings.h"
#include "led_control.h"
#include <mutex>
#include <NimBLEDevice.h>
#include "mqtt_handler.h"
#include "parser.h"
#include "led_control.h"


#define BUFFER_SIZE 300 // Size of the buffer to store incoming BLE data, adjust as needed
#define SEND_INTERVAL 3000UL // Define the interval for sending getDeviceInfo and getConfigInfo (2 seconds)
#define BLE_RSSI_INTERVAL 30000UL // Define the interval for reading BLE RSSI (30 seconds)
#define MAX_TIME_BETWEEN_CELL_DATA_MESSAGES 1000UL // Maximum time allowed between receiving cell data messages (1 second)
#define CI_AND_DI_INTERVAL 16200000UL // Define the interval for sending getDeviceInfo and getConfigInfo (4.5 hours); My V15 PB BMS freezes after 5 hours if no device info  and config Info frame is requested!

// Carries a complete BLE message together with the timestamp (epoch milliseconds) taken right after it was fully received (CRC OK)
struct BleFrame
{
    uint8_t data[BUFFER_SIZE];
    int64_t timestamp;
};

void ble_setup();
void ble_loop();

#endif // BLE_CLIENT_H
