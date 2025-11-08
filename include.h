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
// Macierz sekwencji ruchów: {seq_number, kąt_ręki, delta_hand, kąt_potwora, delta_monster, delay_ms, led_mode}
// seq_number: 1,2,3... = początek sekwencji, 0 = kontynuacja, 99 = koniec sekwencji
// delta_hand: prędkość ruchu ręki (stopni na krok)
// delta_monster: prędkość ruchu potwora (stopni na krok)
// led_mode: 0 = zielona ON/czerwona OFF, 1 = czerwona ON/zielona OFF
// hand 0 120
// monster hide 80 140
#define HAND_HIDE_ANGLE 120
#define HAND_SHOW_ANGLE 0
#define HAND_MIDD_ANGLE 50

#define MONSTER_SHOW_ANGLE 90
#define MONSTER_HIDE_ANGLE 145
#define MONSTER_MIDD_ANGLE 110

#define NO_ACTIVE_SEQUENCE 0

const int moveSequences[][7] PROGMEM = {
  // Sekwencja 1:
  {1, HAND_SHOW_ANGLE,  2,  MONSTER_SHOW_ANGLE,  1, 500, 0},
  {0, HAND_HIDE_ANGLE,  5,  MONSTER_HIDE_ANGLE,  1, 200, 0},  
  {99, -1, -1, -1, -1, -1, 0},
  // Sekwencja 2:
  {2, HAND_MIDD_ANGLE,  2,  MONSTER_SHOW_ANGLE,  1,  500, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  1, 1500, 1},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE, 10, 500, 1},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE, 10, 200, 0},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE, 10, 100, 0},
  {0, HAND_HIDE_ANGLE,  10,  MONSTER_HIDE_ANGLE, 10, 200, 0},
  {99, -1, -1, -1, -1, -1, 0},
  // Sekwencja 3: 
  {3, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE,  1, 600, 1},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE,  1, 200, 0},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE,  5, 500, 0},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE,  4, 500, 1},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE,  2, 800, 1},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE,  2, 200, 0},
  {0, HAND_HIDE_ANGLE,  20,  MONSTER_HIDE_ANGLE,  3, 200, 0},
  {99, -1, -1, -1, -1, -1, 0},
  
  // Sekwencja 4:
  {4, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  10,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  10,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  10,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  10,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  10,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  10,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  10,  100, 1},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE,  5, 1000, 0},
  {0, HAND_HIDE_ANGLE,  10,  MONSTER_HIDE_ANGLE,  5,  200, 0},
   {99, -1, -1, -1, -1, -1, 0},
   // Sekwencja 5:
  {5, HAND_MIDD_ANGLE,  2,  MONSTER_MIDD_ANGLE, 1,  400, 1},
  {0, HAND_SHOW_ANGLE,  5,  MONSTER_SHOW_ANGLE, 2, 1000, 0},
  {0, HAND_HIDE_ANGLE,  8,  MONSTER_HIDE_ANGLE, 2,  200, 0},
  {99, -1, -1, -1, -1, -1, 0},
   // Sekwencja 6:
  {6, HAND_MIDD_ANGLE,  5,  MONSTER_HIDE_ANGLE,  1,  300, 0},
  {0, HAND_MIDD_ANGLE,  5,  MONSTER_SHOW_ANGLE, 10,  600, 0},
  {0, HAND_HIDE_ANGLE,  4,  MONSTER_HIDE_ANGLE, 10,  100, 0},
  {99, -1, -1, -1, -1, -1, 0},
   // Sekwencja 7:
  {7, HAND_HIDE_ANGLE,  5,  MONSTER_HIDE_ANGLE,  1,  2000, 1},
  {0, HAND_HIDE_ANGLE,  5,  MONSTER_HIDE_ANGLE,  1,  1000, 0},
  {99, -1, -1, -1, -1, -1, 0},
   // Sekwencja 8:
  {8, HAND_HIDE_ANGLE,  5,  MONSTER_HIDE_ANGLE,  1,  2000, 1},
  {0, HAND_HIDE_ANGLE,  5,  MONSTER_HIDE_ANGLE,  1,  1000, 0},
  {99, -1, -1, -1, -1, -1, 0},
  // Sekwencja 9:
  {9, HAND_MIDD_ANGLE,      2,  MONSTER_MIDD_ANGLE,  1, 1000, 1},
  {0, HAND_SHOW_ANGLE,      1,  MONSTER_SHOW_ANGLE,  1, 1000, 1},
  {0, HAND_SHOW_ANGLE,      1,  MONSTER_SHOW_ANGLE, 10, 1000, 1},
  {0, HAND_SHOW_ANGLE,      1,  MONSTER_SHOW_ANGLE, 10, 1000, 0},
  {0, HAND_HIDE_ANGLE,      6,  MONSTER_HIDE_ANGLE,  3,  200, 0},
  {99, -1, -1, -1, -1, -1, 0},
};

// Funkcje deklaracje
void setupTimer2();
int measureDistance();
void Config_VL53L0X();
void LedNoDetection();
void LedDetection();
void UpdateServosPosition();
void callResponseNo(int responceNo);
#endif