# B-L475E-IOT01A Wi-Fi hygrometer

Cube HAL source modules for reading the on-board HTS221 and serving its
temperature and relative-humidity measurements over the ISM43362 Wi-Fi
module.

The module creates a WPA2-protected Wi-Fi access point. Connect a phone or
computer to that network and open the network's gateway/router address to view
the page. The page refreshes its readings from `/api/readings`.

## Peripheral settings

- **HTS221:** I2C; humidity averaging set to 512 samples and temperature
  averaging set to 256 samples (maximum available averaging for lowest RMS
  noise); continuous mode at 12.5 Hz (maximum ODR); block-data update enabled.
- **ISM43362-M3G-L44:** use ST's B-L475E-IOT01A ES-WiFi BSP over the board's
  SPI connection. The source uses the BSP's `wifi.h` API to configure a WPA2
  access point on channel 6; it does not bit-bang SPI or configure the
  module's board pins itself.
- **HTTP:** single-client TCP server on port 80. The JSON response includes
  temperature in Celsius and relative humidity in percent.

## Add to an STM32CubeIDE project

1. Create/open a CubeIDE project for the **B-L475E-IOT01A / STM32L475VGTx**.
2. Configure the board's HTS221 I2C bus using the board schematic/BSP as the
   reference. Set the project's `HTS221_I2C` macro to the generated I2C handle
   (default in `peripherals/hts221/hts221.h` is `hi2c2`).
3. Enable the board's ES-WiFi BSP with its **SPI** transport. Add the official
   B-L475E-IOT01A Wi-Fi common sources (`wifi.c`, `es_wifi.c`, and
   `es_wifi_io.c`) and their headers from the STM32CubeL4 package. Ensure
   `es_wifi_io` is configured for the board's SPI transport and matching HAL
   peripheral setup. The on-board radio is not a UART Wi-Fi device.
4. Add the files under `peripherals/` and `application/` to the project and
   add their directories to the compiler include paths.
5. Change `WIFI_AP_PASSWORD` in `peripherals/ism43362/ism43362_wifi.h` to a
   unique WPA2 password (at least 8 characters). The default is a placeholder;
   do not commit a real password to a shared source repository.
6. In the generated `main.c`, call `HygrometerApp_Init()` after HAL and
   peripheral initialization, then repeatedly call `HygrometerApp_RunOnce()`.
   The app prints the AP name with `printf`; configure a UART retarget if you
   want to see it on the ST-LINK virtual COM port. Find the web server IP in
   the connected client's Wi-Fi network details (the gateway/router address),
   then browse to `http://<gateway-ip>/`. The AP password is the value of
   `WIFI_AP_PASSWORD` in the header and is not printed.
   A `printf` retarget is not needed for the web server itself. Because the
   JSON endpoint formats floating-point values, enable the CubeIDE/newlib-nano
   linker option `-u _printf_float` when building with newlib-nano.

Example integration:

```c
HygrometerApp_Init();
for (;;) {
    HygrometerApp_RunOnce();
}
```

Initialization failures are passed to the project's `Error_Handler()`. A
temporary "sensor data not ready" response from the HTS221 is not treated as
an I2C failure.

## Peripheral-oriented source layout

```text
application/
  hygrometer_app.c       Startup, sample scheduling, and IP reporting
  hygrometer_app.h
  web_server.c           HTTP routing, page, and JSON response
  web_server.h
peripherals/
  hts221/
    hts221.c             HTS221 register configuration and conversion
    hts221.h
  ism43362/
    ism43362_wifi.c      Access-point setup using the ES-WiFi API
    ism43362_wifi.h
```

## References

- [STMicroelectronics STM32CubeL4 Wi-Fi HTTP server example](https://github.com/STMicroelectronics/STM32CubeL4/tree/master/Projects/B-L475E-IOT01A/Applications/WiFi/WiFi_HTTP_Server)
- [HTS221 datasheet](https://www.st.com/resource/en/datasheet/hts221.pdf)
- [eS-WiFi Module AT Command Set User Manual](https://www.inventeksys.com/iwin/wp-content/uploads/IWIN_Command_Set_Users_Manual.pdf)
