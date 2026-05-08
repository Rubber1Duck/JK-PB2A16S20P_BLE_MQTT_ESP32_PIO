#ifndef HTML_H
#define HTML_H

#include <Arduino.h>
#include <WebServer.h>
#include <rom/rtc.h>
#include <time.h>

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

#endif // HTML_H
