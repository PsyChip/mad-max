/* Arduino Fuel Gauge Meter
 * Calculates the resistance from fuel meter
 * and display into parallel LiquidCrystal line (faster than i2c)
 * also provides uptime and low fuel notifications
 * root@psychip.net
 * Aug 2022
 */


#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 8);
int minGas = 69;    // FULL
int maxGas = 112;   // EMPTY
int CR = 0;   // control range: (empty-full) = 43

const int buflen = 32;
const int columns = 16;

int buf[buflen];
int bp;
int fval;
int bliprate = 600;
bool blipOn = false;
unsigned long lastBlip = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  CR = maxGas - minGas;
}

int analogPin = A0;
int raw = 0;
int Vin = 5;      // line voltage
float Vout = 0;
float R1 = 1740;  // exact value of the known resistor
float R2 = 0;
float buffer = 0;

// creates a positive value from the measurement
int GetLevel(int value) {
  int x = (value - minGas);
  x = (x * -1) + CR;
  return x;
}

// scales value for 16 column liquid display
int Scale(int value) {
  return ((value / CR) * columns);
}

void CalcAverage() {
  for (int x = 0; x < buflen; x++) {
    raw = raw + buf[x];
  }
  raw = raw / buflen;
}

void CalcResistance() {
  buffer = raw * 5;
  Vout = (buffer) / 1024.0;
  buffer = (Vin / Vout) - 1;
  R2 = R1 * buffer;
}

void Counter() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  if (minutes == 0 && hours == 0) {
    lcd.print(seconds);
    lcd.print("s");
  }
  if (hours == 0) {
    lcd.print(minutes);
    lcd.print("m");
  }
  if (hours == 0) {
    lcd.print(hours);
    lcd.print("h");
    lcd.print(minutes);
    lcd.print("m");
  }
}

void loop() {
  buf[bp] = analogRead(analogPin);
  bp++;

  // proceed to calculation when memory is full
  if (bp == (buflen - 1)) {
    bp = 0;

    CalcAverage();
    CalcResistance();

    lcd.setCursor(0, 0);
    lcd.print(trunc(R2));
    lcd.setCursor(3, 0);
    lcd.print(F(" / "));
    
    Counter();
    int level = Scale((GetLevel(R2)));
    lcd.setCursor(0, 1);

    if (level <= 1) {
      unsigned long now = millis();
      if ((now - lastBlip) >= bliprate) {
        lastBlip = now;
        if (blipOn == true) {
          lcd.print(F("YAKIT YETERSIZ!"));
        } else {
          lcd.print(F("                "));
        }
        blipOn = !blipOn;
      }
    } else {
      for (int b = 0; b < columns; b++) {
        if (level >= b) {
          lcd.print(F("#"));
        } else {
          lcd.print(F("-"));
        }
      }
    }
  }
}
