#ifndef GyroSensor_h
#define GyroSensor_h
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#define DEFAULT_ADRESS 0x68

class GyroSensor
{
private:
    /* data */
public:
    GyroSensor();
    void init();
    sensors_event_t* getData();
    ~GyroSensor();
};

#endif