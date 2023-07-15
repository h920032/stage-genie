#include <EEPROM.h>
#define LEDR 8
#define LEDG 6
#define LEDB 4
#define KEY1 A2
#define KEY2 A1
#define KEY3 A4
#define KEY4 A3
#define jdq 11

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT);
  digitalWrite(5, 0);
  pinMode(7, OUTPUT);
  digitalWrite(7, 0);
  pinMode(9, OUTPUT);
  digitalWrite(9, 0);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, 0);

  pinMode(jdq, OUTPUT);
  digitalWrite(jdq, 1);
  pinMode(LEDR, OUTPUT);					//定义LED为输出引脚
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(KEY1, INPUT_PULLUP);
  pinMode(KEY2, INPUT_PULLUP);
  pinMode(KEY3, INPUT_PULLUP);
  pinMode(KEY4, INPUT_PULLUP);
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  delay(1000);
  digitalWrite(LEDR, 0);
  digitalWrite(LEDG, 1);
  digitalWrite(LEDB, 0);
}

unsigned long times[2] = {0, 0}; // 記錄運行時間
unsigned long timestart = 0;

int state = 0;
int index = 0;
int order = 0;
int value = 0;

void loop()
{
  if (digitalRead(KEY1) == 0  || index >= 1024) // Clearning data
  {
    delay(10);
    if (digitalRead(KEY1) == 0  || index >= 1024)
    {
      Serial.println("key1");
      if (state == 0)
      {
        for (int i = 0; i < 1024; i++) {
          EEPROM.write(i, 0);
        }
        state = 1;
        index = 0;
        order = 0;
        value = 0;
        digitalWrite(LEDR, 0);
        digitalWrite(LEDG, 0);
        digitalWrite(LEDB, 0);
        while (digitalRead(KEY1) == 0);
        while (digitalRead(KEY2) == 1);
        while (digitalRead(KEY2) == 0);
        times[0] = millis();
        times[1] = millis();
        timestart = millis();
      }
      else
      {
        state = 0;
        // EEPROM.write(index, value);
        order = 0;
        value = 0;
        index = 0;
        digitalWrite(LEDR, 1);
        digitalWrite(LEDG, 0);
        digitalWrite(LEDB, 0);
        while (digitalRead(KEY1) == 0);
        digitalWrite(LEDR, 0);
        digitalWrite(LEDG, 1);
        digitalWrite(LEDB, 0);
      }
    }
    // while (digitalRead(KEY1) == 0);
  }
  if (state == 1 && (millis() - times[0]) >= 100) //Learning
  {
    times[0] = millis();
    if ((millis() - times[1]) >= 1000) {
      digitalWrite(LEDR, !digitalRead(LEDR));
      times[1] = millis();
    }
    if (digitalRead(KEY3) == 0)
    {
      // Serial.println("key3");
      value = value ^ (1 << (7 - order));
      // Serial.print(value);
      digitalWrite(jdq, 0);
      digitalWrite(LEDB, 1);
    }
    else
    {
      digitalWrite(jdq, 1);
      digitalWrite(LEDB, 0);
    }
    order++;
    if (order >= 8) {
      EEPROM.write(index, value);
      order = 0;
      value = 0;
      index++;
    }
  }
  if (digitalRead(KEY2) == 0)
  {
    delay(10);
    if (digitalRead(KEY2) == 0)
    {
      // Serial.println("key2");
      if (state == 0)
      {
        state = 2;
        order = 0;
        index = 0;
        value = 0;
        times[0] = millis();
        times[1] = millis();
        timestart = millis();
      }
    }
  }
  if (state == 2  && (millis() - times[0]) >= 100) // Trigger
  {
    times[0] = millis();
    if ((millis() - times[1]) >= 1000) {
      digitalWrite(LEDR, !digitalRead(LEDR));
      times[1] = millis();
    }
    if (order == 0) {
      value = EEPROM.read(index);
    }
    if ((value >> (7 - order)) & 1) {
      digitalWrite(jdq, 0);
      digitalWrite(LEDB, 1);
    }
    else {
      digitalWrite(jdq, 1);
      digitalWrite(LEDB, 0);
    }
    order++;
    if (order >= 8) {
      order = 0;
      index++;
    }
    if (index >= 1024) {
      state = 0;
      order = 0;
      index = 0;
      value = 0;
      digitalWrite(LEDR, 0);
      digitalWrite(LEDG, 1);
      digitalWrite(LEDB, 0);
    }
  }
  if (digitalRead(KEY4) == 0) // Dump data to serial
  {
    delay(10);
    if (digitalRead(KEY4) == 0) {
      Serial.println("key4");
      for (int i = 0; i < 1024; i++) {
        Serial.print(EEPROM.read(i));
        Serial.print(" ,");
      }
    }
  }
  // delay(100);
}
