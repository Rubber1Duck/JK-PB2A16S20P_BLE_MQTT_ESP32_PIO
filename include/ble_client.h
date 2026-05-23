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
#define INITIAL_SEND_INTERVAL 5000UL   // Define the interval for the initial send (5 seconds)
#define BLE_RSSI_INTERVAL 60000UL // Define the interval for reading BLE RSSI (1 minute)
#define MINIMUM_RECIVE_INTERVAL_DEVICEINFO_AND_CONFIGINFO 900000UL // Minimum interval between processing received device info and config info (15 minutes)


void ble_setup();
void ble_loop();

#endif // BLE_CLIENT_H
