# Arduino Skrzynka - Interaktywny System Servo i LED

## 📋 Opis Projektu

Zaawansowany system sterowania dla Arduino Nano z interaktywną zabawką wykorzystującą czujnik odległości VL53L0X, dwa serwomechanizmy i diody LED. System reaguje na zbliżenie obiektu wykonując sekwencje ruchów z synchronizacją świetlną.

## 🔧 Komponenty

### Sprzęt
- **Arduino Nano** (ATmega328P)
- **VL53L0X** - czujnik odległości laserowy (I2C)
- **2x Serwomechanizm** - kontrola ruchu "ręki" i "potwora"
- **2x Dioda LED** - zielona i czerwona
- **Rezystory** - 220Ω dla LED
- **Płytka stykowa** - do połączeń
- **Przewody** - męsko-męskie

### Biblioteki
```cpp
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
```

## 🔌 Schemat Połączeń

### Piny Arduino Nano:
```
┌─────────────────────────────────────┐
│           Arduino Nano              │
├─────────────────────────────────────┤
│ VCC  ──── +5V (zasilanie)          │
│ GND  ──── GND (masa)               │
│ A4   ──── SDA (VL53L0X)            │
│ A5   ──── SCL (VL53L0X)            │
│ D3   ──── Servo "Ręka" (PWM)       │
│ D5   ──── Servo "Potwór" (PWM)     │
│ D7   ──── LED Zielona (+)          │
│ D8   ──── LED Czerwona (+)         │
└─────────────────────────────────────┘
```

### Szczegółowe Połączenia:

#### VL53L0X (I2C):
```
VL53L0X    Arduino Nano
VCC   ──── 3.3V lub 5V
GND   ──── GND
SDA   ──── A4
SCL   ──── A5
```

#### Serwomechanizmy:
```
Servo "Ręka"       Arduino Nano
Czerwony   ────── +5V
Czarny     ────── GND
Żółty      ────── D3

Servo "Potwór"     Arduino Nano  
Czerwony   ────── +5V
Czarny     ────── GND
Żółty      ────── D5
```

#### Diody LED:
```
LED Zielona    Arduino Nano
Katoda (-)  ── GND
Anoda (+)   ── D7 ── [220Ω] ── +5V

LED Czerwona   Arduino Nano
Katoda (-)  ── GND  
Anoda (+)   ── D8 ── [220Ω] ── +5V
```

## 🎮 Funkcjonalność

### Podstawowe Działanie:
1. **Wykrywanie odległości** - VL53L0X monitoruje przestrzeń co 75ms
2. **Debouncing** - wymaga 8 pomiarów z rzędu < 80mm dla aktywacji  
3. **Wykonanie sekwencji** - uruchomienie jednej z 6 predefiniowanych sekwencji
4. **Reset** - powrót do stanu oczekiwania po 1 sekundzie

### Parametry Sekwencji:
Każda sekwencja definiowana przez macierz 7 parametrów:
```cpp
{seq_number, kąt_ręki, delta_hand, kąt_potwora, delta_monster, delay_ms, led_mode}
```

- **seq_number**: 1-6 (start), 0 (kontynuacja), 99 (koniec)
- **kąt_ręki**: 0° (show) - 120° (hide)  
- **delta_hand**: prędkość ruchu ręki (stopni/krok)
- **kąt_potwora**: 80° (show) - 140° (hide)
- **delta_monster**: prędkość ruchu potwora (stopni/krok)
- **delay_ms**: opóźnienie na końcu kroku
- **led_mode**: 0 (zielona), 1 (czerwona)

## 📊 Optymalizacje Pamięci

### Implementacja PROGMEM:
- **Macierz sekwencji w ROM**: ~350 bajtów RAM zaoszczędzone
- **#define zamiast const int**: 8 bajtów RAM zaoszczędzone
- **Usunięcie debug printów**: ~60-80 bajtów zaoszczędzone

### Łączne oszczędności: **~440-460 bajtów RAM (20% pamięci Arduino Nano)**

## ⚙️ Konfiguracja

### Progi Wykrywania:
```cpp
#define DETECTION_THRESHOLD 8    // 8 pomiarów z rzędu (600ms)
const int distanceThreshold = 80; // 80mm próg wykrywania
```

### Pozycje Servo:
```cpp
#define HAND_HIDE_ANGLE 120   // Ręka schowana
#define HAND_SHOW_ANGLE 0     // Ręka widoczna  
#define HAND_MIDD_ANGLE 50    // Ręka w środku

#define MONSTER_HIDE_ANGLE 140  // Potwór schowany
#define MONSTER_SHOW_ANGLE 80   // Potwór widoczny
#define MONSTER_MIDD_ANGLE 120  // Potwór w środku
```

### Timer Configuration:
```cpp
Timer2: 25ms period (slower, smoother servo movement)
Distance check: every 75ms (3 × 25ms)
Servo update: every 25ms
```

## 🚀 Instalacja

1. **Zainstaluj biblioteki Arduino IDE:**
   ```
   Adafruit VL53L0X Library
   Servo Library (wbudowana)
   ```

2. **Podłącz komponenty według schematu**

3. **Wgraj kod na Arduino Nano**

4. **Test działania:**
   - Zielona LED powinna świecić w stanie spoczynku
   - Zbliż rękę/obiekt < 8cm
   - Observe sequence execution with LED synchronization

## 📁 Struktura Plików

```
skrzynka/
├── skrzynka.ino      # Główny plik programu
├── include.h         # Definicje struktur i funkcji  
├── README.md         # Ten plik
└── commit_message.txt # Opis zmian
```

## 🎯 Sekwencje Dostępne

- **Sekwencja 1**: Proste wyskoczenie
- **Sekwencja 2**: Złożona animacja z pauzami  
- **Sekwencja 3**: Seria krótkich ruchów
- **Sekwencja 4**: Migotanie potwora
- **Sekwencja 5**: Szybka akcja
- **Sekwencja 6**: Animacja z efektami LED (aktualnie aktywna)

## 🔧 Troubleshooting

### Problem: Program się wiesza
- **Rozwiązanie**: Zaimplementowano PROGMEM i optymalizacje pamięci

### Problem: Servo rusza się zbyt szybko
- **Rozwiązanie**: Zwiększono okres timera do 25ms

### Problem: Fałszywe wykrycia
- **Rozwiązanie**: Debouncing 8 pomiarów z rzędu

## 🏗️ Rozwój

### Możliwe rozszerzenia:
- [ ] Dodanie czujnika PIR
- [ ] Więcej sekwencji ruchów
- [ ] Obsługa dźwięku (buzzer)
- [ ] Komunikacja WiFi/Bluetooth
- [ ] Menu konfiguracyjne

## 📜 Licencja

MIT License - projekt edukacyjny/hobbystyczny.

---
**Autor**: Daniel  
**Data**: Listopad 2025  
**Wersja**: 1.0 (Memory Optimized)