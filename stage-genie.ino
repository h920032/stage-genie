#include <EEPROM.h>

// Defining the pin numbers for various components
#define RECODRD_LED 4
#define TIMER_LED 8
#define NORMAL_LED 6
#define RELAY_LED 2
#define KEY1 A2
#define KEY2 A1
#define KEY3 A4
#define KEY4 A3
#define jdq 11  // Relay control pin
#define MAX_INDEX 900
#define EEPROM_SIZE 1024

// Function to write an integer into EEPROM at a given address
void writeIntIntoEEPROM(int address, int number) {
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

// Function to read an integer from EEPROM at a given address
int readIntFromEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void setup() {
  // Initializing serial communication, setting up the pins and initial state of
  // LEDs
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
  digitalWrite(jdq, 0);
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
  delay(500);
  digitalWrite(RECODRD_LED, 0);
  digitalWrite(TIMER_LED, 0);
  digitalWrite(NORMAL_LED, 1);
  digitalWrite(RELAY_LED, 0);
}

unsigned long times[2] = {0, 0};  // Array to record running times
unsigned long timestart = 0;
unsigned long button_time_start = 0;
unsigned long button_time_end = 0;

// Defining state variables
int state = 0;
int index = 0;
int order = 0;
uint8_t value = 0;
int end_index = 0;
int key = 0;

uint8_t timeArray[MAX_INDEX] = {0};

void loop() {
  // In State 0
  if (state == 0) {
    // Normal LED on, others off
    // Relay off

    // KEY1 pressed for 5 seconds: Clear EEPROM and go to state 1
    // KEY2 pressed: Go to state 2
    // KEY3 pressed: Manually control the relay
    // KEY4 pressed: Dump EEPROM data to serial
    digitalWrite(jdq, 0);
    digitalWrite(RECODRD_LED, 0);
    digitalWrite(TIMER_LED, 0);
    digitalWrite(NORMAL_LED, 1);
    digitalWrite(RELAY_LED, 0);

    if (digitalRead(KEY1) == 0)  // Clearning data
    {
      if (key != 1) {
        button_time_start = millis();
        button_time_end = millis();
        key = 1;
      } else {
        button_time_end = millis();
      }
      // Serial.println("key1");
      if ((button_time_end - button_time_start) >= 4000) {
        digitalWrite(RECODRD_LED, 1);
        digitalWrite(TIMER_LED, 1);
        digitalWrite(NORMAL_LED, 0);
        digitalWrite(RELAY_LED, 0);
        key = 0;
        for (int i = 0; i < EEPROM_SIZE; i++) {
          EEPROM.write(i, 0);
        }
        writeIntIntoEEPROM(EEPROM_SIZE - 3, 0);
        state = 1, index = 0, order = 0, value = 0;
        for (int i = 0; i < MAX_INDEX; i++) {
          timeArray[i] = 0;
        }
        digitalWrite(RECODRD_LED, 1);
        digitalWrite(TIMER_LED, 0);
        digitalWrite(NORMAL_LED, 0);
        digitalWrite(RELAY_LED, 0);
        while (digitalRead(KEY1) == 0)
          ;
        while (digitalRead(KEY2) == 1)
          ;
        times[0] = millis();
        times[1] = millis();
      }
    } else if (digitalRead(KEY2) == 0)  // press trigger button
    {
      if (key != 2) {
        button_time_start = millis();
        button_time_end = millis();
        key = 2;
      } else {
        button_time_end = millis();
      }
      if ((button_time_end - button_time_start) >= 10) {
        key = 0;
        // Serial.println("key2");
        if (state == 0) {
          state = 2, order = 0, index = 0, value = 0;
          end_index = readIntFromEEPROM(EEPROM_SIZE - 3);
          times[0] = millis();
          times[1] = millis();
        }
      }
    } else if (digitalRead(KEY3) == 0) {
      while (digitalRead(KEY3) == 0) {
        digitalWrite(jdq, 1);
        digitalWrite(RELAY_LED, 1);
      }
    } else if (digitalRead(KEY4) == 0)  // Dump data to serial
    {
      /*
         Enable communicate with serial
      */
      if (key != 4) {
        button_time_start = millis();
        button_time_end = millis();
        key = 4;
      } else {
        button_time_end = millis();
      }
      if ((button_time_end - button_time_start) >= 10) {
        key = 0;
        // Serial.println("key4");
        for (int i = 0; i < MAX_INDEX; i++) {
          Serial.print(EEPROM.read(i));
          Serial.print(" ,");
        }
      }
    } else {
      key = 0;
    }
  }
  // In State 1: "Learning" state
  else if (state == 1) {
    // Recording relay states every 100ms
    // Exit to state 0 if KEY1 is pressed or if we've recorded MAX_INDEX states
    if (digitalRead(KEY3) == 0) {
      digitalWrite(jdq, 1);
      digitalWrite(RELAY_LED, 1);
    } else {
      digitalWrite(jdq, 0);
      digitalWrite(RELAY_LED, 0);
    }
    if (millis() >= times[0])  // Learning
    {
      times[0] += 100;
      if (digitalRead(KEY3) == 0) {
        // Serial.println("key3");
        value = value ^ (1 << (7 - order));
      }
      order++;
      if (order >= 8) {
        // EEPROM.write(index, value);
        timeArray[index] = value;
        order = 0;
        value = 0;
        index++;
        // writeIntIntoEEPROM(EEPROM_SIZE - 3, index);
      }
    }
    if (millis() >= times[1]) {
      digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
      times[1] += 500;
    }
    if (digitalRead(KEY1) == 0 || index >= MAX_INDEX) {
      if (key != 1) {
        button_time_start = millis();
        button_time_end = millis();
        key = 1;
      } else {
        button_time_end = millis();
      }
      if ((button_time_end - button_time_start) >= 10 || index >= MAX_INDEX) {
        key = 0;
        digitalWrite(TIMER_LED, 1);
        digitalWrite(RELAY_LED, 0);
        if (index < MAX_INDEX) {
          // EEPROM.write(index, value);
          timeArray[index] = value;
          index++;
          // writeIntIntoEEPROM(EEPROM_SIZE - 3, index);
        }
        for (int i = 0; i < index; i++) {
          Serial.print(timeArray[i]);
          EEPROM.write(i, timeArray[i]);
        }
        writeIntIntoEEPROM(EEPROM_SIZE - 3, index);
        state = 0, order = 0, value = 0, index = 0;
        end_index = 0;
        // delete timeArray;
        // digitalWrite(jdq, 0);
        while (digitalRead(KEY1) == 0)
          ;
      }
    } else {
      key = 0;
    }
  }
  // In State 2: "Trigger" state
  else if (state == 2) {
    // Replay recorded relay states every 100ms
    // Exit to state 0 if KEY1 is pressed or if we've reached the end of the
    // recorded list
    if (millis() >= times[0])  // Trigger
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
        digitalWrite(jdq, 1);
        digitalWrite(RELAY_LED, 1);
      } else {
        digitalWrite(jdq, 0);
        digitalWrite(RELAY_LED, 0);
      }
      order++;
      if (order >= 8) {
        order = 0;
        index++;
      }
    }
    if (digitalRead(KEY1) == 0 || index >= end_index) {
      if (key != 1) {
        button_time_start = millis();
        button_time_end = millis();
        key = 1;
      } else {
        button_time_end = millis();
      }
      if ((button_time_end - button_time_start) >= 10 || index >= end_index) {
        key = 0;
        state = 0, order = 0, value = 0, index = 0;
        end_index = 0;
        digitalWrite(TIMER_LED, 1);
        digitalWrite(RELAY_LED, 0);
        digitalWrite(jdq, 0);
        while (digitalRead(KEY1) == 0)
          ;
      }
    } else {
      key = 0;
    }
  }
}
