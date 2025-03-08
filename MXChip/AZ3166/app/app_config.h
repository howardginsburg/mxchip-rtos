/* 
 * Copyright (c) Microsoft
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     Microsoft         - Initial version
 *     Frédéric Desbiens - 2024 version.
 */

#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection config
// ----------------------------------------------------------------------------
#define HOSTNAME                    "eclipse-threadx"  //Change to unique hostname.
#define WIFI_SSID                   ""
#define WIFI_PASSWORD               "" 
#define WIFI_MODE                   WPA2_PSK_AES

// ----------------------------------------------------------------------------
// MQTT Config
// ----------------------------------------------------------------------------
#define CLIENT_ID_STRING            "mytestclient"
#define TOPIC_NAME                  "topic"
#define MQTT_SERVER_ADDRESS         IP_ADDRESS(192, 168, 175, 223)
#define MQTT_SERVER_PORT            1883
//#define MQTT_USERNAME             ""
//#define MQTT_PASSWORD             ""

#define SEND_INTERVAL               5 // seconds

#endif
