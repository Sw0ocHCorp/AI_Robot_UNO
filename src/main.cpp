#include "MotorControler.h"
#include "UltrasonicSensor.h"
#include "HUSKYLENS.h"
#include "GyroSensor.h"
#include "SoftwareSerial.h"
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <FastLED.h>
#define PIN_TRIG 13
#define PIN_ECHO 12
#define PIN_PB_MODE 2
#define PIN_RGB_MODE 4
//ATTENTION, !! POUR UN NODEMCU !! IL FAUT S'ASSURER QUE LES FONCTION _min() ET _max() SOIENT BIEN PRéSENTE
//DANS LE FICHIER HEADER "HUSKYLENS.h" SINON, REMPLACER min() et max() PAR _min() et _max() --> Ligne 171 & 297

HUSKYLENS huskylens;
MotorControler motorControler;
UltrasonicSensor ultrasonicSensor;
GyroSensor gyroSensor;
CRGB rgbModeLed[PIN_RGB_MODE];
//SoftwareSerial mySerial(13, 12); // RX, TX
//HUSKYLENS green line >> SDA OR RX -> ; blue line >> SCL OR TX -> 
void printResult(HUSKYLENSResult result);
unsigned long prevTime= 0;
int speedLeft;
int speedRight;
float error;
float brake;
bool camIsInclined= true;
bool stateBp= true;
bool prevStateBp= true;

void setup() {
  Serial.begin(9600);
  //mySerial.begin(9600);
  gyroSensor.init();
  FastLED.addLeds<NEOPIXEL, PIN_RGB_MODE>(rgbModeLed, 1);
  pinMode(PIN_PB_MODE, INPUT);
  motorControler= MotorControler();
  ultrasonicSensor.initPins(PIN_TRIG, PIN_ECHO);
  while (!huskylens.begin(Serial))
  {
      Serial.println(F("Begin failed!"));
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
      
  }
}

void loop() {
    /*sensors_event_t *measures;
    measures= gyroSensor.getData();
    Serial.println("ACELLERATION AXE X= " + String(measures[0].acceleration.x) + "m/s^2");
    Serial.println("ACELLERATION AXE Y= " + String(measures[0].acceleration.y) + "m/s^2");
    Serial.println("ACELLERATION AXE Z= " + String(measures[0].acceleration.z) + "m/s^2");
    Serial.println("ROTATION AXE X= " + String(measures[1].gyro.x) + "rad/s");
    Serial.println("ROTATION AXE Y= " + String(measures[1].gyro.y) + "rad/s");
    Serial.println("ROTATION AXE Z= " + String(measures[1].gyro.z) + "rad/s");
    delay(250);*/

    if ((millis() - prevTime) > 1000) {
        stateBp= digitalRead(PIN_PB_MODE);
        if ((prevStateBp == 1) && (stateBp == 0)) {
            camIsInclined= !camIsInclined;
            Serial.println("Mode Camera Inclinée= " + String(camIsInclined));
            if (camIsInclined == true)
            {
                rgbModeLed[0]= CRGB::Purple;
            } else {
                rgbModeLed[0]= CRGB::Blue;
            }
        }
        prevTime= millis();
    }
    if (!huskylens.request()) { 
        Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
        motorControler.manageMotors("STAND_BY", 0, 0);
    } else if(!huskylens.isLearned()) {
        Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
        motorControler.manageMotors("STAND_BY", 0, 0);
    } else if (!huskylens.available()) {
        Serial.println(F("No block or arrow appears on the screen!"));
        motorControler.manageMotors("STAND_BY", 0, 0);
    } else {
        Serial.println(F(" "));
        while (huskylens.available())
        {
            float intensCoeff= 1.0;
            HUSKYLENSResult result = huskylens.read();
            //printResult(result);
            
            
            if ((result.command == COMMAND_RETURN_BLOCK) || (result.command == COMMAND_RETURN_ARROW)) {
                if (result.command == COMMAND_RETURN_BLOCK){
                    //Ordres de Grandeurs | Très Proche Carré -> ~162 px W et H                     | Très Proche Y -> ~ 200 px
                    //Ordres de Grandeurs | Très Loin Carré -> ~41 px W et H                        | Très Loin Y -> ~24 px
                    //Ordres de Grandeurs | Distance Moyenne Carré(Taille de Base) -> 70 px W et H  | Distance Moyenne Y -> ~120 px
                    //Ordres de Grandeurs | Déviation Gauche -> ~32 px  | Milieu -> ~160 px     | Droite -> ~296 px
                    float sizeSquareCoeff= 0.5;
                    float proximCoeff= 0.5;
                    if (camIsInclined == true) {                        
                        error= 120 - result.yCenter;
                        Serial.println("Erreur Position Y" + String(error));
                        proximCoeff= (100 / (1 + exp(-error / 20))) * 0.6;
                        error= 70 - ((result.width + result.height) / 2);
                        Serial.println("Erreur Cadre Objet" + String(error));
                        if (error > 0){
                            sizeSquareCoeff= (100 / (1 + exp(-error / 5))) * 0.4;
                            Serial.println("SizeSquare pour diff > 0 = " + String(sizeSquareCoeff));
                        } else
                        {
                            sizeSquareCoeff= (100 / (1 + exp(-error / 15))) * 0.4;
                            Serial.println("SizeSquare pour diff < 0 = " + String(sizeSquareCoeff));
                        }
                    } else {
                        error= 120 - result.yCenter;
                        proximCoeff= (100 / (1 + exp(-error / 20))) * 0.4;
                        error= 70 - ((result.width + result.height) / 2);
                        if (error > 0){
                            sizeSquareCoeff= (100 / (1 + exp(-error / 5))) * 0.6;
                        } else
                        {
                            sizeSquareCoeff= (100 / (1 + exp(-error / 15))) * 0.6;
                        }
                    }
                    Serial.println("Coeff Position Y= " + String(proximCoeff)+ "%");
                    Serial.println("Coeff Cadre Objet= " + String(sizeSquareCoeff) + "%");
                    intensCoeff= (proximCoeff + sizeSquareCoeff) / 100;
                    error= 160 - result.xCenter;
                    //Serial.println("Position X de L'objet= " + String(result.xCenter));
                    //Serial.println("--> Position Y de L'objet= " + String(result.yCenter));
                    //Serial.println("====> Largeur du Cadre Objet Axe X= " + String(result.width));      
                    //Serial.println("====> Largeur du Cadre Objet Axe Y= " + String(result.height));     
                    //Serial.println("Déviation de la position de l'Objet cible= " + String(error));      
                    /*if ((millis() - prevTime) > 25)
                    {
                        unsigned long cmRange= ultrasonicSensor.getRange(1);                        //Argument en M, Variable de Retour en CM
                        Serial.println("--> L Objet est à " + String(cmRange) + " cm du Capteur");
                        prevTime= millis();
                    }*/
                    


                } else if (result.command == COMMAND_RETURN_ARROW){
                    error= result.xOrigin - result.xTarget; 
                    Serial.println("Différence de position X entre l'Origine et la flèche= "+ String(error));         
                }
                speedLeft= 100 * (1 / (1+exp(error/100))) * intensCoeff;
                speedRight= 100 * (1 / (1+exp(-error/100))) * intensCoeff;
                
                Serial.println("Puissance du Moteur Gauche= " + String(speedLeft));
                Serial.println("Puissance du Moteur Droit= " + String(speedRight));
                Serial.println("Valeur du Coefficient d'intensité des Moteurs= " + String(intensCoeff));
                Serial.println("-------------------------------------------------");
                motorControler.manageMotors("FORWARD", speedLeft, speedRight);
            }
            else {
                motorControler.manageMotors("STAND_BY", 0, 0);
            }

            
        }    
    } 
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}