#ifndef SYSLOG_HANDLER_H
#define SYSLOG_HANDLER_H

#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include <PicoSyslog.h>

void init_syslog_handler();

#endif // SYSLOG_HANDLER_H