# MQTT Topics Compact

Diese Datei enthaelt eine kompakte Auswahl der wichtigsten 20 Topics fuer Monitoring und schnellen Health-Check.

Basis:
- BASE = TOPIC_BASE + DEVICENAME

## Top 20 Topics

1. BASE/status/status
2. BASE/status/uptime
3. BASE/status/ipaddress
4. BASE/status/wifi_rssi
5. BASE/status/ble_connection
6. BASE/device/device_name
7. BASE/device/sw_version
8. BASE/device/uptime_fmt
9. BASE/data/battery_voltage
10. BASE/data/battery_current
11. BASE/data/battery_power
12. BASE/data/battery_soc
13. BASE/data/battery_soh
14. BASE/data/battery_capacity_remaining
15. BASE/data/battery_total_runtime_fmt
16. BASE/data/temperatures/temp_mosfet
17. BASE/data/temperatures/temp_sensor1
18. BASE/data/alarms/alarm_raw
19. BASE/data/alarms/alarms_mask
20. BASE/config/cell_count

## Optional fuer tieferes Monitoring

- BASE/data/cells/voltage/cell_v_01 ... cell_v_32
- BASE/data/cells/voltage/cell_diff_voltage
- BASE/data/temperatures/temp_sensor_absent
- BASE/status/maxpubqueue
- BASE/status/rawdata_drop_queue_full
