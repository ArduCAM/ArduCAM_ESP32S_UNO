/*
 ESP8266WiFiSTA.cpp - WiFi library for esp8266

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
#include "WiFiAP.h"

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
#include <lwip/ip_addr.h>
}



// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------- Private functions ------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

static bool softap_config_equal(const wifi_config_t& lhs, const wifi_config_t& rhs);



/**
 * compare two AP configurations
 * @param lhs softap_config
 * @param rhs softap_config
 * @return equal
 */
static bool softap_config_equal(const wifi_config_t& lhs, const wifi_config_t& rhs)
{
    if(strcmp(reinterpret_cast<const char*>(lhs.ap.ssid), reinterpret_cast<const char*>(rhs.ap.ssid)) != 0) {
        return false;
    }
    if(strcmp(reinterpret_cast<const char*>(lhs.ap.password), reinterpret_cast<const char*>(rhs.ap.password)) != 0) {
        return false;
    }
    if(lhs.ap.channel != rhs.ap.channel) {
        return false;
    }
    if(lhs.ap.ssid_hidden != rhs.ap.ssid_hidden) {
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- AP function -----------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------


/**
 * Set up an access point
 * @param ssid          Pointer to the SSID (max 63 char).
 * @param passphrase    (for WPA2 min 8 char, for open use NULL)
 * @param channel       WiFi channel number, 1 - 13.
 * @param ssid_hidden   Network cloaking (0 = broadcast SSID, 1 = hide SSID)
 */
bool WiFiAPClass::softAP(const char* ssid, const char* passphrase, int channel, int ssid_hidden)
{

    if(!WiFi.enableAP(true)) {
        // enable AP failed
        return false;
    }

    if(!ssid || *ssid == 0 || strlen(ssid) > 31) {
        // fail SSID too long or missing!
        return false;
    }

    if(passphrase && (strlen(passphrase) > 63 || strlen(passphrase) < 8)) {
        // fail passphrase to long or short!
        return false;
    }

    esp_wifi_start();

    wifi_config_t conf;
    strcpy(reinterpret_cast<char*>(conf.ap.ssid), ssid);
    conf.ap.channel = channel;
    conf.ap.ssid_len = strlen(ssid);
    conf.ap.ssid_hidden = ssid_hidden;
    conf.ap.max_connection = 4;
    conf.ap.beacon_interval = 100;

    if(!passphrase || strlen(passphrase) == 0) {
        conf.ap.authmode = WIFI_AUTH_OPEN;
        *conf.ap.password = 0;
    } else {
        conf.ap.authmode = WIFI_AUTH_WPA2_PSK;
        strcpy(reinterpret_cast<char*>(conf.ap.password), passphrase);
    }

    wifi_config_t conf_current;
    esp_wifi_get_config(WIFI_IF_AP, &conf_current);
    if(softap_config_equal(conf, conf_current)) {
        //DEBUGV("softap config unchanged");
        return true;
    }

    bool ret;

    ret = esp_wifi_set_config(WIFI_IF_AP, &conf) == ESP_OK;

    return ret;
}


/**
 * Configure access point
 * @param local_ip      access point IP
 * @param gateway       gateway IP
 * @param subnet        subnet mask
 */
bool WiFiAPClass::softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet)
{

    if(!WiFi.enableAP(true)) {
        // enable AP failed
        return false;
    }

    tcpip_adapter_ip_info_t info;
    info.ip.addr = static_cast<uint32_t>(local_ip);
    info.gw.addr = static_cast<uint32_t>(gateway);
    info.netmask.addr = static_cast<uint32_t>(subnet);
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    if(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info) == ESP_OK) {
        return tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP) == ESP_OK;
    }
    return false;
}



/**
 * Disconnect from the network (close AP)
 * @param wifioff disable mode?
 * @return one value of wl_status_t enum
 */
bool WiFiAPClass::softAPdisconnect(bool wifioff)
{
    bool ret;
    wifi_config_t conf;
    *conf.ap.ssid = 0;
    *conf.ap.password = 0;
    ret = esp_wifi_set_config(WIFI_IF_AP, &conf) == ESP_OK;

    if(wifioff) {
        ret = WiFi.enableAP(false) == ESP_OK;
    }

    return ret;
}


/**
 * Get the count of the Station / client that are connected to the softAP interface
 * @return Stations count
 */
uint8_t WiFiAPClass::softAPgetStationNum()
{
    wifi_sta_list_t clients;
    if(esp_wifi_ap_get_sta_list(&clients) == ESP_OK) {
        return clients.num;
    }
    return 0;
}

/**
 * Get the softAP interface IP address.
 * @return IPAddress softAP IP
 */
IPAddress WiFiAPClass::softAPIP()
{
    tcpip_adapter_ip_info_t ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
    return IPAddress(ip.ip.addr);
}


/**
 * Get the softAP interface MAC address.
 * @param mac   pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
 * @return      pointer to uint8_t*
 */
uint8_t* WiFiAPClass::softAPmacAddress(uint8_t* mac)
{
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    return mac;
}

/**
 * Get the softAP interface MAC address.
 * @return String mac
 */
String WiFiAPClass::softAPmacAddress(void)
{
    uint8_t mac[6];
    char macStr[18] = { 0 };
    esp_wifi_get_mac(WIFI_IF_AP, mac);

    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

/**
 * Get the softAP interface Host name.
 * @return char array hostname
 */
const char * WiFiAPClass::softAPgetHostname()
{
    const char * hostname;
    if(tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_AP, &hostname)) {
        return NULL;
    }
    return hostname;
}

/**
 * Set the softAP    interface Host name.
 * @param  hostname  pointer to const string
 * @return true on   success
 */
bool WiFiAPClass::softAPsetHostname(const char * hostname)
{
    return tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, hostname) == ESP_OK;
}

/**
 * Enable IPv6 on the softAP interface.
 * @return true on success
 */
bool WiFiAPClass::softAPenableIpV6()
{
    return tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_AP) == ESP_OK;
}

/**
 * Get the softAP interface IPv6 address.
 * @return IPv6Address softAP IPv6
 */
IPv6Address WiFiAPClass::softAPIPv6()
{
    static ip6_addr_t addr;
    if(tcpip_adapter_get_ip6_linklocal(TCPIP_ADAPTER_IF_AP, &addr)) {
        return IPv6Address();
    }
    return IPv6Address(addr.addr);
}
