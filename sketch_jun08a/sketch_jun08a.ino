/* This sketch triggering events based on
 *  voltage measurements 
 *  @PsyChip
 *  root@psychip.net
 *  June 2022
 */


#include <LiquidCrystal.h>
const int voltageSensor = A0;

float vcc = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float lastVCC;

float mars = 11.25;
float start = 12.55;
float idle[2] = {11.50, 12.20};
float dead = 7.5;
int state = 0;
int oldstate = 0;
int statedelay = 500;
unsigned long now;
unsigned long lastState;
unsigned long updateVCC;
int idleRate = 600;

unsigned long lastpoll;
int pollrate = 200;

LiquidCrystal lcd(12, 11, 5, 4, 3, 8);

void setup() {
  lcd.begin(16, 2);
  lcd.print("Hazir..");
  Serial.begin(9600);
  delay(1000);
}

double readVcc() {
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result;
  return result / 1024.0;
}

double readVoltage() {
  return ((analogRead(voltageSensor) * readVcc()) / 1024.0) / (R2 / (R1 + R2));
}

void parseVCC() {
  if (vcc == 0 ) {
    state = 0;
  }
  if (vcc > 0 && vcc <= dead) {
    state = 1;
  }
  if (vcc >= idle[0] && vcc <= idle[1]) {
    if ((now - updateVCC) >= idleRate) {
      state = 2;
    }
  }
  if (vcc > 0 && vcc <= mars) {
    state = 3;
  }
  if (vcc >= start) {
    state = 4;
  }
}

void checkState() {
  if (state != oldstate && (now - lastState) >= statedelay) {
    lcd.setCursor(0, 1);
    switch (state) {
      case 0: {
          lcd.print("Kontak Kapali        ");
        }
        break;
      case 1: {
          lcd.print("Akü Bos          ");
        }
        break;
      case 2: {
          lcd.print("Motor Durdu        ");
        }
        break;
      case 3: {
          lcd.print("Calistiriliyor..       ");
        }
        break;
      case 4: {
          lcd.print("Calisiyor..      ");
        }
    }
    oldstate = state;
    lastState = now;
  }
}

void loop() {
  now = millis();
  if ((now - lastpoll) >= pollrate) {
    lastpoll = now;
    vcc = readVoltage();
  }

  if (vcc != lastVCC) {
    lastVCC = vcc;
    updateVCC = now;
    lcd.setCursor(0, 0);
    lcd.print("VCC =         ");
    lcd.setCursor(9, 0);
    lcd.print(vcc);
    parseVCC();
    checkState();
  }
}
