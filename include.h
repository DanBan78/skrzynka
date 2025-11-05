#ifndef INCLUDE_H
#define INCLUDE_H

#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include "Adafruit_VL53L0X.h"

typedef struct servoConfig {
  int   servoPin;
  int   initPos;
  int   currentPos;
  int   reqPos;
  int   deltaAngle;
  Servo servo;
};

void setupTimer2();
int measureDistance();
void Config_VL53L0X();
void LedNoDetection();
void LedDetection();
void UpdateServosPosition();
void callResponseNo(int responceNo);
#endif