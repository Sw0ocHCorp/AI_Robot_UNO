#include "UltrasonicSensor.h"
int tPin;
int ePin;
UltrasonicSensor::UltrasonicSensor(){

}

void UltrasonicSensor::initPins(int TrigPin, int EchoPin) {
    pinMode(TrigPin, OUTPUT);
    pinMode(EchoPin, INPUT);
    tPin= TrigPin;
    ePin= EchoPin;
}

unsigned long UltrasonicSensor::getRange(float maxRange) {
    digitalWrite(tPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(tPin, LOW);
    return pulseIn(ePin, HIGH, maxRange * 6000)/58;
}

UltrasonicSensor::~UltrasonicSensor() {
    
}