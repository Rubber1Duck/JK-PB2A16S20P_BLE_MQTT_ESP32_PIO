#ifndef RESET_HISTORY_H
#define RESET_HISTORY_H
#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include <preferences.h>
#include "html.h"

extern Preferences prefs;
extern const char *nvs_namespace;
extern const char *RESET_HISTORY_KEY; //limited to 15 characters due to NVS key length limit!
extern ResetEntry history[MAX_RESET_REASONS];

void debug_print_reset_history();

#endif // RESET_HISTORY_H