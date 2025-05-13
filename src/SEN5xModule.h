#ifndef SEN5XMODULE_H
#define SEN5XMODULE_H

#include <SensirionI2CSen5x.h>
#include <Wire.h>

class SEN5xModule {
   public:
    SEN5xModule();
    bool begin();
    bool startMeasurement();
    bool readValues(float &pm1p0, float &pm2p5, float &pm4p0, float &pm10p0,
                    float &humidity, float &temperature, float &vocIndex,
                    float &noxIndex);

   private:
    SensirionI2CSen5x sen5x;
};

#endif
