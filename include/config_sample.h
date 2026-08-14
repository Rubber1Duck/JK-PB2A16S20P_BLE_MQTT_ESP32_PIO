#ifndef CONFIG_H
#define CONFIG_H
////////////////////////////// Settings //////////////////////////////

// if devicename is not defined in platformio.ini, use this default
#ifndef DEVICENAME
#define DEVICENAME "JK-PB2A16S20P"
#endif

// Uncomment to enable debug output to syslog server (UDP) and serial console
// #define USE_SYSLOG
#ifdef USE_SYSLOG
#define SYSLOG_SERVER "192.168.178.52"
#define SYSLOG_PORT 514
#endif

// Uncomment if Hardware is V19
// #define V19

// Uncomment to pulish protocol numbers and enable status from device info frame
// #define PROTOCOL_NUMBERS_AND_ENABLE_STATUS

// uncomment to publish trigger values for LCD buzzer and dry contacts from device info frame
// #define LCD_AND_DRY_TRIGGER_VALUES

// Number of reset reasons to store in history, adjust as needed but be careful with NVS storage limits
// (see: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#nvs-flash)
#define MAX_RESET_REASONS 10

// Uncomment to enable webserver and OTA update functionality
#define USE_WEBSERVER

// WiFi credentials
// #define USE_WIFI_STATIC_IP // uncomment to use static IP configuration instead of DHCP (see below for settings)

#define SSID_NAME "your ssid"
#define SSID_PASSWORD "your password"

#ifdef USE_WIFI_STATIC_IP
#define IP_ADDRESS "" // optional static IP address, leave empty for DHCP
#define GATEWAY ""    // optional gateway for static IP, leave empty for DHCP
#define SUBNET ""     // optional subnet mask for static IP, leave empty for DHCP
#define DNS ""        // optional DNS server for static IP, leave empty for DHCP
#define DNS2 ""       // optional secondary DNS server for static IP, leave empty for DHCP
#endif

// MQTT Settings
// #define USE_TLS // uncomment to enable TLS support for MQTT

#define MQTT_SERVER "ip or dns name of your MQTT broker"
#define MQTT_USERNAME "your MQTT username"
#define MQTT_PASSWORD "your MQTT password"
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
  "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n" \
  "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
  "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n" \
  "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n" \
  "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n" \
  "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n" \
  "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n" \
  "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n" \
  "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n" \
  "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n" \
  "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n" \
  "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n" \
  "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n" \
  "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n" \
  "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n" \
  "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n" \
  "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n" \
  "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n" \
  "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" \
  "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n" \
  "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n" \
  "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n" \
  "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n" \
  "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n" \
  "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n" \
  "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n" \
  "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n" \
  "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n" \
  "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" \
  "-----END CERTIFICATE-----\n";
#endif

// MQTT Topic configuration
#define TOPIC_BASE "jk_ble_listener/" // Base topic tree used for all MQTT communication; ATTN: must end with a "/"!

// Optional: Home Assistant MQTT Auto-Discovery
// Default is OFF. Uncomment to publish HA discovery config topics on MQTT connect.
// #define USE_HA_DISCOVERY

// Optional custom discovery prefix (default: "homeassistant")
// #define HA_DISCOVERY_PREFIX "homeassistant"

// this attached a unique id to the mqtt client name. This is useful if
// you have multiple devices with the same name. It collides with the
// retained messages on the broker, so last will messages are not working
// Uncomment only if you have problems connecting to the broker
// #define USE_RANDOM_CLIENT_ID

// only publish every this seconds (0 -> publish every change immediately) default: 5s.
// ATTN: 0 is possible but thats 4 DataFrames per second and the device might not be able to keep up with the load of publishing too many messages!
// for 0 the publish interval (see below) must be set to 35 or lower to prevent the queue from filling up.
// But maybe this will cause to stability issues with the MQTT client!
#define PUBLISH_DELAY 5

// time between publish attempts in milliseconds, can be adjust via MQTT, default is 50ms,
// which means max 20 publishes per second, adjust if you have a lot of messages to publish and the queue is filling up,
// but be careful with too low values as it can cause stability issues with the MQTT client
#define PUBLISH_INTERVAL 50

// publish values also if they are not changed. 0 = off, n = seconds
#define MIN_PUB_TIME 300

// Differential voltage publishing
#define DIFFV_DIVIDER 1000 // Set to 1000 to get differential cell voltage in V or to 1 for mV

// NTP Configuration
#define NTPSERVER "pool.ntp.org"              // NTP support disabled if not defined; may be IP or DNS name
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3" // Germany Timezone including DST rules (see: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html)
// #define GMTOFFSET 3600  // OPTIONAL config w/o TIMEZONE (CET, UTC+1)
// #define DLOFFSET 3600   // OPTIONAL config w/o TIMEZONE (CEST, UTC+2)

// SSL/TLS Configuration for MQTT
// Uncomment if you have SSL certificate verification issues (e.g., "SSL - Verification of the message MAC failed")
// Debug only: this disables certificate verification entirely (insecure)
// #define MQTT_SKIP_CERT_VERIFY

///////////////////////////// End Settings //////////////////////////////////
#endif // CONFIG_H