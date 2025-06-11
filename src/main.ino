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
#define MAXTRIX_SPEED_TIME_OUTPUT 75
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
int16_t g_animationCount = 0;
int8_t g_timeminute = 0;
const uint32_t wifiOnAnimation[][4] = {
    {0x90000, 0x6000000, 0xc00c00, 66},
    {0x88020, 0x5002401, 0x40d40d40, 66},
    {0x48000048, 0x3100501, 0x50d50d50, 66},
    {0x44028144, 0x53950551, 0x55d55d55, 66}};
const uint32_t wifiOffAnimation[][4] = {
    {0x210214a, 0x10210217, 0xa102103f, 66},
    {0x210214a, 0x10213214, 0xad32103f, 66},
    {0x210214a, 0x102d4a13, 0x2d02103f, 66},
    {0x2102d4a, 0x102d4a17, 0xad32103f, 66}};

RTCModule rtcModule;
SEN5xModule sen5xModule;
SdFat SD;
String filename;
ArduinoLEDMatrix matrix;
Adafruit_ADS1115 ads;
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

    if (wifiMode != LOW) {
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
            matrix.loadFrame(wifiOnAnimation[0]);
        }
    } else {
        Serial.println("WiFi mode is OFF");
        matrix.loadFrame(wifiOffAnimation[0]);
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
    // set default gain to GAIN_TWOTHIRDS, which corresponds to a range of
    // ±6.144V
    //  and each LSB is approximately 0.1875 mV (0.0001875 V)
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
    if (wifiMode != LOW) {
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
    if (wifiMode != LOW) {
        Serial.println("WiFi mode is ON__loop");
        matrix.loadFrame(wifiOnAnimation[(g_animationCount++) % 4]);
    } else {
        Serial.println("WiFi mode is OFF__loop");
        matrix.loadFrame(wifiOffAnimation[(g_animationCount++) % 4]);
    }
    // show the time on the matrix
    if (g_timeminute != rtcModule.getTimeminute()) {
        g_timeminute = rtcModule.getTimeminute();
        matrix.beginDraw();
        matrix.stroke(0xFFFFFFFF);
        matrix.textScrollSpeed(MAXTRIX_SPEED_TIME_OUTPUT);
        matrix.textFont(Font_5x7);
        String text = rtcModule.getTimestamphhmm();
        matrix.beginText(0, 1, 0xFFFFFF);
        matrix.println(text);
        matrix.endText(SCROLL_LEFT);
        matrix.endDraw();
    }
    delay(1000);
}