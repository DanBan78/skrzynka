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

byte contactDetected = 0;
int currentDistance = 0;
byte responseNo = 0;

volatile bool TimeForCheckDistance = false;
volatile bool UpdateServos = false;
volatile byte distanceCounter = 0;  // Licznik dla rzadszych pomiarów
volatile int LastRespNo = 0;


void setup() {
  Serial.begin(9600);  // Włączony dla printu sekwencji
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
  TeddyHand.servo.detach();
  TeddyHide.servo.detach();
  delay(300);
  // Najpierw zainicjuj VL53L0X
  Config_VL53L0X();
  // uruchom timer (żeby nie zakłócać I2C)
  setupTimer2();
  
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);
  delay(100);
}


void loop() {
  if (TimeForCheckDistance) {
    TimeForCheckDistance = false; // Resetuj flagę
    if (!contactDetected && responseNo == 0) {
      currentDistance = measureDistance();
    }
  }

  if (!contactDetected && currentDistance != -1 &&
      currentDistance < 80 && currentDistance > 10 &&
      responseNo == NO_ACTIVE_SEQUENCE) {
    
    LedDetection();
    contactDetected = true;
    responseNo = LastRespNo + 1;
    if (responseNo > 9) responseNo = 1;  // Zakładamy 9 sekwencji, cyklicznie
    LastRespNo = responseNo;

    TeddyHand.servo.attach(TeddyHand.servoPin);
    TeddyHide.servo.attach(TeddyHide.servoPin);
  }

  if (responseNo != NO_ACTIVE_SEQUENCE) callResponseNo(responseNo);
  delay(100);
}


void callResponseNo(int responceNo) {
  if (responceNo == NO_ACTIVE_SEQUENCE) return;
  executeSequence(responceNo);
  // RESET stanu po wykonaniu sekwencji - BARDZO WAŻNE!
  TeddyHand.servo.detach();
  TeddyHide.servo.detach();
  responseNo = NO_ACTIVE_SEQUENCE;
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
      if (TeddyHand.currentPos > TeddyHand.reqPos) TeddyHand.currentPos = TeddyHand.reqPos;
    } else if (TeddyHand.currentPos > TeddyHand.reqPos) {
      TeddyHand.currentPos -= TeddyHand.deltaAngle;
      if (TeddyHand.currentPos < TeddyHand.reqPos) TeddyHand.currentPos = TeddyHand.reqPos;
    }
    TeddyHand.servo.write(TeddyHand.currentPos);
  }
  if (TeddyHide.currentPos != TeddyHide.reqPos) {
  // Aktualizacja pozycji serw TeddyHide
    if (TeddyHide.currentPos < TeddyHide.reqPos) {
      TeddyHide.currentPos += TeddyHide.deltaAngle;
      if (TeddyHide.currentPos > TeddyHide.reqPos) TeddyHide.currentPos = TeddyHide.reqPos;
    } else if (TeddyHide.currentPos > TeddyHide.reqPos) {
      TeddyHide.currentPos -= TeddyHide.deltaAngle;
      if (TeddyHide.currentPos < TeddyHide.reqPos) TeddyHide.currentPos = TeddyHide.reqPos;
    }
    TeddyHide.servo.write(TeddyHide.currentPos);
  }
  delay(25);  // Zwiększony delay z 10ms na 25ms (zgodnie z nowym timerem)
}


void executeSequence(int sequenceNumber) {
  Serial.println(""); Serial.print("SEQ:"); Serial.println(sequenceNumber);       // Print numeru wykonywanej sekwencji
  // Znajdź początek sekwencji
  int startIndex = -1;
  for (int i = 0; i < sizeof(moveSequences) / sizeof(moveSequences[0]); i++) {
    if (pgm_read_word(&moveSequences[i][0]) == sequenceNumber) {
      startIndex = i;
      break;
    }
  }
  if (startIndex == -1) return; // Błąd - nie znaleziono sekwencji

  int currentStep = startIndex;
  while (pgm_read_word(&moveSequences[currentStep][0]) != 99) {         // Dopóki nie koniec sekwencji (99)
    Serial.print("Step:"); Serial.println(currentStep - startIndex);    // Print numeru kroku
    // Odczytaj wartości z PROGMEM
    TeddyHand.reqPos      = pgm_read_word(&moveSequences[currentStep][1]);   // Kąt ręki
    TeddyHand.deltaAngle  = pgm_read_word(&moveSequences[currentStep][2]);   // Prędkość ręki
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
     
    // Wykonaj delay
    int delayTime = pgm_read_word(&moveSequences[currentStep][5]);
    if (delayTime > 0) {
      delay(delayTime);
    }
    currentStep++;
  }
}


void setupTimer2() {
  cli();  // Wyłącz przerwania podczas konfiguracji
  // Timer2
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
  // Włącz przerwania globalnie
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
    return distance;
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