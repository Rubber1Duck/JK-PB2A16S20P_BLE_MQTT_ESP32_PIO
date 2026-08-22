#include "settings.h"

Preferences prefs;
const char *nvs_namespace = "system"; // NVS namespace for storing settings

// Global variables to hold the settings values.
// As of limitations of the preferences libary the key length is limited to 15 characters, so we have to use short names for the settings keys.
// the variables are named same as the keys to make it easier to understand which variable corresponds to which key.
//       123456789012345
uint16_t publish_delay;
uint16_t min_pub_time;
bool     debug_flg = false;
bool     debug_flg_full = false;
uint16_t publishInterval;

static bool openSettingsPrefs(Preferences &localPrefs, bool readOnly)
{
    if (!localPrefs.begin(nvs_namespace, readOnly))
    {
        Serial.println(readOnly
                           ? "ERROR: Failed to open NVS namespace for read"
                           : "ERROR: Failed to open NVS namespace for read/write");
        return false;
    }
    return true;
}

void write_setting(const char *setting_name, uint16_t value)
{
    Preferences localPrefs;
    if (!openSettingsPrefs(localPrefs, false))
    {
        return;
    }
    // first read the current value to check if it has changed, if not, we can skip writing to flash
    uint16_t current_value = localPrefs.getUShort(setting_name, value);
    if (current_value == value) {
        localPrefs.end();
        return;
    }
    // Save the uint16_t value
    size_t written = localPrefs.putUShort(setting_name, value);
    localPrefs.end();
    if (written != sizeof(uint16_t))
    {
        Serial.println(String("ERROR: Failed to write uint16 setting: ") + setting_name);
        return;
    }
    DEBUG_PRINTF("Value for %s changed to %u\n", setting_name, static_cast<unsigned int>(value));
    re_read_settings();
}

void write_setting(const char *setting_name, bool value)
{
    Preferences localPrefs;
    if (!openSettingsPrefs(localPrefs, false))
    {
        return;
    }
    // first read the current value to check if it has changed, if not, we can skip writing to flash
    uint8_t current_value = localPrefs.getUChar(setting_name, value ? 1 : 0);
    if (current_value == (value ? 1 : 0)) {
        localPrefs.end();
        return;
    }
    // Save the bool value as uint8_t (1 for true, 0 for false)
    size_t written = localPrefs.putUChar(setting_name, value ? 1 : 0);
    localPrefs.end();
    if (written != sizeof(uint8_t))
    {
        Serial.println(String("ERROR: Failed to write bool setting: ") + setting_name);
        return;
    }
    DEBUG_PRINTF("Value for %s changed to %s\n", setting_name, value ? "true" : "false");
    re_read_settings();
}

uint16_t read_setting(const char *setting_name, uint16_t default_value)
{
    Preferences localPrefs;
    if (!openSettingsPrefs(localPrefs, true))
    {
        return default_value;
    }
    uint16_t value = localPrefs.getUShort(setting_name, default_value);
    localPrefs.end();
    return value;
}

bool read_setting(const char *setting_name, bool default_value)
{
    Preferences localPrefs;
    if (!openSettingsPrefs(localPrefs, true))
    {
        return default_value;
    }
    bool value = localPrefs.getUChar(setting_name, default_value ? 1 : 0) == 1;
    localPrefs.end();
    return value;
}

void re_read_settings()
{
    Preferences localPrefs;
    if (!openSettingsPrefs(localPrefs, false))
    {
        return;
    }
    if (!localPrefs.isKey("publish_delay")) {
        Serial.println("publish_delay setting is missing in NVS. Re-initializing with default value.");
        if (localPrefs.putUShort("publish_delay", (uint16_t)PUBLISH_DELAY) != sizeof(uint16_t))
        {
            Serial.println("ERROR: Failed to persist publish_delay");
        }
    }
    if (!localPrefs.isKey("min_pub_time")) {
        Serial.println("min_pub_time setting is missing in NVS. Re-initializing with default value.");
        if (localPrefs.putUShort("min_pub_time", (uint16_t)MIN_PUB_TIME) != sizeof(uint16_t))
        {
            Serial.println("ERROR: Failed to persist min_pub_time");
        }
    }
    if (!localPrefs.isKey("debug_flg")) {
        Serial.println("debug_flg setting is missing in NVS. Re-initializing with default value.");
        if (localPrefs.putUChar("debug_flg", 0) != sizeof(uint8_t))
        {
            Serial.println("ERROR: Failed to persist debug_flg");
        }
    }
    if (!localPrefs.isKey("debug_flg_full")) {
        Serial.println("debug_flg_full setting is missing in NVS. Re-initializing with default value.");
        if (localPrefs.putUChar("debug_flg_full", 0) != sizeof(uint8_t))
        {
            Serial.println("ERROR: Failed to persist debug_flg_full");
        }
    }
    if (!localPrefs.isKey("publishInterval")) {
        Serial.println("publishInterval setting is missing in NVS. Re-initializing with default value.");
        if (localPrefs.putUShort("publishInterval", (uint16_t)PUBLISH_INTERVAL) != sizeof(uint16_t))
        {
            Serial.println("ERROR: Failed to persist publishInterval");
        }
    }

    localPrefs.end();
    
    publish_delay = read_setting("publish_delay", (uint16_t)PUBLISH_DELAY);
    // Limit to 1000 seconds as sometimes the value is corrupted 0xFFFF
    publish_delay = publish_delay > 1000 ? (uint16_t)PUBLISH_DELAY : publish_delay;
    min_pub_time = read_setting("min_pub_time", (uint16_t)MIN_PUB_TIME);
    // sometimes the value is corrupted 0xFFFF
    min_pub_time = min_pub_time > 1000 ? (uint16_t)MIN_PUB_TIME : min_pub_time;
    debug_flg = read_setting("debug_flg", false);
    debug_flg_full = read_setting("debug_flg_full", false);
    publishInterval = read_setting("publishInterval", (uint16_t)PUBLISH_INTERVAL);
    // Limit to 200 milliseconds
    publishInterval = publishInterval > 200 ? (uint16_t)PUBLISH_INTERVAL : publishInterval;
}

void init_settings()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        Serial.println(err == ESP_ERR_NVS_NO_FREE_PAGES ? "ESP_ERR_NVS_NO_FREE_PAGES" : "ESP_ERR_NVS_NEW_VERSION_FOUND");
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    re_read_settings();
}