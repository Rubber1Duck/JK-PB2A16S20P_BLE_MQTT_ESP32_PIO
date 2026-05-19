# SSL/TLS MQTT Error Troubleshooting Guide

## Error: `[E][ssl_client.cpp:37] (-29056) SSL - Verification of the message MAC failed`

This error occurs when the ESP32 cannot verify the SSL/TLS handshake with the MQTT broker.

### Root Causes and Solutions

#### 1. **System Time Not Synchronized (MOST COMMON)**
**Symptoms:**
- Error occurs immediately after WiFi connects
- Error repeats every reconnect attempt
- No "NTP time sync successful" message in serial output

**Solution:**
The fix has been implemented! The code now waits for NTP synchronization before MQTT connection.

**Verify it's working:**
- Check serial output for: `"Waiting for NTP time synchronization..."`
- Then: `"NTP time sync successful: ..."`
- If these messages don't appear, check:
  - `NTPSERVER` is defined in `config.h`
  - WiFi connection is stable
  - DNS resolution is working

#### 2. **Invalid or Expired ROOT CA Certificate**
**Symptoms:**
- Error persists even after time sync
- Happens consistently with same certificate

**Solution:**
Update the ROOT CA certificate:
- For HiveMQ Cloud, download latest from: https://docs.hivemq.com/mqtt-cloud/mqtt-cloud-certificate.html
- Convert to PEM format if needed: https://cert2arduino.netlify.app/
- Update `MQTT_ROOT_CA_CERT` in `include/config.h`

#### 3. **DNS/Network Issues**
**Symptoms:**
- Connection times out frequently
- Certificate verification fails intermittently

**Solution:**
- Check DNS settings in `config.h` (USE_WIFI_STATIC_IP section)
- Try alternative NTP servers:
  - `"time.nist.gov"`
  - `"time.google.com"`

#### 4. **Hostname Verification Issues (Less Common)**
**Symptoms:**
- Certificate verification fails despite correct time
- Error message mentions certificate details

**Solution:**
As a debug workaround, uncomment in `config.h`:
```c
#define MQTT_SKIP_CERT_VERIFY
```

**WARNING:** This disables hostname verification. Only use for debugging!

---

## Debugging Steps

### Step 1: Check Serial Output
Monitor the serial console (115200 baud) for these messages:

```
JK-BMS Listener V ...
Starting ...
Zertifikat erfolgreich in PSRAM kopiert.
NTP-Time synced
Current time: ...
Waiting for NTP time synchronization...
Waiting for time sync... (1000 ms elapsed)
Waiting for time sync... (2000 ms elapsed)
NTP time sync successful: ...
Attempting MQTT connection...
Connected to broker...
```

### Step 2: Verify Time Sync
If time sync is failing:
1. Check WiFi connection: `isWifiConnected` should be `true`
2. Verify NTP server is reachable
3. Check network DNS is working

### Step 3: Test Certificate
To verify the ROOT CA certificate is correct:
1. Extract the certificate from `config.h`
2. Convert from C string format to standard PEM
3. Compare with official certificate from broker provider
4. Check certificate validity dates

### Step 4: Enable Debug Logging
Set in `config.h`:
```c
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
```

---

## Additional Resources

- **HiveMQ Cloud Documentation**: https://docs.hivemq.com/
- **ESP32 WiFiClientSecure**: https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiClientSecure.h
- **PubSubClient MQTT Library**: https://github.com/knolleary/pubsubclient

## If Problem Persists

1. Check your MQTT broker credentials in `config.h`
2. Verify broker URL and port (8883 for HiveMQ Cloud TLS)
3. Test connection with external MQTT client tool
4. Check ESP32 heap memory (`Serial.printf("Free heap: %d\n", ESP.getFreeHeap());`)
5. Consider increasing buffer size or reducing debug output

---

**Last Updated:** 2026-05-19  
**Project:** JK-PB2A16S20P BLE MQTT ESP32
