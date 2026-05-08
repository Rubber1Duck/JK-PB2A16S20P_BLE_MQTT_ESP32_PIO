# MQTT Topics

Stand: aus dem aktuellen Code in src/parser.cpp und src/mqtt_handler.cpp.

## Basis

- Basis pro Geraet: TOPIC_BASE + DEVICENAME
- In den Beispielen unten steht BASE = TOPIC_BASE + DEVICENAME
- Last-Will Topic: BASE/status/status

## Parameter Topics (Publish + Subscribe)

- BASE/parameter/debugging_active
- BASE/parameter/debugging_active_full
- BASE/parameter/publish_delay
- BASE/parameter/min_publish_time
- BASE/parameter/publish_interval

## Status Topics

- BASE/status/version
- BASE/status/ipaddress
- BASE/status/ble_connection
- BASE/status/status
- BASE/status/uptime
- BASE/status/rawpool_free_slots
- BASE/status/rawpool_capacity
- BASE/status/rawdata_enqueued
- BASE/status/rawdata_drop_init_failed
- BASE/status/rawdata_drop_oversize
- BASE/status/rawdata_drop_pool_exhausted
- BASE/status/rawdata_drop_queue_full
- BASE/status/maxpubqueue
- BASE/status/wifi_rssi
- BASE/status/MinFreeHeap
- BASE/status/parser_frames_30s
- BASE/status/parser_heap_before_min
- BASE/status/parser_heap_after_min
- BASE/status/parser_heap_delta_max
- BASE/status/parser_heap_delta_avg
- BASE/status/ble_device_mac
- BASE/status/ble_device_rssi

## Device Topics

- BASE/device/read_count
- BASE/device/vendor_id
- BASE/device/hw_revision
- BASE/device/sw_version
- BASE/device/uptime
- BASE/device/uptime_fmt
- BASE/device/power_up_times
- BASE/device/device_name
- BASE/device/device_passwd
- BASE/device/manufacturing_date
- BASE/device/serial_number
- BASE/device/passcode (nur wenn V19 nicht definiert ist)
- BASE/device/user_data
- BASE/device/setup_passcode
- BASE/device/user_data2
- BASE/device/rcv_time
- BASE/device/rfv_time

## Device Topics (optional, wenn PROTOCOL_NUMBERS_AND_ENABLE_STATUS)

- BASE/device/uart1_protocol_number
- BASE/device/uart1_protocol_txt
- BASE/device/uart1_protocol_enable
- BASE/device/can_protocol_number
- BASE/device/can_protocol_txt
- BASE/device/uart_protocol_enable_0_15
- BASE/device/uart2_protocol_number
- BASE/device/uart2_protocol_txt
- BASE/device/uart2_protocol_enable
- BASE/device/uart_protocol_lib_version

## Device Topics (optional, wenn LCD_AND_DRY_TRIGGER_VALUES)

- BASE/device/lcd_buzzer_trigger
- BASE/device/lcd_buzzer_trigger_txt
- BASE/device/lcd_buzzer_trigger_value
- BASE/device/lcd_buzzer_release_value
- BASE/device/dry1_trigger
- BASE/device/dry1_trigger_txt
- BASE/device/dry1_trigger_value
- BASE/device/dry1_release_value
- BASE/device/dry2_trigger
- BASE/device/dry2_trigger_txt
- BASE/device/dry2_trigger_value
- BASE/device/dry2_release_value
- BASE/device/can_protocol_lib_version

## Data Topics

- BASE/data/readcount
- BASE/data/battery_charged_mAh
- BASE/data/battery_discharged_mAh
- BASE/data/cells_used (nur wenn debug_flg)
- BASE/data/cells/voltage/cell_avg_voltage
- BASE/data/cells/voltage/cell_diff_voltage
- BASE/data/cells/voltage/high_voltage_cell
- BASE/data/cells/voltage/low_voltage_cell
- BASE/data/temperatures/temp_mosfet
- BASE/data/cell_resistance_alert (nur wenn debug_flg)
- BASE/data/battery_voltage
- BASE/data/battery_power
- BASE/data/battery_current
- BASE/data/battery_power_calculated
- BASE/data/temperatures/temp_sensor1
- BASE/data/temperatures/temp_sensor2
- BASE/data/alarms/alarm_raw
- BASE/data/alarms/alarms_mask (nur wenn debug_flg)
- BASE/data/balance_current
- BASE/data/balance_status
- BASE/data/battery_soc
- BASE/data/battery_capacity_remaining
- BASE/data/battery_capacity_total
- BASE/data/battery_cycle_count
- BASE/data/battery_cycle_capacity_total
- BASE/data/battery_soh
- BASE/data/battery_precharge_status
- BASE/data/battery_user_alarm1
- BASE/data/battery_total_runtime_sec
- BASE/data/battery_total_runtime_fmt
- BASE/data/charging_mosfet_status
- BASE/data/discharging_mosfet_status
- BASE/data/battery_user_alarm2
- BASE/data/timeDcOCPR
- BASE/data/timeDcSCPR
- BASE/data/timeCOCPR
- BASE/data/timeCSCPR
- BASE/data/timeUVPR
- BASE/data/timeOVPR
- BASE/data/temperatures/temp_sensor_absent
- BASE/data/temperatures/temp_sensor_absent_mask (nur wenn debug_flg)
- BASE/data/temperatures/battery_heating
- BASE/data/time_emergency
- BASE/data/heat_current
- BASE/data/sys_run_ticks
- BASE/data/temperatures/temp_sensor3
- BASE/data/temperatures/temp_sensor4
- BASE/data/temperatures/temp_sensor5
- BASE/data/rtc_ticks

## Data Topics (dynamische Reihen)

- BASE/data/cells/voltage/cell_v_01 bis BASE/data/cells/voltage/cell_v_32
- BASE/data/cells/resistance/cell_r_01 bis BASE/data/cells/resistance/cell_r_32
- BASE/data/alarms/AlarmWireRes
- BASE/data/alarms/AlarmMosOTP
- BASE/data/alarms/AlarmCellQuantity
- BASE/data/alarms/AlarmCurSensorErr
- BASE/data/alarms/AlarmCellOVP
- BASE/data/alarms/AlarmBatOVP
- BASE/data/alarms/AlarmChOCP
- BASE/data/alarms/AlarmChSCP
- BASE/data/alarms/AlarmChOTP
- BASE/data/alarms/AlarmChUTP
- BASE/data/alarms/AlarmCPUAuxCommuErr
- BASE/data/alarms/AlarmCellUVP
- BASE/data/alarms/AlarmBatUVP
- BASE/data/alarms/AlarmDchOCP
- BASE/data/alarms/AlarmDchSCP
- BASE/data/alarms/AlarmDchOTP
- BASE/data/alarms/AlarmChargeMOS
- BASE/data/alarms/AlarmDischargeMOS
- BASE/data/alarms/AlarmGPSDisconneted
- BASE/data/alarms/AlarmModifyPWD_in_time
- BASE/data/alarms/AlarmDischargeOnFailed
- BASE/data/alarms/BatteryOverTempAlarm
- BASE/data/alarms/TemperatureSensorAnomaly
- BASE/data/alarms/AlarmPLCModuleAnomaly
- BASE/data/temperatures/MOS_TempSensorAbsent
- BASE/data/temperatures/BATTempSensor1Absent
- BASE/data/temperatures/BATTempSensor2Absent
- BASE/data/temperatures/BATTempSensor3Absent
- BASE/data/temperatures/BATTempSensor4Absent
- BASE/data/temperatures/BATTempSensor5Absent

## Debug Topics

- BASE/debug/enabled
- BASE/debug/rawdata

Hinweis: BASE/debug/rawdata wird bei debug_flg_full als Base64-Rohdaten publiziert, sonst als Text not published.

## Config Topics

- BASE/config/vol_smart_sleep
- BASE/config/vol_cell_uv
- BASE/config/vol_cell_uvpr
- BASE/config/vol_cell_ov
- BASE/config/vol_cell_ovpr
- BASE/config/vol_balan_trig
- BASE/config/vol_100_percent
- BASE/config/vol_0_percent
- BASE/config/vol_cell_rcv
- BASE/config/vol_cell_rfv
- BASE/config/vol_sys_pwr_off
- BASE/config/cur_bat_coc
- BASE/config/time_bat_cocp_delay
- BASE/config/time_bat_cocprd_delay
- BASE/config/cur_bat_dc_oc
- BASE/config/time_bat_dc_ocp_delay
- BASE/config/time_bat_dc_oprd_delay
- BASE/config/time_bat_scprd_delay
- BASE/config/cur_balance_max
- BASE/config/tmp_bat_cot
- BASE/config/tmp_bat_cotpr
- BASE/config/tmp_bat_dc_ot
- BASE/config/tmp_bat_dc_otpr
- BASE/config/tmp_bat_cut
- BASE/config/tmp_bat_cutpr
- BASE/config/tmp_mos_ot
- BASE/config/tmp_mos_otpr
- BASE/config/cell_count
- BASE/config/switches/bat_charge_enabled
- BASE/config/switches/bat_discharge_enabled
- BASE/config/switches/balancing_enabled
- BASE/config/cap_bat_cell
- BASE/config/scp_delay
- BASE/config/vol_start_balance
- BASE/config/dev_address
- BASE/config/tim_pro_discharge
- BASE/config/switches/heating_enabled
- BASE/config/switches/temp_sensor_disabled
- BASE/config/switches/gps_heartbeat
- BASE/config/switches/port_switch
- BASE/config/switches/lcd_always_on
- BASE/config/switches/special_charger
- BASE/config/switches/smart_sleep
- BASE/config/switches/disable_pcl_module
- BASE/config/switches/timed_stored_data
- BASE/config/switches/charging_float_mode
- BASE/config/tmp_heating_start
- BASE/config/tmp_heating_stop
- BASE/config/time_smart_sleep
