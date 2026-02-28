#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <cstring>
#include <PubSubClient.h>
#include <WiFi.h>
#include "version.h"
#include "config.h"
#include "settings.h"
#include "led_control.h"
#include "macros.h"
#include "struct_MQTT_Queue.h"
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <mutex>

#ifdef USE_TLS
#include <WiFiClientSecure.h>
#endif

void setState(String key, String value, bool publish);
void mqtt_loop();
void mqtt_init();
void toMqttQueue(String topic, String payload);

// MQTT Setting
// MQTT Client name used when connecting to broker
#define MQTT_CLTNAME TEXTIFY(CLTNAME)
extern String mqttname;
extern String mqtt_main_topic;
extern PubSubClient mqtt_client;
extern QueueHandle_t publishQueue;
extern boolean isWifiConnected;
extern uint16_t min_pub_time;
extern bool debug_flg;
extern bool debug_flg_full;
extern uint16_t publish_delay;
extern uint16_t publishInterval;

#endif // MQTT_HANDLER_H