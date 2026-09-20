#include "reset_history.h"

const char *RESET_HISTORY_KEY = "reset_history"; //limited to 15 characters due to NVS key length limit!
ResetEntry history[MAX_RESET_REASONS];

void debug_print_reset_history(){
      DEBUG_PRINTLN("\n--- ESP32 Reset History ---");
    uint8_t currentReason = (uint8_t)esp_reset_reason();
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        DEBUG_PRINTLN("Zeit-Sync fehlgeschlagen");

    prefs.begin(nvs_namespace, false);
    prefs.getBytes(RESET_HISTORY_KEY, history, sizeof(history));
    memmove(&history[1], &history[0], sizeof(ResetEntry) * (MAX_RESET_REASONS - 1));
    history[0].reason = currentReason;
    time(&history[0].timestamp);
    prefs.putBytes(RESET_HISTORY_KEY, history, sizeof(history));
    prefs.end();

    for (int i = 0; i < MAX_RESET_REASONS; i++)
    {
        if (history[i].reason == 0 && i > 0)
            continue;
        DEBUG_PRINT("Eintrag [");
        DEBUG_PRINT(i);
        DEBUG_PRINT("]: ");
        DEBUG_PRINT(formatTime(history[i].timestamp));
        DEBUG_PRINT(" - ");
        DEBUG_PRINT(get_reset_reason_string((esp_reset_reason_t)history[i].reason));
        DEBUG_PRINT(" (Code: ");
        DEBUG_PRINT((int)history[i].reason);
        DEBUG_PRINTLN(")");
    }
    DEBUG_PRINTLN("---------------------------\n");
}