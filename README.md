# B-L475E-IOT01A Wi-Fi hygrometer

Cube HAL source modules for reading the on-board HTS221 and serving its
temperature and relative-humidity measurements over the ISM43362 Wi-Fi
module.

The module creates a WPA2-protected Wi-Fi access point. Connect a phone or
computer to that network and open the network's gateway/router address to view
the page. The page refreshes its readings from `/api/readings`.

## Peripheral settings

- **HTS221:** I2C; humidity averaging set to 512 samples and temperature averaging set to 256 samples; continuous mode at 12.5 Hz; block-data update enabled.
- **ISM43362-M3G-L44:** use ES-WiFi BSP over the board's SPI connection. The source uses the `wifi.h` API to configure a WPA2 access point.
- **HTTP:** single-client TCP server on port 80. The JSON response includes temperature in Celsius and relative humidity in percent.

## Notes

1. Change `WIFI_AP_PASSWORD` in `WiFi/Inc/ism43362_wifi.h` to a unique WPA2 password (at least 8 characters). The default is a placeholder; do not commit a real password to a shared source repository.
2. In the generated `main.c`, call `HygrometerApp_Init()` after HAL and peripheral initialization, then repeatedly call `HygrometerApp_RunOnce()`. The app prints the AP name with `printf`, which is retargeted to USART1 (ST-Link virtual COM port, 115200 8N1). Find the web server IP in the connected client's Wi-Fi network details (the gateway/router address), then browse to `http://<gateway-ip>/`. The AP password is the value of `WIFI_AP_PASSWORD` in the header and is not printed.


Example integration:

```c
HygrometerApp_Init();
for (;;) {
    HygrometerApp_RunOnce();
}
```

## Peripheral-oriented source layout

```text
application/
  hygrometer_app.c       Init sequence and the per-loop read/poll step
  hygrometer_app.h
  web_server.c           HTTP routing, HTML page, and JSON response
  web_server.h
Sensors/
  hts221/
    hts221.c             HTS221 register configuration and conversion
    hts221.h
WiFi/
  Inc/
    ism43362_wifi.h      AP SSID/password and ISM43362_StartAccessPoint()
    wifi.h               Transport-agnostic Wi-Fi API used by the app
    es_wifi.h            ES-WiFi status codes and ES_WIFI_* interface
    es_wifi_conf.h       ES-WiFi options: SPI transport, buffer, timeouts
    es_wifi_io.h         SPI transport hooks and module control macros
  Src/
    ism43362_wifi.c      Access-point bring-up on top of wifi.h
    wifi.c               Thin wrapper that hides ES-WiFi behind wifi.h
    es_wifi.c            ES-WiFi AT command implementation
    es_wifi_io.c         SPI3 transport, reset/wake-up/DRDY handling
Core/
  Inc/                   CubeMX-generated HAL headers
  Src/                   CubeMX-generated init and retarget sources
                           main.c, gpio.c, i2c.c, usart.c, stm32l4xx_it.c,
                           syscalls.c, sysmem.c, system_stm32l4xx.c
Drivers/                 STM32L4 HAL driver and CMSIS
cmake/
  stm32cubemx/CMakeLists.txt  CubeMX-generated source list and include paths
  gcc-arm-none-eabi.cmake     arm-none-eabi-gcc toolchain file
  starm-clang.cmake           starm-clang toolchain file (picolibc)
```

## References

- [STMicroelectronics STM32CubeL4 Wi-Fi HTTP server example](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/B-L475E-IOT01A/Applications/WiFi/WiFi_HTTP_Server)
- [HTS221 datasheet](https://www.st.com/resource/en/datasheet/hts221.pdf)
- [eS-WiFi Module AT Command Set User Manual](https://www.inventeksys.com/iwin/wp-content/uploads/IWIN_Command_Set_Users_Manual.pdf)
- [ISM43362-M3G-L44 Product Page](https://www.inventeksys.com/ism4336-m3g-l44-e-embedded-serial-to-wifi-module/)
