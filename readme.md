# Info

- this is a project use sen5x sensor to get the data from the air
- the data will be writen to a csv file in the SD card

# Board

- Arduino uno R4 with WiFi

# components

- sen5x sensor
- Arduino uno R4 with WiFi
- SD card module
- RTC Modules(HW-111)

# Arduino Libraries

- RTClib
- SdFat
- Sesirion Core
- Sesirion I2C SEN5X
- Adafruit BusIO

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
