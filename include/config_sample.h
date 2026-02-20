#include "secrets.h"
////////////////////////////// Settings //////////////////////////////

// if devicename is not defined in platformio.ini, use this default
#ifndef DEVICENAME
#define DEVICENAME "JK-PB2A16S20P"
#endif

// Uncomment if Hardware is V19
// #define V19

// Uncomment to pulish protocol numbers and enable status from device info frame
// #define PROTOCOL_NUMBERS_AND_ENABLE_STATUS

// uncomment to publish trigger values for LCD buzzer and dry contacts from device info frame
// #define LCD_AND_DRY_TRIGGER_VALUES

// WiFi credentials
// #define USE_WIFI_STATIC_IP // uncomment to use static IP configuration instead of DHCP (see below for settings)

#define SSID_NAME SECRET_WIFI_SSID
#define SSID_PASSWORD SECRET_WIFI_PASSWORD

#ifdef USE_WIFI_STATIC_IP
#define IP_ADDRESS "" // optional static IP address, leave empty for DHCP
#define GATEWAY ""    // optional gateway for static IP, leave empty for DHCP
#define SUBNET ""     // optional subnet mask for static IP, leave empty for DHCP
#define DNS ""        // optional DNS server for static IP, leave empty for DHCP
#define DNS2 ""       // optional secondary DNS server for static IP, leave empty for DHCP
#endif

// MQTT Settings
// #define USE_TLS // uncomment to enable TLS support for MQTT

#define MQTT_SERVER "" // ip or dns name of your MQTT broker
#define MQTT_USERNAME SECRET_MQTT_USERNAME
#define MQTT_PASSWORD SECRET_MQTT_PASSWORD
#ifndef USE_TLS
#define MQTT_PORT 1883
#else // USE_TLS
#define MQTT_TLS_PORT 8883
// ROOT CA certificate for MQTT TLS connection (PEM format).
// You can obtain this from your MQTT broker provider or generate it yourself if you are running your own broker with TLS support.
// For example, for free HiveMQTT broker, you can download the certificate from their website and convert it to PEM format if needed.
// to bring the certificate into a single line string,
// you can use online tools (https://cert2arduino.netlify.app/) or command line utilities like `awk` or `sed` to replace newlines with `\n`
// and add the necessary quotes for C++ string literals.
// Example ROOT CA certificate (here HiveMQ Service) (replace with your needs):
#define MQTT_ROOT_CA_CERT                                              \
  "-----BEGIN CERTIFICATE-----\n"                                      \
  "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\n" \
  "-----END CERTIFICATE-----\n";
#endif

// MQTT Topic configuration
#define TOPIC_BASE "jk_ble_listener/" // Base topic tree used for all MQTT communication; ATTN: must end with a "/"!

// this attached a unique id to the mqtt client name. This is useful if
// you have multiple devices with the same name. It collides with the
// retained messages on the broker, so last will messages are not working
// Uncomment only if you have problems connecting to the broker
// #define USE_RANDOM_CLIENT_ID

// reboot after BLE scans without success
#define REBOOT_AFTER_BLE_RETRY 20

// only publish every this seconds (0 -> publish every change immediately)
#define PUBLISH_DELAY 0

// publish values also if they are not changed. 0 = off, n = seconds
#define MIN_PUB_TIME 300

// Differential voltage publishing
#define DIFFV_DIVIDER 1000 // Set to 1000 to get differential cell voltage in V or to 1 for mV

// NTP Configuration
#define NTPSERVER "pool.ntp.org"              // NTP support disabled if not defined; may be IP or DNS name
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3" // Germany Timezone including DST rules (see: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html)
// #define GMTOFFSET 3600  // OPTIONAL config w/o TIMEZONE (CET, UTC+1)
// #define DLOFFSET 3600   // OPTIONAL config w/o TIMEZONE (CEST, UTC+2)

// #define USE_INFLUXDB

#ifdef USE_INFLUXDB
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
// InfluxDB 1.8+  (v2 compatibility API) server url, e.g. http://192.168.1.48:8086
#define INFLUXDB_URL "http://192.168.178.195:8086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
// InfluxDB 1.8+ (v2 compatibility API) use form user:password, eg. admin:adminpass
#define INFLUXDB_TOKEN "your token"
// InfluxDB v2 organization name or id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
// InfluxDB 1.8+ (v2 compatibility API) use any non empty string
#define INFLUXDB_ORG "your org"
// InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
// InfluxDB 1.8+ (v2 compatibility API) use database name
#define INFLUXDB_BUCKET "your bucket"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// #define INFLUX_TEMP_SENSOR_1
// #define INFLUX_TEMP_SENSOR_2
// #define INFLUX_TEMP_SENSOR_3
// #define INFLUX_TEMP_SENSOR_4
// #define INFLUX_TEMP_SENSOR_5
// #define INFLUX_TEMP_SENSOR_MOSFET
// #define INFLUX_BATTERY_VOLTAGE
// #define INFLUX_BATTERY_CURRENT
// #define INFLUX_BATTERY_POWER
// #define INFLUX_BATTERY_SOC
// #define INFLUX_CELLS_VOLTAGE

#endif

// This is not ready and todo. Leave that commented out
// #define USE_RS485

#ifdef USE_RS485
#define RS485_RX 16
#define RS485_TX 17
#define DE_PIN 4
#define RE_PIN 2
#endif