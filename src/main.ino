#define BLYNK_TEMPLATE_ID "TMPL4VE-IYzNe"
#define BLYNK_TEMPLATE_NAME "air sensor"
#define BLYNK_AUTH_TOKEN "JvCsXuEoVqWsifvcW5izns7BB0H7-OuH"

#include <Arduino.h>
#include <BlynkSimpleWifi.h>
#include <SPI.h>
#include <SdFat.h>
#include <WiFiS3.h>
#include <Wire.h>

#include "RTCModule.h"
#include "SEN5xModule.h"

#define SD_CS_PIN 9  // SD card chip select pin

char ssid[] = "IIA";
char pass[] = "Iia-2020!-863";

// Global sensor value variables
float g_pm1p0 = 0;
float g_pm2p5 = 0;
float g_pm4p0 = 0;
float g_pm10p0 = 0;
float g_humidity = 0;
float g_temperature = 0;
float g_vocIndex = 0;
float g_noxIndex = 0;

RTCModule rtcModule;
SEN5xModule sen5xModule;
SdFat SD;
String filename;
int refresh = 0;

// Blynk callback for virtual pin V8 (refresh control)
BLYNK_WRITE(V8) {
    // Get value from V8 as an integer
    refresh = param.asInt();
    Serial.print("Received from V8: ");
    Serial.println(refresh);
    // When refresh is 1, send the latest sensor values to Blynk virtual pins
    // V0~V7.
    if (refresh == 1) {
        Blynk.virtualWrite(V0, g_pm1p0);          // PM1.0
        Blynk.virtualWrite(V1, g_pm2p5);          // PM2.5
        Blynk.virtualWrite(V2, g_pm4p0);          // PM4.0
        Blynk.virtualWrite(V3, g_pm10p0);         // PM10.0
        Blynk.virtualWrite(V4, g_humidity);       // Humidity
        Blynk.virtualWrite(V5, g_temperature);    // Temperature
        Blynk.virtualWrite(V6, int(g_vocIndex));  // VOC
        Blynk.virtualWrite(V7, int(g_noxIndex));  // NOx
        Serial.println("Refresh complete: Sensor data updated to web.");
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // Begin Blynk connection process
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Wait for WiFi connection with timeout (10 seconds)
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 10000)) {
        delay(500);
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Error: WiFi did not connect!");
    } else {
        Serial.println("WiFi connected successfully.");
    }

    // Initialize RTC
    if (!rtcModule.begin()) {
        while (1);
    }

    // Initialize SEN5x sensor
    if (!sen5xModule.begin() || !sen5xModule.startMeasurement()) {
        while (1);
    }

    // Initialize SD card
    if (!SD.begin(SD_CS_PIN, SD_SCK_MHZ(4))) {
        Serial.println("SD card initialization failed!");
        while (1);
    }
    Serial.println("SD card initialization succeeded.");

    // Setup filename based on RTC timestamp
    String rawTimestamp = rtcModule.getTimestamp();
    rawTimestamp.replace("/", "-");
    rawTimestamp.replace(" ", "-");
    rawTimestamp.replace(":", "-");
    filename = rawTimestamp + ".csv";

    // Write header to SD file
    String header =
        "Time,PM1.0,PM2.5,PM4.0,PM10.0,Humidity,Temperature,VOC,NOx\n";
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
    Blynk.run();

    // Get current timestamp
    String timestamp = rtcModule.getTimestamp();

    // Read SEN5x values and update global sensor variables
    if (!sen5xModule.readValues(g_pm1p0, g_pm2p5, g_pm4p0, g_pm10p0, g_humidity,
                                g_temperature, g_vocIndex, g_noxIndex)) {
        delay(1000);
        return;
    }

    // Prepare data line string for SD card logging
    String dataLine = timestamp + "," + String(g_pm1p0) + "," +
                      String(g_pm2p5) + "," + String(g_pm4p0) + "," +
                      String(g_pm10p0) + "," + String(g_humidity) + "," +
                      String(g_temperature) + "," + String(g_vocIndex) + "," +
                      String(g_noxIndex) + "\n";

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
    delay(1000);
}