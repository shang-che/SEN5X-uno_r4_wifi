#include "SEN5xModule.h"

SEN5xModule::SEN5xModule() {}

bool SEN5xModule::begin() {
    sen5x.begin(Wire);
    uint16_t error = sen5x.deviceReset();
    if (error) {
        Serial.println("SEN5x initialization failed!");
        return false;
    }
    Serial.println("SEN5x initialization succeeded.");
    return true;
}

bool SEN5xModule::startMeasurement() {
    uint16_t error = sen5x.startMeasurement();
    if (error) {
        Serial.println("SEN5x start measurement failed!");
        return false;
    }
    Serial.println("SEN5x measurement started.");
    return true;
}

bool SEN5xModule::readValues(float &pm1p0, float &pm2p5, float &pm4p0,
                             float &pm10p0, float &humidity, float &temperature,
                             float &vocIndex, float &noxIndex) {
    uint16_t error = sen5x.readMeasuredValues(
        pm1p0, pm2p5, pm4p0, pm10p0, humidity, temperature, vocIndex, noxIndex);
    if (error) {
        char errorMessage[256];
        errorToString(error, errorMessage, 256);
        Serial.print("SEN5x read values failed! Error: ");
        Serial.println(errorMessage);
        return false;
    }
    return true;
}
