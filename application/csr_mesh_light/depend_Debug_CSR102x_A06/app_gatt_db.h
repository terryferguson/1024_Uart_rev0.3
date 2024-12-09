/*
 * THIS FILE IS AUTOGENERATED, DO NOT EDIT!
 *
 * generated by gattdbgen from depend_Debug_CSR102x_A06/app_gatt_db.db_
 */
#ifndef __APP_GATT_DB_H
#define __APP_GATT_DB_H

#include <main.h>

#define HANDLE_GAIA_SERVICE             (0x0001)
#define HANDLE_GAIA_SERVICE_END         (0x0008)
#define ATTR_LEN_GAIA_SERVICE           (16)
#define HANDLE_GAIA_COMMAND_ENDPOINT    (0x0003)
#define ATTR_LEN_GAIA_COMMAND_ENDPOINT  (1)
#define HANDLE_GAIA_RESPONSE_ENDPOINT   (0x0005)
#define ATTR_LEN_GAIA_RESPONSE_ENDPOINT (1)
#define HANDLE_GAIA_RESPONSE_CLIENT_CONFIG (0x0006)
#define ATTR_LEN_GAIA_RESPONSE_CLIENT_CONFIG (0)
#define HANDLE_GAIA_DATA_ENDPOINT       (0x0008)
#define ATTR_LEN_GAIA_DATA_ENDPOINT     (1)
#define HANDLE_GATT_SERVICE             (0x0009)
#define HANDLE_GATT_SERVICE_END         (0x000c)
#define ATTR_LEN_GATT_SERVICE           (2)
#define HANDLE_SERVICE_CHANGED          (0x000b)
#define ATTR_LEN_SERVICE_CHANGED        (0)
#define HANDLE_SERVICE_CHANGED_CLIENT_CONFIG (0x000c)
#define ATTR_LEN_SERVICE_CHANGED_CLIENT_CONFIG (2)
#define HANDLE_GAP_SERVICE              (0x000d)
#define HANDLE_GAP_SERVICE_END          (0x0013)
#define ATTR_LEN_GAP_SERVICE            (2)
#define HANDLE_DEVICE_NAME              (0x000f)
#define ATTR_LEN_DEVICE_NAME            (1)
#define HANDLE_DEVICE_APPEARANCE        (0x0011)
#define ATTR_LEN_DEVICE_APPEARANCE      (1)
#define HANDLE_DEVICE_PREF_CONN_PARAMS  (0x0013)
#define ATTR_LEN_DEVICE_PREF_CONN_PARAMS (6)
#define HANDLE_MESH_CONTROL_SERVICE     (0x0014)
#define HANDLE_MESH_CONTROL_SERVICE_END (0xffff)
#define ATTR_LEN_MESH_CONTROL_SERVICE   (2)
#define HANDLE_NETWORK_KEY              (0x0016)
#define ATTR_LEN_NETWORK_KEY            (1)
#define HANDLE_DEVICE_UUID              (0x0018)
#define ATTR_LEN_DEVICE_UUID            (16)
#define HANDLE_DEVICE_ID                (0x001a)
#define ATTR_LEN_DEVICE_ID              (1)
#define HANDLE_MTL_CONTINUATION_CP      (0x001c)
#define ATTR_LEN_MTL_CONTINUATION_CP    (1)
#define HANDLE_MTL_CP_CLIENT_CONFIG     (0x001d)
#define ATTR_LEN_MTL_CP_CLIENT_CONFIG   (2)
#define HANDLE_MTL_COMPLETE_CP          (0x001f)
#define ATTR_LEN_MTL_COMPLETE_CP        (1)
#define HANDLE_MTL_CP2_CLIENT_CONFIG    (0x0020)
#define ATTR_LEN_MTL_CP2_CLIENT_CONFIG  (2)
#define HANDLE_MTL_TTL                  (0x0022)
#define ATTR_LEN_MTL_TTL                (1)
#define HANDLE_MESH_APPEARANCE          (0x0024)
#define ATTR_LEN_MESH_APPEARANCE        (1)

extern uint16 *GattGetDatabase(uint16 *len);

#endif

/* End-of-File */
