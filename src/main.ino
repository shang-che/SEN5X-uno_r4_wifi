#include <Arduino.h>
#include <SdFat.h>
#include <Wire.h>

#include "RTCModule.h"
#include "SEN5xModule.h"

#define SD_CS_PIN 9  // SD card chip select pin

RTCModule rtcModule;
SEN5xModule sen5xModule;
SdFat SD;
String filename;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // initialize RTC
    if (!rtcModule.begin()) {
        while (1);
    }

    // initialize SEN5x
    if (!sen5xModule.begin() || !sen5xModule.startMeasurement()) {
        while (1);
    }

    // initialize SD card
    if (!SD.begin(SD_CS_PIN, SD_SCK_MHZ(4))) {
        Serial.println("SD card initialization failed!");
        while (1);
    }
    Serial.println("SD card initialization succeeded.");

    // setup filename
    String rawTimestamp = rtcModule.getTimestamp();
    rawTimestamp.replace("/", "-");
    rawTimestamp.replace(" ", "-");
    rawTimestamp.replace(":", "-");
    filename = rawTimestamp + ".csv";

    // write header to file
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
    // get current timestamp
    String timestamp = rtcModule.getTimestamp();

    // read SEN5x values
    float pm1p0, pm2p5, pm4p0, pm10p0, humidity, temperature, vocIndex,
        noxIndex;
    if (!sen5xModule.readValues(pm1p0, pm2p5, pm4p0, pm10p0, humidity,
                                temperature, vocIndex, noxIndex)) {
        delay(1000);
        return;
    }

    // prepare writing data line
    String dataLine = timestamp + "," + String(pm1p0) + "," + String(pm2p5) +
                      "," + String(pm4p0) + "," + String(pm10p0) + "," +
                      String(humidity) + "," + String(temperature) + "," +
                      String(vocIndex) + "," + String(noxIndex) + "\n";

    // write dataline to file(SD card)
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
