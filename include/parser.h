#ifndef PARSER_H
#define PARSER_H

#include "config.h"
#include "settings.h"
#include <Arduino.h>
#include "mqtt_handler.h"
#include <map>
#include "time.h"
#include "data_struct_device_data.h" // DeviceData structure
#include "data_struct_cell_data.h"   // CellData structure
#include "data_struct_config_data.h" // ConfigData structure

#ifdef USE_INFLUXDB
#include <InfluxDbClient.h>
#include "influxdb_handler.h"
extern InfluxDBClient influx_client;
#endif

void readDeviceDataRecord(void *message, const char *devicename);
void readCellDataRecord(void *message, const char *devicename);
void readConfigDataRecord(void *message, const char *devicename);
String getLocalTimeString();

extern uint16_t min_pub_time;
extern uint16_t publish_delay;
extern bool debug_flg_full;
extern bool debug_flg;

#endif // PARSER_H