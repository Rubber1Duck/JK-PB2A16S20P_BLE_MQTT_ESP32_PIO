#include <Arduino.h>
#include "version.h"
#include "config.h"
#include "wifi_handler.h"
#include "mqtt_handler.h"
#include "ble_client.h"
#include "led_control.h"
#include "macros.h"
#include <time.h>
#include <settings.h>
#include <rom/rtc.h> // Erforderlich für detaillierte Reset-Infos
#include "html.h"

#ifdef USE_TLS
extern WiFiClientSecure secure_wifi_client;
#endif

extern void publish_init();
extern Preferences prefs;
extern const char *nvs_namespace;
