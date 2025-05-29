#include "BlynkConfig.h"
#include <Adafruit_ADS1X15.h>
#include <Arduino.h>
#include <BlynkSimpleWifi.h>
#include <SPI.h>
#include <SdFat.h>
#include <WiFiS3.h>
#include <Wire.h>

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

#include "RTCModule.h"
#include "SEN5xModule.h"
#include "BlynkHandlers.h"

#define MAXTRIX_SPEED 50
#define SD_CS_PIN 9             // SD card chip select pin
#define WIFI_MODE_SWITCH_PIN 7  // Pin for WiFi mode switch

int wifiMode = 0;  // 0: Off, 1: On

// Global sensor value variables
float g_pm1p0 = 0;
float g_pm2p5 = 0;
float g_pm4p0 = 0;
float g_pm10p0 = 0;
float g_humidity = 0;
float g_temperature = 0;
float g_vocIndex = 0;
float g_noxIndex = 0;
int16_t g_adsChannel0Raw = 0;
int16_t g_adsChannel1Raw = 0;
int16_t g_adsChannel2Raw = 0;
int16_t g_adsChannel3Raw = 0;

RTCModule rtcModule;
SEN5xModule sen5xModule;
SdFat SD;
String filename;
ArduinoLEDMatrix matrix;
Adafruit_ADS1115 ads;

uint8_t wifiBitmap_On[8][12] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0},
    {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}, {0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}};

void setup() {
    Serial.begin(115200);
    matrix.begin();  // Initialize LED matrix
    Wire.begin();    // Initialize I2C communication

    while (!Serial) {
        delay(100);
    }
    pinMode(7, INPUT_PULLUP);  // Set pin 7 as input with pull-up resistor to be
                               // a switch
    wifiMode = digitalRead(7);

    if (wifiMode == LOW) {
        Serial.println("WiFi mode is ON");
        // If pin 7 is connected to GND, WiFi mode is ON
        // Begin Blynk connection process
        Blynk.begin(BLYNK_AUTH_TOKEN, blynk_ssid, blynk_pass);

        // Wait for WiFi connection with timeout (10 seconds)
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED &&
               (millis() - startTime < 10000)) {
            delay(500);
        }
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Error: WiFi did not connect!");
        } else {
            Serial.println("WiFi connected successfully.");
            matrix.renderBitmap(wifiBitmap_On, 8, 12);
        }
    } else {
        Serial.println("WiFi mode is OFF");
    }
    delay(2000);  // Wait for serial monitor to be ready
    Serial.println("Initializing modules...");

    // Initialize RTC
    if (!rtcModule.begin()) {
        while (1);
    }

    // Initialize SEN5x sensor
    if (!sen5xModule.begin() || !sen5xModule.startMeasurement()) {
        while (1);
    }
    // Initialize ADS1115
    if (!ads.begin()) {
        Serial.println("Failed to initialize ADS1115. Check wiring!");
        while (1) {
            delay(100);
        }
    } else {
        Serial.println("ADS1115 initialized successfully.");
    }
    // 設定預設增益: GAIN_TWOTHIRDS 對應 ±6.144V 範圍，
    // 每個 LSB 大約 0.1875 mV (0.0001875 V)
    ads.setGain(GAIN_TWOTHIRDS);
    // Initialize SD card
    if (!SD.begin(SD_CS_PIN, SD_SCK_MHZ(4))) {
        Serial.println("SD card initialization failed!");
        while (1);
    }
    Serial.println("SD card initialized successfully.");

    // Setup filename based on RTC timestamp
    String rawTimestamp = rtcModule.getTimestamp();
    rawTimestamp.replace("/", "-");
    rawTimestamp.replace(" ", "-");
    rawTimestamp.replace(":", "-");
    filename = rawTimestamp + ".csv";

    // Write header to SD file
    String header =
        "Time,PM1.0,PM2.5,PM4.0,PM10.0,Humidity,Temperature,VOC,NOx,CH0 "
        "Raw,CH0 Voltage,CH1 Raw,CH1 Voltage,CH2 Raw,CH2 Voltage,CH3 Raw,CH3 "
        "Voltage\n";
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.print(header);
        dataFile.close();
        Serial.println("File initialization succeeded:" + filename);
    } else {
        Serial.println("Can't write to SD card!");
    }
}

void loop() {
    if (wifiMode == LOW) {
        // If WiFi mode is OFF, do not run Blynk
        Blynk.run();
    }

    // Get current timestamp
    String timestamp = rtcModule.getTimestamp();

    // Read SEN5x values and update global sensor variables
    if (!sen5xModule.readValues(g_pm1p0, g_pm2p5, g_pm4p0, g_pm10p0, g_humidity,
                                g_temperature, g_vocIndex, g_noxIndex)) {
        delay(1000);
        return;
    }
    // read ADS1115 values channel 0~3
    g_adsChannel0Raw = ads.readADC_SingleEnded(0);
    g_adsChannel1Raw = ads.readADC_SingleEnded(1);
    g_adsChannel2Raw = ads.readADC_SingleEnded(2);
    g_adsChannel3Raw = ads.readADC_SingleEnded(3);

    // Convert raw values to voltage (0.1875 mV/LSB)
    float conversionFactor = 0.0001875;
    float volt0 = g_adsChannel0Raw * conversionFactor;
    float volt1 = g_adsChannel1Raw * conversionFactor;
    float volt2 = g_adsChannel2Raw * conversionFactor;
    float volt3 = g_adsChannel3Raw * conversionFactor;

    // Prepare data line string for SD card logging
    String dataLine =
        timestamp + "," + String(g_pm1p0) + "," + String(g_pm2p5) + "," +
        String(g_pm4p0) + "," + String(g_pm10p0) + "," + String(g_humidity) +
        "," + String(g_temperature) + "," + String(g_vocIndex) + "," +
        String(g_noxIndex) + "," + String(g_adsChannel0Raw) + "," +
        String(volt0) + "," + String(g_adsChannel1Raw) + "," + String(volt1) +
        "," + String(g_adsChannel2Raw) + "," + String(volt2) + "," +
        String(g_adsChannel3Raw) + "," + String(volt3) + "\n";

    // Write data line to SD card file
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.print(dataLine);
        dataFile.close();
        Serial.println("Data written to file: " + filename);
        Serial.print(dataLine);
    } else {
        Serial.println("Can't write to SD card!");
    }
    if (wifiMode == LOW) {
        Serial.println("WiFi mode is ON__loop");
    } else {
        Serial.println("WiFi mode is OFF__loop");
    }
    delay(1000);
}