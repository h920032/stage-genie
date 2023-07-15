#include <EEPROM.h>
#define RECODRD_LED 4
#define TIMER_LED 8
#define NORMAL_LED 6
#define RELAY_LED 2
#define KEY1 A2
#define KEY2 A1
#define KEY3 A4
#define KEY4 A3
#define jdq 11
#define MAX_INDEX 900
#define EEPROM_SIZE 1024

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  digitalWrite(3, 0);
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
  pinMode(RECODRD_LED, OUTPUT);
  pinMode(TIMER_LED, OUTPUT);
  pinMode(NORMAL_LED, OUTPUT);
  pinMode(RELAY_LED, OUTPUT);
  pinMode(KEY1, INPUT_PULLUP);
  pinMode(KEY2, INPUT_PULLUP);
  pinMode(KEY3, INPUT_PULLUP);
  pinMode(KEY4, INPUT_PULLUP);
  digitalWrite(RECODRD_LED, HIGH);
  digitalWrite(TIMER_LED, HIGH);
  digitalWrite(NORMAL_LED, HIGH);
  digitalWrite(RELAY_LED, HIGH);
  delay(1000);
  digitalWrite(RECODRD_LED, 0);
  digitalWrite(TIMER_LED, 0);
  digitalWrite(NORMAL_LED, 1);
  digitalWrite(RELAY_LED, 0);
}

unsigned long times[2] = {0, 0}; // 記錄運行時間
unsigned long timestart = 0;

int state = 0;
int index = 0;
int order = 0;
int value = 0;
int end_index = 0;

void loop()
{
  if (digitalRead(KEY1) == 0  || index >= MAX_INDEX) // Clearning data
  {
    delay(10);
    if (digitalRead(KEY1) == 0  || index >= MAX_INDEX)
    {
      unsigned long button_time_start = 0;
      unsigned long button_time_end = 0;
      button_time_start = millis();
      button_time_end = millis();
      while (digitalRead(KEY1) == 0 && (button_time_end - button_time_start) < 5000)
      {
        button_time_end = millis();
      }
      Serial.println("key1");
      if (state == 0 && ((button_time_end - button_time_start) >= 5000))
      {
        for (int i = 0; i < EEPROM_SIZE; i++) {
          EEPROM.write(i, 0);
        }
        writeIntIntoEEPROM(EEPROM_SIZE - 3, 0);
        state = 1;
        index = 0;
        order = 0;
        value = 0;
        digitalWrite(RECODRD_LED, 1);
        digitalWrite(TIMER_LED, 0);
        digitalWrite(NORMAL_LED, 0);
        digitalWrite(RELAY_LED, 0);
        while (digitalRead(KEY1) == 0);
        while (digitalRead(KEY2) == 1);
        times[0] = millis();
        times[1] = millis();
      }
      else
      {
        if (state = 1) {
          if (index < MAX_INDEX) {
            EEPROM.write(index, value);
            index++;
            writeIntIntoEEPROM(EEPROM_SIZE - 3, index);
          }
        }
        state = 0;
        order = 0;
        value = 0;
        index = 0;
        end_index = 0;
        digitalWrite(RECODRD_LED, 0);
        digitalWrite(TIMER_LED, 1);
        digitalWrite(NORMAL_LED, 0);
        digitalWrite(RELAY_LED, 0);
        while (digitalRead(KEY1) == 0);
        digitalWrite(RECODRD_LED, 0);
        digitalWrite(TIMER_LED, 0);
        digitalWrite(NORMAL_LED, 1);
        digitalWrite(RELAY_LED, 0);
      }
    }
  }
  else if (digitalRead(KEY2) == 0 && state == 0)
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
        end_index = readIntFromEEPROM(EEPROM_SIZE - 3);
        times[0] = millis();
        times[1] = millis();
      }
    }
  }
  else if (digitalRead(KEY4) == 0 && state == 0) // Dump data to serial
  {
    /*
     * Enable communicate with serial
     */
    delay(10);
    if (digitalRead(KEY4) == 0) {
      Serial.println("key4");
      for (int i = 0; i < MAX_INDEX; i++) {
        Serial.print(EEPROM.read(i));
        Serial.print(" ,");
      }
    }
  }
  if (state == 1 && millis() >= times[0]) //Learning
  {
    times[0] += 100;
    if (millis() >= times[1]) {
      digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
      times[1] += 500;
    }
    if (digitalRead(KEY3) == 0)
    {
      // Serial.println("key3");
      value = value ^ (1 << (7 - order));
      // Serial.print(value);
      digitalWrite(jdq, 0);
      digitalWrite(RELAY_LED, 1);
    }
    else
    {
      digitalWrite(jdq, 1);
      digitalWrite(RELAY_LED, 0);
    }
    order++;
    if (order >= 8) {
      EEPROM.write(index, value);
      order = 0;
      value = 0;
      index++;
      writeIntIntoEEPROM(EEPROM_SIZE - 3, index);
    }
  }
  if (state == 2  && millis() >= times[0]) // Trigger
  {
    times[0] += 100;
    if (millis() >= times[1]) {
      digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
      times[1] += 500;
    }
    if (order == 0) {
      value = EEPROM.read(index);
    }
    if ((value >> (7 - order)) & 1) {
      digitalWrite(jdq, 0);
      digitalWrite(RELAY_LED, 1);
    }
    else {
      digitalWrite(jdq, 1);
      digitalWrite(RELAY_LED, 0);
    }
    order++;
    if (order >= 8) {
      order = 0;
      index++;
    }
    if (index >= end_index) {
      state = 0;
      order = 0;
      index = 0;
      value = 0;
      end_index = 0;
      digitalWrite(RECODRD_LED, 0);
      digitalWrite(TIMER_LED, 0);
      digitalWrite(NORMAL_LED, 1);
      digitalWrite(RELAY_LED, 0);
    }
  }
}
