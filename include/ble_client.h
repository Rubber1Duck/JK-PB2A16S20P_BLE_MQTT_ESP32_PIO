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
#define INITIAL_SEND_INTERVAL 1000UL   // Define the interval for the initial send (1 second)
#define SEND_INTERVAL 1000UL // Define the interval for sending getDeviceInfo and getConfigInfo (1 second)
#define BLE_RSSI_INTERVAL 60000UL // Define the interval for reading BLE RSSI (1 minute)
#define MIN_RCV_ITV_DI_AND_CI_INFO 900000UL // Minimum interval between processing received device info and config info (15 minutes)
#define WAIT_FOR_RESPONSE_TIMEOUT 10000UL // Timeout for waiting for a response after sending getDeviceInfo or getConfigInfo (10 seconds)


void ble_setup();
void ble_loop();

#endif // BLE_CLIENT_H
