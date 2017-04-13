/*
 ESP8266WiFiScan.cpp - WiFi library for esp8266

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 Reworked on 28 Dec 2015 by Markus Sattler

 */

#include "WiFi.h"
#include "WiFiGeneric.h"
#include "WiFiScan.h"

extern "C" {
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp32-hal.h>
#include <lwip/ip_addr.h>
#include "lwip/err.h"
}

bool WiFiScanClass::_scanAsync = false;
bool WiFiScanClass::_scanStarted = false;
bool WiFiScanClass::_scanComplete = false;

uint16_t WiFiScanClass::_scanCount = 0;
void* WiFiScanClass::_scanResult = 0;

/**
 * Start scan WiFi networks available
 * @param async         run in async mode
 * @param show_hidden   show hidden networks
 * @return Number of discovered networks
 */
int8_t WiFiScanClass::scanNetworks(bool async, bool show_hidden)
{
    if(WiFiScanClass::_scanStarted) {
        return WIFI_SCAN_RUNNING;
    }

    WiFiScanClass::_scanAsync = async;

    WiFi.enableSTA(true);

    scanDelete();

    wifi_scan_config_t config;
    config.ssid = 0;
    config.bssid = 0;
    config.channel = 0;
    config.show_hidden = show_hidden;
    if(esp_wifi_scan_start(&config, WiFiScanClass::_scanAsync) == ESP_OK) {
        WiFiScanClass::_scanComplete = false;
        WiFiScanClass::_scanStarted = true;

        if(WiFiScanClass::_scanAsync) {
            return WIFI_SCAN_RUNNING;
        }
        while(!(WiFiScanClass::_scanComplete)) {
            delay(10);
        }
        return WiFiScanClass::_scanCount;
    } else {
        return WIFI_SCAN_FAILED;
    }

}


/**
 * private
 * scan callback
 * @param result  void *arg
 * @param status STATUS
 */
void WiFiScanClass::_scanDone()
{
    WiFiScanClass::_scanComplete = true;
    WiFiScanClass::_scanStarted = false;
    esp_wifi_scan_get_ap_num(&(WiFiScanClass::_scanCount));
    if(WiFiScanClass::_scanCount) {
        WiFiScanClass::_scanResult = new wifi_ap_record_t[WiFiScanClass::_scanCount];
        if(WiFiScanClass::_scanResult) {
            esp_wifi_scan_get_ap_records(&(WiFiScanClass::_scanCount), (wifi_ap_record_t*)_scanResult);
        } else {
            //no memory
            WiFiScanClass::_scanCount = 0;
        }
    }
}

/**
 *
 * @param i specify from which network item want to get the information
 * @return bss_info *
 */
void * WiFiScanClass::_getScanInfoByIndex(int i)
{
    if(!WiFiScanClass::_scanResult || (size_t) i > WiFiScanClass::_scanCount) {
        return 0;
    }
    return reinterpret_cast<wifi_ap_record_t*>(WiFiScanClass::_scanResult) + i;
}

/**
 * called to get the scan state in Async mode
 * @return scan result or status
 *          -1 if scan not fin
 *          -2 if scan not triggered
 */
int8_t WiFiScanClass::scanComplete()
{

    if(_scanStarted) {
        return WIFI_SCAN_RUNNING;
    }

    if(_scanComplete) {
        return WiFiScanClass::_scanCount;
    }

    return WIFI_SCAN_FAILED;
}

/**
 * delete last scan result from RAM
 */
void WiFiScanClass::scanDelete()
{
    if(WiFiScanClass::_scanResult) {
        delete[] reinterpret_cast<wifi_ap_record_t*>(WiFiScanClass::_scanResult);
        WiFiScanClass::_scanResult = 0;
        WiFiScanClass::_scanCount = 0;
    }
    _scanComplete = false;
}


/**
 * loads all infos from a scanned wifi in to the ptr parameters
 * @param networkItem uint8_t
 * @param ssid  const char**
 * @param encryptionType uint8_t *
 * @param RSSI int32_t *
 * @param BSSID uint8_t **
 * @param channel int32_t *
 * @return (true if ok)
 */
bool WiFiScanClass::getNetworkInfo(uint8_t i, String &ssid, uint8_t &encType, int32_t &rssi, uint8_t* &bssid, int32_t &channel)
{
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return false;
    }
    ssid = (const char*) it->ssid;
    encType = it->authmode;
    rssi = it->rssi;
    bssid = it->bssid;
    channel = it->primary;
    return true;
}


/**
 * Return the SSID discovered during the network scan.
 * @param i     specify from which network item want to get the information
 * @return       ssid string of the specified item on the networks scanned list
 */
String WiFiScanClass::SSID(uint8_t i)
{
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return String();
    }
    return String(reinterpret_cast<const char*>(it->ssid));
}


/**
 * Return the encryption type of the networks discovered during the scanNetworks
 * @param i specify from which network item want to get the information
 * @return  encryption type (enum wl_enc_type) of the specified item on the networks scanned list
 */
wifi_auth_mode_t WiFiScanClass::encryptionType(uint8_t i)
{
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return WIFI_AUTH_OPEN;
    }
    return it->authmode;
}

/**
 * Return the RSSI of the networks discovered during the scanNetworks
 * @param i specify from which network item want to get the information
 * @return  signed value of RSSI of the specified item on the networks scanned list
 */
int32_t WiFiScanClass::RSSI(uint8_t i)
{
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return 0;
    }
    return it->rssi;
}


/**
 * return MAC / BSSID of scanned wifi
 * @param i specify from which network item want to get the information
 * @return uint8_t * MAC / BSSID of scanned wifi
 */
uint8_t * WiFiScanClass::BSSID(uint8_t i)
{
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return 0;
    }
    return it->bssid;
}

/**
 * return MAC / BSSID of scanned wifi
 * @param i specify from which network item want to get the information
 * @return String MAC / BSSID of scanned wifi
 */
String WiFiScanClass::BSSIDstr(uint8_t i)
{
    char mac[18] = { 0 };
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return String();
    }
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", it->bssid[0], it->bssid[1], it->bssid[2], it->bssid[3], it->bssid[4], it->bssid[5]);
    return String(mac);
}

int32_t WiFiScanClass::channel(uint8_t i)
{
    wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(_getScanInfoByIndex(i));
    if(!it) {
        return 0;
    }
    return it->primary;
}

