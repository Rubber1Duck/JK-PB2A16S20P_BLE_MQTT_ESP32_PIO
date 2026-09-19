#include "mdns_handler.h"

void init_mdns_handler() {

    if (!MDNS.begin(MDNS_HOSTNAME)) {
        DEBUG_PRINTLN("Error setting up MDNS responder!");
    } else {
        DEBUG_PRINTLN("mDNS responder started. Hostname: " MDNS_HOSTNAME);
        MDNS.addService("http", "tcp", 80);
    }
}