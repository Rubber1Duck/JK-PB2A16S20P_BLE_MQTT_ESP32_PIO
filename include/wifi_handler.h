#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include "macros.h"

void init_wifi();
void wifi_loop();

// DHCP Hostname
#define WIFI_DHCPNAME TEXTIFY(CLTNAME)

extern void setState(String key, String value, bool publish);

#endif