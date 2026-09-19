#include "mqtt_publish_config.h"

#include <Preferences.h>
#include <cstring>

namespace
{
const char *NVS_NAMESPACE = "mqttcfg";

const MqttPublishField DEVICE_FIELDS[] = {
    {"read_count", "Frame Counter"},
    {"vendor_id", "Hersteller-ID"},
    {"hw_revision", "Hardware-Version"},
    {"hardware_options", "Hardware-Optionen"},
    {"sw_version", "Software-Version"},
    {"uptime", "Laufzeit"},
    {"uptime_fmt", "Laufzeit formatiert"},
    {"power_up_times", "Einschaltzyklen"},
    {"device_name", "Gerätename"},
    {"device_passwd", "Geräte-Passwort"},
    {"manufacturing_date", "Herstellungsdatum"},
    {"serial_number", "Seriennummer"},
    {"passcode", "Passcode"},
    {"user_data", "Benutzerdaten"},
    {"setup_passcode", "Setup-Passcode"},
    {"user_data2", "Benutzerdaten 2"},
    {"uart1_protocol_number", "UART1 Protokollnummer"},
    {"uart1_protocol_txt", "UART1 Protokoll"},
    {"uart1_protocol_enable", "UART1 aktiviert"},
    {"can_protocol_number", "CAN Protokollnummer"},
    {"can_protocol_txt", "CAN Protokoll"},
    {"uart_protocol_enable_0_15", "UART Protokollstatus"},
    {"uart2_protocol_number", "UART2 Protokollnummer"},
    {"uart2_protocol_txt", "UART2 Protokoll"},
    {"uart2_protocol_enable", "UART2 aktiviert"},
    {"uart_protocol_lib_version", "UART Bibliotheksversion"},
    {"lcd_buzzer_trigger", "LCD-Buzzer Trigger"},
    {"lcd_buzzer_trigger_txt", "LCD-Buzzer Triggertext"},
    {"lcd_buzzer_trigger_value", "LCD-Buzzer Triggerwert"},
    {"lcd_buzzer_release_value", "LCD-Buzzer Rücksetzwert"},
    {"dry1_trigger", "Dry1 Trigger"},
    {"dry1_trigger_txt", "Dry1 Triggertext"},
    {"dry1_trigger_value", "Dry1 Triggerwert"},
    {"dry1_release_value", "Dry1 Rücksetzwert"},
    {"dry2_trigger", "Dry2 Trigger"},
    {"dry2_trigger_txt", "Dry2 Triggertext"},
    {"dry2_trigger_value", "Dry2 Triggerwert"},
    {"dry2_release_value", "Dry2 Rücksetzwert"},
    {"can_protocol_lib_version", "CAN Bibliotheksversion"},
    {"rcv_time", "RCV-Zeit"},
    {"rfv_time", "RFV-Zeit"}
};

const MqttPublishField CONFIG_FIELDS[] = {
    {"read_count", "Frame Counter"},
    {"vol_smart_sleep", "Smart-Sleep-Spannung"},
    {"vol_cell_uv", "UVP"},
    {"vol_cell_uvpr", "UVPR"},
    {"vol_cell_ov", "OVP"},
    {"vol_cell_ovpr", "OVPR"},
    {"vol_balan_trig", "Balance-Startspannung"},
    {"vol_100_percent", "Spannung bei 100 %"},
    {"vol_0_percent", "Spannung bei 0 %"},
    {"vol_cell_rcv", "RCV"},
    {"vol_cell_rfv", "RFV"},
    {"vol_sys_pwr_off", "System-Abschaltspannung"},
    {"cur_bat_coc", "Charge OCP"},
    {"time_bat_cocp_delay", "Charge OCP Delay"},
    {"time_bat_cocprd_delay", "Charge OCPR Delay"},
    {"cur_bat_dc_oc", "Discharge OCP"},
    {"time_bat_dc_ocp_delay", "Discharge OCP Delay"},
    {"time_bat_dc_oprd_delay", "Discharge OCPR Delay"},
    {"time_bat_scprd_delay", "SCPR Delay"},
    {"cur_balance_max", "Max Balance-Strom"},
    {"tmp_bat_cot", "Charge OTP"},
    {"tmp_bat_cotpr", "Charge OTPR"},
    {"tmp_bat_dc_ot", "Discharge OTP"},
    {"tmp_bat_dc_otpr", "Discharge OTPR"},
    {"tmp_bat_cut", "Charge UTP"},
    {"tmp_bat_cutpr", "Charge UTPR"},
    {"tmp_mos_ot", "MOS OTP"},
    {"tmp_mos_otpr", "MOS OTPR"},
    {"cell_count", "Zellanzahl"},
    {"switches/bat_charge_enabled", "Laden aktiviert"},
    {"switches/bat_discharge_enabled", "Entladen aktiviert"},
    {"switches/balancing_enabled", "Balancing aktiviert"},
    {"cap_bat_cell", "Zellkapazität"},
    {"scp_delay", "SCP Delay"},
    {"vol_start_balance", "Balance-Start-Spannung"},
    {"dev_address", "Geräteadresse"},
    {"tim_pro_discharge", "Vorentladezeit"},
    {"switches/heating_enabled", "Heizung aktiviert"},
    {"switches/temp_sensor_disabled", "Temperatursensor deaktiviert"},
    {"switches/gps_heartbeat", "GPS-Heartbeat"},
    {"switches/port_switch", "Port"},
    {"switches/lcd_always_on", "LCD immer an"},
    {"switches/special_charger", "Speziallader"},
    {"switches/smart_sleep", "Smart Sleep"},
    {"switches/disable_pcl_module", "PCL-Modul deaktiviert"},
    {"switches/timed_stored_data", "Zeitgespeicherte Daten"},
    {"switches/charging_float_mode", "Ladeerhaltung"},
    {"tmp_heating_start", "Heizungsstart-Temperatur"},
    {"tmp_heating_stop", "Heizungsstopp-Temperatur"},
    {"time_smart_sleep", "Smart-Sleep-Zeit"},
    {"cells/cell_cwr_01", "Konf. Drahtwiderstand 1"},
    {"cells/cell_cwr_02", "Konf. Drahtwiderstand 2"},
    {"cells/cell_cwr_03", "Konf. Drahtwiderstand 3"},
    {"cells/cell_cwr_04", "Konf. Drahtwiderstand 4"},
    {"cells/cell_cwr_05", "Konf. Drahtwiderstand 5"},
    {"cells/cell_cwr_06", "Konf. Drahtwiderstand 6"},
    {"cells/cell_cwr_07", "Konf. Drahtwiderstand 7"},
    {"cells/cell_cwr_08", "Konf. Drahtwiderstand 8"},
    {"cells/cell_cwr_09", "Konf. Drahtwiderstand 9"},
    {"cells/cell_cwr_10", "Konf. Drahtwiderstand 10"},
    {"cells/cell_cwr_11", "Konf. Drahtwiderstand 11"},
    {"cells/cell_cwr_12", "Konf. Drahtwiderstand 12"},
    {"cells/cell_cwr_13", "Konf. Drahtwiderstand 13"},
    {"cells/cell_cwr_14", "Konf. Drahtwiderstand 14"},
    {"cells/cell_cwr_15", "Konf. Drahtwiderstand 15"},
    {"cells/cell_cwr_16", "Konf. Drahtwiderstand 16"},
    {"cells/cell_cwr_17", "Konf. Drahtwiderstand 17"},
    {"cells/cell_cwr_18", "Konf. Drahtwiderstand 18"},
    {"cells/cell_cwr_19", "Konf. Drahtwiderstand 19"},
    {"cells/cell_cwr_20", "Konf. Drahtwiderstand 20"},
    {"cells/cell_cwr_21", "Konf. Drahtwiderstand 21"},
    {"cells/cell_cwr_22", "Konf. Drahtwiderstand 22"},
    {"cells/cell_cwr_23", "Konf. Drahtwiderstand 23"},
    {"cells/cell_cwr_24", "Konf. Drahtwiderstand 24"},
    {"cells/cell_cwr_25", "Konf. Drahtwiderstand 25"},
    {"cells/cell_cwr_26", "Konf. Drahtwiderstand 26"},
    {"cells/cell_cwr_27", "Konf. Drahtwiderstand 27"},
    {"cells/cell_cwr_28", "Konf. Drahtwiderstand 28"},
    {"cells/cell_cwr_29", "Konf. Drahtwiderstand 29"},
    {"cells/cell_cwr_30", "Konf. Drahtwiderstand 30"},
    {"cells/cell_cwr_31", "Konf. Drahtwiderstand 31"},
    {"cells/cell_cwr_32", "Konf. Drahtwiderstand 32"}
};

const MqttPublishField CELL_FIELDS[] = {
    {"readcount", "Frame Counter"},
    {"battery_charged_mAh", "Geladene Kapazität"},
    {"battery_discharged_mAh", "Entladene Kapazität"},
    {"cells_used", "Verwendete Zellen"},
    {"cells/voltage/cell_avg_voltage", "Zellspannung Durchschnitt"},
    {"cells/voltage/cell_diff_voltage", "Zellspannungsdifferenz"},
    {"cells/voltage/high_voltage_cell_nbr", "Zelle mit höchster Spannung"},
    {"cells/voltage/low_voltage_cell_nbr", "Zelle mit niedrigster Spannung"},
    {"cells/voltage/high_voltage_cell_voltage", "Höchste Zellspannung"},
    {"cells/voltage/low_voltage_cell_voltage", "Niedrigste Zellspannung"},
    {"temperatures/temp_mosfet", "MOSFET-Temperatur"},
    {"cell_resistance_alert", "Zellwiderstandsstatus"},
    {"battery_voltage", "Batteriespannung"},
    {"battery_power", "Batterieleistung"},
    {"battery_current", "Batteriestrom"},
    {"battery_power_calculated", "Berechnete Batterieleistung"},
    {"temperatures/temp_sensor1", "Temperatursensor 1"},
    {"temperatures/temp_sensor2", "Temperatursensor 2"},
    {"alarms/alarm_raw", "Alarm Rohwert"},
    {"alarms/alarms_mask", "Alarm Bitmaske"},
    {"alarms/*", "Einzelne Alarmmeldungen"},
    {"balance_current", "Balancing-Strom"},
    {"balance_status", "Balancing-Status"},
    {"battery_soc", "Ladezustand SOC"},
    {"battery_capacity_remaining","Restkapazität"},
    {"battery_capacity_total", "Gesamtkapazität"},
    {"battery_cycle_count", "Zyklen"},
    {"battery_cycle_capacity_total", "Zykluskapazität"},
    {"battery_soh", "Gesundheitszustand SOH"},
    {"battery_precharge_status", "Vorladung"},
    {"battery_user_alarm1", "Benutzeralarm 1"},
    {"battery_total_runtime_sec", "Gesamtlaufzeit Sekunden"},
    {"battery_total_runtime_fmt", "Gesamtlaufzeit formatiert"},
    {"charging_mosfet_status", "Lade-MOSFET"},
    {"discharging_mosfet_status", "Entlade-MOSFET"},
    {"battery_user_alarm2", "Benutzeralarm 2"},
    {"timeDcOCPR", "Entlade-Überstrom Rückkehrzeit"},
    {"timeDcSCPR", "Entlade-Kurzschluss Rückkehrzeit"},
    {"timeCOCPR", "Lade-Überstrom Rückkehrzeit"},
    {"timeCSCPR", "Lade-Kurzschluss Rückkehrzeit"},
    {"timeUVPR", "Unterspannung Rückkehrzeit"},
    {"timeOVPR", "Überspannung Rückkehrzeit"},
    {"temperatures/temp_sensor_absent", "Fehlende Temperatursensoren"},
    {"temperatures/temp_sensor_absent_mask", "Temperatursensor Bitmaske"},
    {"temperatures/battery_heating", "Batterieheizung"},
    {"time_emergency", "Notfallzeit"},
    {"heat_current", "Heizstrom"},
    {"heat_power", "Heizleistung"},
    {"sys_run_ticks", "Systemlaufzeit-Ticks"},
    {"temperatures/temp_sensor3", "Temperatursensor 3"},
    {"temperatures/temp_sensor4", "Temperatursensor 4"},
    {"temperatures/temp_sensor5", "Temperatursensor 5"},
    {"rtc_ticks", "RTC-Ticks"},
    {"time_enter_sleep", "Schlafbeginn"},
    {"pcl_module_status", "PCL-Modulstatus"},
    {"charge_status_time", "Ladestatuszeit"},
    {"charge_status", "Ladestatus"},
    {"dry_contact_1", "Trockenkontakt 1"},
    {"dry_contact_2", "Trockenkontakt 2"},
    {"cells/voltage/cell_v_01", "Zellspannung 1"},
    {"cells/voltage/cell_v_02", "Zellspannung 2"},
    {"cells/voltage/cell_v_03", "Zellspannung 3"},
    {"cells/voltage/cell_v_04", "Zellspannung 4"},
    {"cells/voltage/cell_v_05", "Zellspannung 5"},
    {"cells/voltage/cell_v_06", "Zellspannung 6"},
    {"cells/voltage/cell_v_07", "Zellspannung 7"},
    {"cells/voltage/cell_v_08", "Zellspannung 8"},
    {"cells/voltage/cell_v_09", "Zellspannung 9"},
    {"cells/voltage/cell_v_10", "Zellspannung 10"},
    {"cells/voltage/cell_v_11", "Zellspannung 11"},
    {"cells/voltage/cell_v_12", "Zellspannung 12"},
    {"cells/voltage/cell_v_13", "Zellspannung 13"},
    {"cells/voltage/cell_v_14", "Zellspannung 14"},
    {"cells/voltage/cell_v_15", "Zellspannung 15"},
    {"cells/voltage/cell_v_16", "Zellspannung 16"},
    {"cells/voltage/cell_v_17", "Zellspannung 17"},
    {"cells/voltage/cell_v_18", "Zellspannung 18"},
    {"cells/voltage/cell_v_19", "Zellspannung 19"},
    {"cells/voltage/cell_v_20", "Zellspannung 20"},
    {"cells/voltage/cell_v_21", "Zellspannung 21"},
    {"cells/voltage/cell_v_22", "Zellspannung 22"},
    {"cells/voltage/cell_v_23", "Zellspannung 23"},
    {"cells/voltage/cell_v_24", "Zellspannung 24"},
    {"cells/voltage/cell_v_25", "Zellspannung 25"},
    {"cells/voltage/cell_v_26", "Zellspannung 26"},
    {"cells/voltage/cell_v_27", "Zellspannung 27"},
    {"cells/voltage/cell_v_28", "Zellspannung 28"},
    {"cells/voltage/cell_v_29", "Zellspannung 29"},
    {"cells/voltage/cell_v_30", "Zellspannung 30"},
    {"cells/voltage/cell_v_31", "Zellspannung 31"},
    {"cells/voltage/cell_v_32", "Zellspannung 32"},
    {"cells/wire_res/cell_wr_01", "Drahtwiderstand 1"},
    {"cells/wire_res/cell_wr_02", "Drahtwiderstand 2"},
    {"cells/wire_res/cell_wr_03", "Drahtwiderstand 3"},
    {"cells/wire_res/cell_wr_04", "Drahtwiderstand 4"},
    {"cells/wire_res/cell_wr_05", "Drahtwiderstand 5"},
    {"cells/wire_res/cell_wr_06", "Drahtwiderstand 6"},
    {"cells/wire_res/cell_wr_07", "Drahtwiderstand 7"},
    {"cells/wire_res/cell_wr_08", "Drahtwiderstand 8"},
    {"cells/wire_res/cell_wr_09", "Drahtwiderstand 9"},
    {"cells/wire_res/cell_wr_10", "Drahtwiderstand 10"},
    {"cells/wire_res/cell_wr_11", "Drahtwiderstand 11"},
    {"cells/wire_res/cell_wr_12", "Drahtwiderstand 12"},
    {"cells/wire_res/cell_wr_13", "Drahtwiderstand 13"},
    {"cells/wire_res/cell_wr_14", "Drahtwiderstand 14"},
    {"cells/wire_res/cell_wr_15", "Drahtwiderstand 15"},
    {"cells/wire_res/cell_wr_16", "Drahtwiderstand 16"},
    {"cells/wire_res/cell_wr_17", "Drahtwiderstand 17"},
    {"cells/wire_res/cell_wr_18", "Drahtwiderstand 18"},
    {"cells/wire_res/cell_wr_19", "Drahtwiderstand 19"},
    {"cells/wire_res/cell_wr_20", "Drahtwiderstand 20"},
    {"cells/wire_res/cell_wr_21", "Drahtwiderstand 21"},
    {"cells/wire_res/cell_wr_22", "Drahtwiderstand 22"},
    {"cells/wire_res/cell_wr_23", "Drahtwiderstand 23"},
    {"cells/wire_res/cell_wr_24", "Drahtwiderstand 24"},
    {"cells/wire_res/cell_wr_25", "Drahtwiderstand 25"},
    {"cells/wire_res/cell_wr_26", "Drahtwiderstand 26"},
    {"cells/wire_res/cell_wr_27", "Drahtwiderstand 27"},
    {"cells/wire_res/cell_wr_28", "Drahtwiderstand 28"},
    {"cells/wire_res/cell_wr_29", "Drahtwiderstand 29"},
    {"cells/wire_res/cell_wr_30", "Drahtwiderstand 30"},
    {"cells/wire_res/cell_wr_31", "Drahtwiderstand 31"},
    {"cells/wire_res/cell_wr_32", "Drahtwiderstand 32"}
};

const MqttPublishCategory CATEGORIES[] = {
    {"device", "Device Data", DEVICE_FIELDS, sizeof(DEVICE_FIELDS) / sizeof(DEVICE_FIELDS[0]), true},
    {"config", "Config Data", CONFIG_FIELDS, sizeof(CONFIG_FIELDS) / sizeof(CONFIG_FIELDS[0]), true},
    {"data", "CellData (Livedata)", CELL_FIELDS, sizeof(CELL_FIELDS) / sizeof(CELL_FIELDS[0]), false}
};

uint8_t FIELD_ENABLED[3][160] = {};
uint8_t FIELD_RETAINED[3][160] = {};
bool SETTINGS_LOADED = false;

uint32_t fieldHash(const char *categoryId, const char *suffix)
{
    uint32_t hash = 2166136261u;
    for (const char *part : {categoryId, suffix})
    {
        while (*part != '\0')
        {
            hash ^= static_cast<uint8_t>(*part++);
            hash *= 16777619u;
        }
        hash ^= '/';
        hash *= 16777619u;
    }
    return hash;
}

void makeKey(char *key, size_t keySize, const char *categoryId, const char *suffix)
{
    snprintf(key, keySize, "p%08lx", static_cast<unsigned long>(fieldHash(categoryId, suffix)));
}

void makeRetainKey(char *key, size_t keySize, const char *categoryId, const char *suffix)
{
    snprintf(key, keySize, "r%08lx", static_cast<unsigned long>(fieldHash(categoryId, suffix)));
}

const MqttPublishCategory *findCategory(const char *id)
{
    for (const MqttPublishCategory &category : CATEGORIES)
    {
        if (strcmp(category.id, id) == 0)
            return &category;
    }
    return nullptr;
}

bool findField(const char *categoryId, const char *suffix, size_t &categoryIndex, size_t &fieldIndex)
{
    for (size_t i = 0; i < sizeof(CATEGORIES) / sizeof(CATEGORIES[0]); i++)
    {
        if (strcmp(CATEGORIES[i].id, categoryId) != 0)
            continue;
        for (size_t j = 0; j < CATEGORIES[i].fieldCount; j++)
        {
            if (strcmp(CATEGORIES[i].fields[j].suffix, suffix) == 0)
            {
                categoryIndex = i;
                fieldIndex = j;
                return true;
            }
        }
    }
    return false;
}

void loadSettings()
{
    if (SETTINGS_LOADED)
        return;

    Preferences prefs;
    if (prefs.begin(NVS_NAMESPACE, true))
    {
        for (size_t i = 0; i < sizeof(CATEGORIES) / sizeof(CATEGORIES[0]); i++)
        {
            uint8_t defaultRetained = CATEGORIES[i].defaultRetained ? 1 : 0;
            for (size_t j = 0; j < CATEGORIES[i].fieldCount; j++)
            {
                char key[16];
                makeKey(key, sizeof(key), CATEGORIES[i].id, CATEGORIES[i].fields[j].suffix);
                FIELD_ENABLED[i][j] = prefs.getUChar(key, 1);

                char retainKey[16];
                makeRetainKey(retainKey, sizeof(retainKey), CATEGORIES[i].id, CATEGORIES[i].fields[j].suffix);
                FIELD_RETAINED[i][j] = prefs.getUChar(retainKey, defaultRetained);
            }
        }
        prefs.end();
    }
    else
    {
        for (size_t i = 0; i < sizeof(CATEGORIES) / sizeof(CATEGORIES[0]); i++)
        {
            uint8_t defaultRetained = CATEGORIES[i].defaultRetained ? 1 : 0;
            for (size_t j = 0; j < CATEGORIES[i].fieldCount; j++)
            {
                FIELD_ENABLED[i][j] = 1;
                FIELD_RETAINED[i][j] = defaultRetained;
            }
        }
    }
    SETTINGS_LOADED = true;
}
}

const MqttPublishCategory *getMqttPublishCategories(size_t &categoryCount)
{
    categoryCount = sizeof(CATEGORIES) / sizeof(CATEGORIES[0]);
    return CATEGORIES;
}

bool getMqttPublishFieldEnabled(const char *categoryId, const char *suffix)
{
    loadSettings();
    size_t categoryIndex = 0;
    size_t fieldIndex = 0;
    if (!findField(categoryId, suffix, categoryIndex, fieldIndex))
        return true;
    return FIELD_ENABLED[categoryIndex][fieldIndex] != 0;
}

void setMqttPublishFieldEnabled(const char *categoryId, const char *suffix, bool enabled)
{
    loadSettings();
    size_t categoryIndex = 0;
    size_t fieldIndex = 0;
    if (!findField(categoryId, suffix, categoryIndex, fieldIndex))
        return;

    char key[16];
    makeKey(key, sizeof(key), categoryId, suffix);
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false))
        return;
    prefs.putUChar(key, enabled ? 1 : 0);
    prefs.end();
    FIELD_ENABLED[categoryIndex][fieldIndex] = enabled ? 1 : 0;
}

bool isMqttPublishFieldEnabled(const char *topic)
{
    if (topic == nullptr)
        return false;

    const char *categoryId = nullptr;
    const char *suffix = nullptr;
    for (const MqttPublishCategory &category : CATEGORIES)
    {
        String marker = String("/") + category.id + "/";
        const char *found = strstr(topic, marker.c_str());
        if (found != nullptr)
        {
            categoryId = category.id;
            suffix = found + marker.length();
            break;
        }
    }

    if (categoryId == nullptr || suffix == nullptr || *suffix == '\0')
        return true;

    if (strncmp(suffix, "alarms/", 7) == 0 && strcmp(suffix, "alarms/alarm_raw") != 0 && strcmp(suffix, "alarms/alarms_mask") != 0)
        return getMqttPublishFieldEnabled(categoryId, "alarms/*");

    return getMqttPublishFieldEnabled(categoryId, suffix);
}

bool getMqttPublishFieldRetained(const char *categoryId, const char *suffix)
{
    loadSettings();
    size_t categoryIndex = 0;
    size_t fieldIndex = 0;
    if (!findField(categoryId, suffix, categoryIndex, fieldIndex))
        return false;
    return FIELD_RETAINED[categoryIndex][fieldIndex] != 0;
}

void setMqttPublishFieldRetained(const char *categoryId, const char *suffix, bool retained)
{
    loadSettings();
    size_t categoryIndex = 0;
    size_t fieldIndex = 0;
    if (!findField(categoryId, suffix, categoryIndex, fieldIndex))
        return;

    char key[16];
    makeRetainKey(key, sizeof(key), categoryId, suffix);
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false))
        return;
    prefs.putUChar(key, retained ? 1 : 0);
    prefs.end();
    FIELD_RETAINED[categoryIndex][fieldIndex] = retained ? 1 : 0;
}

bool isMqttPublishFieldRetained(const char *topic)
{
    if (topic == nullptr)
        return false;

    const char *categoryId = nullptr;
    const char *suffix = nullptr;
    for (const MqttPublishCategory &category : CATEGORIES)
    {
        String marker = String("/") + category.id + "/";
        const char *found = strstr(topic, marker.c_str());
        if (found != nullptr)
        {
            categoryId = category.id;
            suffix = found + marker.length();
            break;
        }
    }

    if (categoryId == nullptr || suffix == nullptr || *suffix == '\0')
        return false;

    if (strncmp(suffix, "alarms/", 7) == 0 && strcmp(suffix, "alarms/alarm_raw") != 0 && strcmp(suffix, "alarms/alarms_mask") != 0)
        return getMqttPublishFieldRetained(categoryId, "alarms/*");

    return getMqttPublishFieldRetained(categoryId, suffix);
}
