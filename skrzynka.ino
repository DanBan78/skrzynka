#include "include.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Definicje pinów
#define LED_GREEN_PIN 7    // Dioda LED 1 Hz
#define LED_RED_PIN 8    // Dioda LED 2 Hz
#define SERVO_HAND_PIN 3     // Pierwszy serwo
#define SERVO_MONSTER_HIDE_PIN 5     // Drugi serwo

servoConfig TeddyHide = {
  SERVO_MONSTER_HIDE_PIN,
  140,
  140,
  140,
  5,
  Servo()
};

servoConfig TeddyHand = {
  SERVO_HAND_PIN,
  120,
  120,
  120,
  10,
  Servo()
};

// Zmienne stanu (zamiast struktury toy_status)
byte contactDetected = 0;
int currentDistance = 0;
byte responseNo = 0;

// Zmienne do debouncing pomiarów
int consecutiveDetections = 0;
const int DETECTION_THRESHOLD = 8;  // Wymaga 8 pomiarów z rzędu (400ms)

// Prosty filtr (zamiast średniej ruchomej)
int lastValidDistance = 0;

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

#define MONSTER_SHOW_ANGLE 80
#define MONSTER_HIDE_ANGLE 140
#define MONSTER_MIDD_ANGLE 120

const int moveSequences[][7] PROGMEM = {
  // Sekwencja 1:
  {1, HAND_SHOW_ANGLE,  2,  MONSTER_SHOW_ANGLE,  1, 500, 0},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  2, 300, 0},  
  {99, -1, -1, -1, -1, -1, 0},
  // Sekwencja 2:
  {2, HAND_MIDD_ANGLE,  1,  MONSTER_SHOW_ANGLE,  1,  500, 1},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1, 1500, 1},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE, 20, 500, 1},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE, 20,  200, 0},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE,  20,  100, 0},
  {0, HAND_HIDE_ANGLE,  10,  MONSTER_HIDE_ANGLE,  10,  500, 0},
  {99, -1, -1, -1, -1, -1, 0},
  // Sekwencja 3: 
  {3, HAND_MIDD_ANGLE,  2,  MONSTER_SHOW_ANGLE,  1,  500, 1},
  {0, HAND_SHOW_ANGLE,  2,  MONSTER_SHOW_ANGLE,  1, 500, 0},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE, 20, 500, 0},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE,  5,  200, 1},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE,  2,  200, 1},
  {0, HAND_SHOW_ANGLE,  2,  MONSTER_SHOW_ANGLE,  1, 500, 0},
  {0, HAND_HIDE_ANGLE,  10,  MONSTER_HIDE_ANGLE,  10,  500, 0},
  {99, -1, -1, -1, -1, -1, 0},
  
  // Sekwencja 4:
  {4, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  20,   100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  20,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  20,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  20,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  20,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_HIDE_ANGLE,  20,  100, 1},
  {0, HAND_HIDE_ANGLE,  2,  MONSTER_MIDD_ANGLE,  20,  100, 1},
  {0, HAND_SHOW_ANGLE,  10,  MONSTER_SHOW_ANGLE, 10, 1000, 0},
  {0, HAND_HIDE_ANGLE,  10,  MONSTER_HIDE_ANGLE,  10,  2500, 0},
  {0, HAND_MIDD_ANGLE,  10,  MONSTER_SHOW_ANGLE,  10,  500, 0},
  {0, HAND_HIDE_ANGLE,  10,  MONSTER_HIDE_ANGLE,  10,  500, 0},
  {99, -1, -1, -1, -1, -1, 0},
   // Sekwencja 5:
  {5, HAND_MIDD_ANGLE,  2,  MONSTER_MIDD_ANGLE,  1,   400, 1},
  {0, HAND_SHOW_ANGLE,  1,  MONSTER_SHOW_ANGLE, 20,  1000, 0},
  {0, HAND_HIDE_ANGLE,  20, MONSTER_HIDE_ANGLE, 20,  100, 0},
  {99, -1, -1, -1, -1, -1, 0},
    // Sekwencja 6:
  {6, HAND_MIDD_ANGLE,  2,  MONSTER_MIDD_ANGLE,  1,   1000, 1},
  {0, HAND_SHOW_ANGLE + 10,  1,  MONSTER_SHOW_ANGLE, 1,  1000, 1},
  {0, HAND_SHOW_ANGLE + 5,   1, MONSTER_SHOW_ANGLE, 20,  1000, 1},
  {0, HAND_SHOW_ANGLE,    1, MONSTER_SHOW_ANGLE, 20,  1000, 0},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  200, 0},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  200, 1},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  200, 0},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  200, 1},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  200, 0},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  200, 1},
  {0, HAND_HIDE_ANGLE,  1,  MONSTER_HIDE_ANGLE,  1,  500, 0},
  {99, -1, -1, -1, -1, -1, 0},



};


volatile bool TimeForCheckDistance = false;
volatile bool UpdateServos = false;
volatile byte distanceCounter = 0;  // Licznik dla rzadszych pomiarów
volatile int LastRespNo = 0;


void setup() {
  // Serial.begin(9600);  // Wyłączony dla oszczędności pamięci
  
  // KONFIGURACJA PINÓW LED I SERW
  pinMode(LED_GREEN_PIN, OUTPUT);           // Pin 7 jako wyjście dla LED GREEN
  pinMode(LED_RED_PIN, OUTPUT);             // Pin 8 jako wyjście dla LED RED
  pinMode(TeddyHand.servoPin, OUTPUT);          // Pin 3 jako wyjście dla serwo 1
  pinMode(TeddyHide.servoPin, OUTPUT);  // Pin 5 jako wyjście dla serwo 2
  delay(20);
  TeddyHand.servo.attach(TeddyHand.servoPin);   // Pin 3
  TeddyHide.servo.attach(TeddyHide.servoPin);   // Pin 5
  TeddyHand.servo.write(TeddyHand.initPos);
  TeddyHide.servo.write(TeddyHide.initPos);
  delay(50);
  TeddyHand.servo.detach();
  TeddyHide.servo.detach();
  // Najpierw zainicjuj VL53L0X (bez przerwań)
  Config_VL53L0X();

  // Dopiero potem uruchom timer (żeby nie zakłócać I2C)
  setupTimer2();
  
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);
  LastRespNo = 0;

  delay(20);
}

// Funkcja do wykonywania konkretnej sekwencji na podstawie numeru
void executeSequence(int sequenceNumber) {
  // Usuń print start sekwencji dla oszczędności pamięci
  // Serial.print("  Sekwencja:");
  // Serial.println(sequenceNumber);
  
  // Znajdź początek sekwencji
  int startIndex = -1;
  for (int i = 0; i < sizeof(moveSequences) / sizeof(moveSequences[0]); i++) {
    if (pgm_read_word(&moveSequences[i][0]) == sequenceNumber) {
      startIndex = i;
      break;
    }
  }
  
  if (startIndex == -1) {
    return; // Błąd - nie znaleziono sekwencji
  }
  
  // Wykonuj kroki sekwencji
  int currentStep = startIndex;
  
  while (pgm_read_word(&moveSequences[currentStep][0]) != 99) {  // Dopóki nie koniec sekwencji (99)
    // Odczytaj wartości z PROGMEM
    TeddyHand.reqPos      = pgm_read_word(&moveSequences[currentStep][1]);
    TeddyHand.deltaAngle  = pgm_read_word(&moveSequences[currentStep][2]);      // Prędkość ręki
    TeddyHide.reqPos      = pgm_read_word(&moveSequences[currentStep][3]);   // Kąt potwora
    TeddyHide.deltaAngle  = pgm_read_word(&moveSequences[currentStep][4]);   // Prędkość potwora

    // Czekaj na osiągnięcie pozycji
    while (TeddyHand.currentPos != TeddyHand.reqPos || TeddyHide.currentPos != TeddyHide.reqPos) {
      UpdateServosPosition();
    }
    // Ustaw LED zgodnie z macierzą (odczytaj z PROGMEM)
    if (pgm_read_word(&moveSequences[currentStep][6]) == 1) {
      LedDetection();
    } else {
      LedNoDetection();
    }
     
    // Wykonaj delay bez printów (odczytaj z PROGMEM)
    int delayTime = pgm_read_word(&moveSequences[currentStep][5]);
    if (delayTime > 0) {
      delay(delayTime);
    }
    
    currentStep++;
  }
  
  // Usuń print końca sekwencji dla oszczędności pamięci
  // Serial.println("END");
}

void loop() {
  if (TimeForCheckDistance) {
    TimeForCheckDistance = false; // Resetuj flagę
    if (!contactDetected && responseNo == 0) {
      currentDistance = measureDistance();
    }
  }

  // Debouncing - wymaga kilku pomiarów z rzędu + bardziej konserwatywny próg
  if (!contactDetected && 
      currentDistance < 80 &&          // Zmniejszony próg z 100 na 80mm
      currentDistance > 10 &&          // Dodany dolny próg (eliminuje 0 i bardzo małe wartości)
      currentDistance != -1 &&
      responseNo == 0) {

      // Usuń printy dla oszczędności pamięci
      // Serial.println();  // Pierwsza pusta linia
      // Serial.println();  // Druga pusta linia
      // Serial.print("D:");
      // Serial.println(currentDistance);
    
      LedDetection();
      contactDetected = true;
      responseNo = LastRespNo + 1;
      if (responseNo > 6) responseNo = 1;  // Zakładamy 6 sekwencji, cyklowanie
      LastRespNo = responseNo;


      TeddyHand.servo.attach(TeddyHand.servoPin);
      TeddyHide.servo.attach(TeddyHide.servoPin);
  }

  if (responseNo != 0) callResponseNo(responseNo);
  delay(100);
}


void callResponseNo(int responceNo) {
  if (responceNo == 0) return;
  executeSequence(responceNo);
  // RESET stanu po wykonaniu sekwencji - BARDZO WAŻNE!
  TeddyHand.servo.detach();
  TeddyHide.servo.detach();
  responseNo = 0;
  contactDetected = false;
  currentDistance = -1;
  delay(1000);  // Pauza przed kolejnym cyklem
}


void UpdateServosPosition() {
  if (!UpdateServos) return;

  UpdateServos = false;
  if (TeddyHand.currentPos == TeddyHand.reqPos && TeddyHide.currentPos == TeddyHide.reqPos) return;

  if (TeddyHand.currentPos != TeddyHand.reqPos) {
    // Aktualizacja pozycji serw TeddyHand
    if (TeddyHand.currentPos < TeddyHand.reqPos) {
      TeddyHand.currentPos += TeddyHand.deltaAngle;
    } else if (TeddyHand.currentPos > TeddyHand.reqPos) {
      TeddyHand.currentPos -= TeddyHand.deltaAngle;
    }
    if (TeddyHand.currentPos < HAND_SHOW_ANGLE) TeddyHand.currentPos = HAND_SHOW_ANGLE;
    if (TeddyHand.currentPos > HAND_HIDE_ANGLE) TeddyHand.currentPos = HAND_HIDE_ANGLE;
    TeddyHand.servo.write(TeddyHand.currentPos);
  }
  if (TeddyHide.currentPos != TeddyHide.reqPos) {
  // Aktualizacja pozycji serw TeddyHide
    if (TeddyHide.currentPos < TeddyHide.reqPos) {
      TeddyHide.currentPos += TeddyHide.deltaAngle;
    } else if (TeddyHide.currentPos > TeddyHide.reqPos) {
      TeddyHide.currentPos -= TeddyHide.deltaAngle;
    }
    if (TeddyHide.currentPos < MONSTER_SHOW_ANGLE) TeddyHide.currentPos = MONSTER_SHOW_ANGLE;
    if (TeddyHide.currentPos > MONSTER_HIDE_ANGLE) TeddyHide.currentPos = MONSTER_HIDE_ANGLE;
    TeddyHide.servo.write(TeddyHide.currentPos);
  }
  delay(25);  // Zwiększony delay z 10ms na 25ms (zgodnie z nowym timerem)
}

void setupTimer2() {
  // Wyłącz przerwania podczas konfiguracji
  cli();
  
  // Timer2 - bezpieczny, nie używany przez millis()
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  
  // Dla ~25ms przy prescaler 1024 (wolniejsze update servo)
  OCR2A = 389;               // Wartość dla 25ms zamiast 10ms
  
  // Tryb CTC dla Timer2
  TCCR2A |= (1 << WGM21);    // WGM21 dla Timer2
  
  // Prescaler 1024 dla Timer2 (stabilny)
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // 1024 dla Timer2
  
  // Przerwanie Timer2
  TIMSK2 |= (1 << OCIE2A);
  
  sei();
}

ISR(TIMER2_COMPA_vect) {
  distanceCounter++;

  // Pomiar odległości co 75ms (3 × 25ms) - zachowujemy częstotliwość
  if (distanceCounter >= 3) {
    TimeForCheckDistance = true;
    distanceCounter = 0;
  }
  
  UpdateServos = true; // Serwa teraz co 25ms (wolniejsze)
}

int measureDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  measure.RangeMilliMeter = -1; 
  lox.rangingTest(&measure, false);
  
  // Sprawdź status pomiaru
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    int distance = measure.RangeMilliMeter;
    
    // Filtrowanie nieprawdopodobnych wartości
    if (distance > 0 && distance < 2000) {  // Zakres 0-2000mm
      return distance;
    }
  }
  return -1;
}

void Config_VL53L0X() {
  while (!lox.begin()) {
    delay(100);
  }
  lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
                    VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW,
                    VL53L0X_INTERRUPTPOLARITY_LOW);

  // Set Interrupt Treashholds
  // Low reading set to 50mm  High Set to 100mm
  FixPoint1616_t LowThreashHold = (50 * 65536.0);
  FixPoint1616_t HighThreashHold = (100 * 65536.0);
  lox.setInterruptThresholds(LowThreashHold, HighThreashHold, true);

  // Enable Continous Measurement Mode
  lox.setDeviceMode(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, false);

  lox.startMeasurement();
}

void LedNoDetection() {
  delay(200);
  // Przywróć stan domyślny: zielona ON, czerwona OFF
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW); 
}

void LedDetection() {
  // Zapal czerwoną diodę, zgaś zieloną
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, HIGH);
} 