#ifndef HTML_H
#define HTML_H

#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include <rom/rtc.h>
#include <time.h>
#include <WebServer.h>

struct ResetEntry
{
    uint8_t reason;
    time_t timestamp;
};


String formatTime(time_t t);
String get_reset_reason_string(esp_reset_reason_t reason);

const char *get_reset_reason_class(esp_reset_reason_t reason);

void handleBmsPage(WebServer &server);
void handleResetHistoryPage(WebServer &server, const ResetEntry *history, size_t historyCount);
void handleMqttConfigPage(WebServer &server);

#endif // HTML_H
