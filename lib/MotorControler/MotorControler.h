#ifndef MotorControler_H_
#define MotorControler_H_

#include <Arduino.h>

#define PIN_Motor_PWMA 5
#define PIN_Motor_PWMB 6
#define PIN_Motor_BIN_1 8
#define PIN_Motor_AIN_1 7
#define PIN_Motor_STBY 3
#define speed_Max 255

class MotorControler
{
private:
    /* data */
public:
    MotorControler(/* args */);
    void manageMotors(String direction, uint8_t percentSpeedL, uint8_t percentSpeedR);
    ~MotorControler();
};

#endif