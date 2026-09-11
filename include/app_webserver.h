#ifndef APP_WEBSERVER_H
#define APP_WEBSERVER_H

#include "config.h"

#ifdef USE_WEBSERVER

#include <Arduino.h>
#include "html.h"

#define OTA_HOSTNAME TEXTIFY(CLTNAME)

void setupWebserver(ResetEntry *history, size_t historyCount, const char *nvsKey);
void webserverLoop();

#endif // USE_WEBSERVER

#endif // APP_WEBSERVER_H