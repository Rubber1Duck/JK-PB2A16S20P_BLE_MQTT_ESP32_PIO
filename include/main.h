#include <Arduino.h>
#include "version.h"
#include "config.h"
#include "wifi_handler.h"
#include "mqtt_handler.h"
#include "ble_client.h"
#include "rs485_handler.h"
#include "led_control.h"
#include "macros.h"
#include <time.h>
#include <Preferences.h>
#include <rom/rtc.h> // Erforderlich für detaillierte Reset-Infos
#include <WebServer.h>

#ifdef USE_INFLUXDB
#include "influxdb_handler.h"
#endif

#ifdef USE_TLS
extern WiFiClientSecure secure_wifi_client;
extern const char *root_ca_cert PROGMEM;
#endif

extern void publish_init();
extern Preferences prefs;