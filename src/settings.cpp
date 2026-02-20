#include "settings.h"

Preferences prefs;

// Global variables to hold the settings values.
// As of limitations of the Preferences the key length is limited to 15 characters, so we have to use short names for the settings keys.
//       123456789012345
uint16_t publish_delay;
uint16_t min_pub_time;
bool debug_flg = false;
bool debug_flg_full = false;

void write_setting(const char *setting_name, uint16_t value)
{
    // "storage" is the namespace
    prefs.begin("storage", false);
    // Save the uint16_t value
    prefs.putUShort(setting_name, value);
    prefs.end();
    DEBUG_PRINTLN("Value for " + String(setting_name) + "  changed to  " + String(value));
    re_read_settings();
}

void write_setting(const char *setting_name, bool value)
{
    prefs.begin("storage", false);
    prefs.putUChar(setting_name, value ? 1 : 0);
    prefs.end();
    DEBUG_PRINTLN("Value for " + String(setting_name) + "  changed to  " + String(value));
    re_read_settings();
}

uint16_t read_setting(const char *setting_name, uint16_t default_value)
{
    prefs.begin("storage", true);
    uint16_t value = prefs.getUShort(setting_name, default_value);
    prefs.end();
    return value;
}

bool read_setting(const char *setting_name, bool default_value)
{
    prefs.begin("storage", true);
    bool value = prefs.getUChar(setting_name, default_value ? 1 : 0) == 1;
    prefs.end();
    return value;
}

void re_read_settings()
{
    publish_delay = read_setting("publish_delay", (uint16_t)PUBLISH_DELAY);
    // Limit to 1000 seconds as sometimes the value is corrupted 0xFFFF
    publish_delay = publish_delay > 1000 ? (uint16_t)PUBLISH_DELAY : publish_delay;
    min_pub_time = read_setting("min_pub_time", (uint16_t)MIN_PUB_TIME);
    // sometimes the value is corrupted 0xFFFF
    min_pub_time = min_pub_time > 1000 ? (uint16_t)MIN_PUB_TIME : min_pub_time;
    debug_flg = read_setting("debug_flg", false);
    debug_flg_full = read_setting("debug_flg_full", false);
}

void init_settings()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    re_read_settings();
}