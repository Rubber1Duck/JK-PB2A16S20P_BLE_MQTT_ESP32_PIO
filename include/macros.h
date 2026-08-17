#ifndef MACROS_H
#define MACROS_H

#define DOUBLEESCAPE(a) #a
#define TEXTIFY(a) DOUBLEESCAPE(a)

// Serial Output configuration
#ifdef USE_SYSLOG
#include <PicoSyslog.h>
extern PicoSyslog::Logger syslog;
#define DEBUG_PRINT(...) syslog.print(__VA_ARGS__)
#define DEBUG_PRINTF(...) syslog.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...) syslog.println(__VA_ARGS__)

#elif defined(SERIAL_OUT)
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)

#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTLN(...)
#endif

#endif // MACROS_H