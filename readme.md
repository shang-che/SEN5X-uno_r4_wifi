# Info

- this is a project use sen5x sensor to get the data from the air
- the data will be writen to a csv file in the SD card

# Board

- Arduino uno R4 with WiFi
- baud rate: 115200

# components

- Sesirion SEN5x sensor
- Arduino uno R4 with WiFi
- SD card module
- RTC Modules(HW-111)

# Arduino Libraries

- RTClib
- SdFat
- Sesirion Core
- Sesirion I2C SEN5X
- Adafruit BusIO
- ArduinoGraphics
- Blynk
- BlynkNcpDriver
- Adafruit BusIO
- Adafruit ADS1X15

# Blynk Vurtual Pins

- V0: PM1.0
- V1: PM2.5
- V2: PM4.0
- V3: PM10
- V4: PM1.0
- V5: PM2.5
- V6: PM4.0
- V7: PM10
- V8: refresh bottom
- V9: show PM1.0 on the LED matrix
- V10: show PM2.5 on the LED matrix
- V11: show PM4.0 on the LED matrix
- V12: show PM10 on the LED matrix
- V13: show Humidity on the LED matrix
- V14: show Temperature on the LED matrix
- V15: show VOC on the LED matrix
- V16: show NOx on the LED matrix

# Connections

- sen5x sensor
  - VCC -> 5V
  - GND -> GND
  - SDA -> SDA
  - SCL -> SCL
- SD card module
  - VCC -> 5V
  - GND -> GND
  - CS -> 9
  - MOSI -> 11
  - MISO -> 12
  - SCK -> 13
- RTC module
  - VCC -> 5V
  - GND -> GND
  - SDA -> SDA
  - SCL -> SCL
