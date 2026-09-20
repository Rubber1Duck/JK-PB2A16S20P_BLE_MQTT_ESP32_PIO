#include <Arduino.h>
#include "version.h"
#include "config.h"
#include "wifi_handler.h"
#include "mqtt_handler.h"
#include "app_webserver.h"
#include "ble_client.h"
#include "led_control.h"
#include "macros.h"
#include "syslog_handler.h"
#include <time.h>
#include <settings.h>
#include <rom/rtc.h> // Erforderlich für detaillierte Reset-Infos
#include "html.h"
#include "mdns_handler.h"
#include "reset_history.h"
#ifdef USE_TLS
extern WiFiClientSecure secure_wifi_client;
#endif

#ifdef NTPSERVER
const char *ntpServer = NTPSERVER;
#ifdef TIMEZONE
const char *time_zone = TIMEZONE;
#else
const long gmtOffset_sec = GMTOFFSET;
const int daylightOffset_sec = DLOFFSET;
#endif
#endif // NTPSERVER

extern void publish_init();

