#include "hygrometer_app.h"

#include <stdio.h>

#include "main.h"
#include "hts221.h"
#include "ism43362_wifi.h"
#include "web_server.h"

extern I2C_HandleTypeDef HTS221_I2C;

static HTS221_Sample current_sample;

void HygrometerApp_Init(void)
{
    if (HTS221_Init() != HTS221_OK) {
        Error_Handler();
    }
    if (ISM43362_StartAccessPoint() != 0) {
        Error_Handler();
    }

    (void)printf("Join Wi-Fi '%s' and open the network gateway address.\r\n",
                 WIFI_AP_SSID);
    if (WebServer_Start() != 0) {
        Error_Handler();
    }
}

void HygrometerApp_RunOnce(void)
{
    HTS221_Status sensor_status = HTS221_Read(&current_sample);

    if ((sensor_status != HTS221_OK) && (sensor_status != HTS221_NOT_READY)) {
        Error_Handler();
    }
    if (WebServer_Poll(&current_sample) != 0) {
        Error_Handler();
    }
}
