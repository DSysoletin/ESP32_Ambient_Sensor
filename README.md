# ESP32_Ambient_Sensor
ESP32-based MQTT sensor capable reporting CO2 level, temperature, humidity, pressure and light level.
Also have support for TFT touchscreen LCD.

Please use User_Setup.h to setup your Little VGL library. Pinout for connecting LCD and touchscreen also can be found in User_Setup.h

Please chage settings in ESP32_HomeController.ino according to your WiFi and MQTT settings.


Sensor connecting pinout:

Pinout:
For SHT1x temp/humidity sensor
SHT1x SDA (data)  18
SHT1x SCL (clock) 5

For CJMCU8128 temp-humidity-pressure-CO2 sensor board:
SCL D22
SDA D21

Please note: In CJMCU8128 board we have temp-humidity sensor, but it isn't working for me. I'am not sure if it is hardware or software issue, but it looks like problem in sensor itself, so I've added SHT10 sensor.

Solar cell (for measuring light level) is connected between GND and D35.
