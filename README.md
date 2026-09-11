# JK Inverter BMS Bluetooth to MQTT gateway

## Description
This project is initaly made by Waldmensch1 (see [Waldmensch1/JK-PB2A16S20P_BLE_MQTT_ESP32_PIO](https://github.com/Waldmensch1/JK-PB2A16S20P_BLE_MQTT_ESP32_PIO). Since he apparently no longer maintains his repo, I have decided to continue the project in this fork.

Thanks Waldmensch1 for your grat work!

This project is inspired by the Akkudoktor.net forum and especially by this thread: [JKBMS auslesen über BLE Bluetooth oder RS485 Adapter mittels EPS IoBroker](https://akkudoktor.net/t/jkbms-auslesen-uber-ble-bluetooth-oder-rs485-adapter-mittels-eps-iobroker/722). As the code there did not work properly with my JK-PB2A16S20P (these are delivered with a lot of DIY battery boxes), this project has been raised. For details on supported JK inverter BMS models see [table below](#supported-jk-bms-models).  
The communication between the BMS and the ESP32 is primarily designed for bluetooth, thus an ESP32 model with BT support is required.  
If you use more than one ESP32 it is shown as a structure in MQTT:  
```
jk_ble_listener
    +---DEVICENAME1
    |
    +---DEVICENAME2
```

!!! ATTENTION !!!
This projekt will READ all values only! I will not support writing values ​​to the BMS! Not now, and not in the future either.


## Table of Contents
- [Installation](#installation)
- [Build Troubleshooting](#build-troubleshooting)
- [Usage](#usage)
- [Supported JK BMS Models](#supported-jk-bms-models)
- [Contributing](#contributing)
- [License](#license)

## Installation
Use VS-Code with [PlatformIO](https://platformio.org/install/ide?install=vscode) Add-On to build and flash the firmware.

## Build Troubleshooting
If your build fails in `ESPAsyncWebServer/src/AsyncJson.cpp` with an error like
`JsonHandlerTypes was not declared in this scope`, pin `ESPAsyncWebServer` in
`platformio.ini` to a known working version:

```ini
ESP32Async/ESPAsyncWebServer@3.7.3
```

This project currently keeps that pin in `lib_deps` to ensure reproducible builds
across all environments.

## Usage
- Create a config.h file in folder includes. A sample file exist, you can modify and rename it. The available configuration options are well documented in the sample file.
- Modify the platformio.ini for your needs. Especially set the `DEVICENAME` (e.g. `JK-PB2A16S20P-01`), which is the Bluetooth name **as shown by the JK-BMS smartphone app**. This overwrites the `DEVICENAME` specified in `/include/config.h`. This will allow you to build targets for multiple ESP32 boards
- Specify COM ports in platformio.ini; may be deleted to enabled auto-detect (if you only have one ESP connected to your host)
- Build and upload to your ESP32
- a build in Webserver is providing a simple startpage under http://xxx.xxx.xxx.xxx/. At the botton are links to a MQTT config Page, OTA update and to a page witch shows the last resetreasons (to see if the ESP crashes some time) 

**Attention:**
- because of using WIFI and BLE (witch is used over the same antenna on ESP32) a ESP32 with "good" quality is recommended
- If you want to use MQTT with TLS a "normal" ESP32 is not suitable! Tests with an ESP32-S3-N16R8 were successful; the ESP run for several days with a TLS connection. No reboots or crashes occurred. (on "normal" ESP32 the WIFI connection is very unstable!)
- using a ESP32 with additional PSRAM is also recommended if you are in a "unstable" Wifi environment, because then 3/4 of the PS RAM will be used to cache the MQTT values. On my esp32-S3-N16R8 with 8MB PSRAM 6MB is used for the publish queue, that is place for 24472 entrys. Depending on how many values ​​are selected for publishing (on the MQTT configuration webpage), the settings for `publish_delay` and `min_pub_time`, and the number of cells installed, the buffer lasts for up to three hours. For example, I am currently using a 4-cell battery setup where all device info and configuration info values ​​are published every 300 sec, along with 15 values ​​from the live data every 5 sec.(cell data). With the default settings of `publish_delay` = 5 and `min_pub_time` = 300, this results in an average of 89 messages per minute. The cache is sufficient for approximately 275 minutes! Since all values ​​are published with a timestamp, they can be written to a database (e.g., InfluxDB) with the correct timestamps even after the connection is restored. (However, clearing a full cache takes over 17 minutes, depending on the PUBLISH_INTERVAL value, which defaults to 40 (every 40 ms). Do not lower this under 25 ms! this will result in instability issues!)
  
**Attention:** Do note that you will not be able to connect to the BMS with your smartphone app while the ESP32 is communicating with your BMS.

## Supported JK BMS Models
This project has been primarily designed to be used for the JK-PB2A16S20P, however it may work with different models of the **JK PB BMS and B BMS series**, all BMS witch use the so called **JK02_32S** protocol should run.  
Here is a list of currently evaluated BMS models along with the tested hard- and firmware revisions:

| Model         | HW-Rev. | FW-Ver | Status |
| ------------- | ------- | ------ | ------ |
| JK-B2A8S20P   | 11XW    | 11.288 | supported |
| JK-PB2A16S20P | 15A     | 15.37  | supported |
| JK-PB2A16S15P | 15A     | 15.38  | supported |
| JK-PB2A16SxxP | 15H     | 15.41  | supported |
| JK-B2A8S20P   | 19H     | 19.26  | supported (*)|
| JK-PB2A16S20P | 19A     | 19.26  | supported (*)|

(*) for V19 BMS uncomment "//define V19" in config file

Hardware rev. 14 and 15 of the JK inverter BMS will most probably also work with a recent firmware installed.  
**Note:** The most promising precondition to successfully use this project is probably by running a **recent firmware** on your BMS. You may check [Andy's homepage](https://off-grid-garage.com/battery-management-systems-bms/) for firmware updates for the JK inverter BMS series.

## Contributing
The parsing is been done mostly aligned to this protocol documentation https://github.com/syssi/esphome-jk-bms/blob/main/docs/protocol-design-ble.md but there may be some glitches in parsing. Feel free to contribute

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.