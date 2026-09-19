#ifndef MDNS_HANDLER_H
#define MDNS_HANDLER_H

#include <Arduino.h>
#include <ESPmDNS.h>

#include "config.h"
#include "macros.h"

// DHCP Hostname
#define MDNS_HOSTNAME TEXTIFY(CLTNAME)

void init_mdns_handler();

#endif // MDNS_HANDLER_H