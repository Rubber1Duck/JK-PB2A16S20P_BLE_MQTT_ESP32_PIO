#ifndef MACROS_H
#define MACROS_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define DOUBLEESCAPE(a) #a
#define TEXTIFY(a) DOUBLEESCAPE(a)

// Serial-Mutex für thread-safe Ausgaben
extern SemaphoreHandle_t serialMutex;

// Serial Output configuration
#ifdef SERIAL_OUT
#define DEBUG_PRINT(...) \
    do { \
        if (serialMutex && xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) { \
            Serial.print(__VA_ARGS__); \
            xSemaphoreGive(serialMutex); \
        } \
    } while(0)

#define DEBUG_PRINTLN(...) \
    do { \
        if (serialMutex && xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) { \
            Serial.println(__VA_ARGS__); \
            xSemaphoreGive(serialMutex); \
        } \
    } while(0)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

#endif //MACROS_H