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
#define REPEAT_SEND_INTERVAL 3600000UL // Define the interval for sending data (1 hour)
#define INITIAL_SEND_INTERVAL 3000UL   // Define the interval for the initial send (3 seconds)
#define BLE_RSSI_INTERVAL 60000UL // Define the interval for reading BLE RSSI (1 minute)


void ble_setup();
void ble_loop();

#endif // BLE_CLIENT_H
