#ifndef UltrasonicSensor_H_
#define UltrasonicSensor_H_
#include <Arduino.h>
class UltrasonicSensor
{
private:
    /* data */
public:
    UltrasonicSensor();
    void initPins(int TrigPin, int EchoPin);
    unsigned long getRange(float maxRange);
    ~UltrasonicSensor();
};
#endif