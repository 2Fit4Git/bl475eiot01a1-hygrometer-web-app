#include "ism43362_wifi.h"

#include "wifi.h"

int ISM43362_StartAccessPoint(void)
{
    if (WIFI_Init() != WIFI_STATUS_OK) {
        return -1;
    }
    if (WIFI_ConfigureAP((const uint8_t *)WIFI_AP_SSID,
                         (const uint8_t *)WIFI_AP_PASSWORD,
                         WIFI_ECN_WPA2_PSK, 6U, 1U) != WIFI_STATUS_OK) {
        return -1;
    }
    return 0;
}
