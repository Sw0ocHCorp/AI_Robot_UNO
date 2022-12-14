#include <GyroSensor.h>
Adafruit_MPU6050 sensor;

GyroSensor::GyroSensor() {

}

void GyroSensor::init() {
    if (!sensor.begin(DEFAULT_ADRESS))
    {
        Serial.println("Le Capteurs n'est pas détecté");
        delay(10);
    }
    sensor.setAccelerometerRange(MPU6050_RANGE_16_G);
    sensor.setGyroRange(MPU6050_RANGE_250_DEG);
    sensor.setFilterBandwidth(MPU6050_BAND_21_HZ);    
    
}

sensors_event_t *GyroSensor::getData() {
    sensors_event_t accel, gyro, temp;
    sensor.getEvent(&accel, &gyro, &temp);
    sensors_event_t measures[3]= {accel, gyro, temp};
    return measures;
}

GyroSensor::~GyroSensor() {}