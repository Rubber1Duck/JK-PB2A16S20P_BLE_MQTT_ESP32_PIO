#include "syslog_handler.h"

#ifdef USE_SYSLOG
PicoSyslog::Logger syslog;
#endif // USE_SYSLOG

void init_syslog_handler() {
#ifdef USE_SYSLOG
  syslog.server = SYSLOG_SERVER;
  syslog.port = SYSLOG_PORT;
  syslog.app = SYSLOG_APP;
  syslog.default_loglevel = PicoSyslog::LogLevel::information;
  syslog.host = SYSLOG_HOST;
#endif // USE_SYSLOG
}