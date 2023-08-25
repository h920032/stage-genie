#include <SPIMemory.h>
#include <SoftPWM.h>

// Define component pin assignments
#define RECODRD_LED 6
#define TIMER_LED 8
#define NORMAL_LED 7
#define RELAY_LED 5
#define POWER_LED 9
#define KEY1 A2
#define KEY2 A1
#define KEY3 A4
#define METER A6
#define POWER A5
#define jdq 2  // Relay control pin
#define INDEX_NUM_ADDR 65536
#define MAX_INDEX 64000
#define SPI_PAGESIZE 256

SPIFlash flash;
uint8_t pageBuffer[SPI_PAGESIZE];
uint8_t data_buffer[SPI_PAGESIZE];

unsigned long times[2] = { 0, 0 };  // Store timing data for operations
unsigned long button_time_start = 0;
unsigned long button_time_end = 0;

// State variable declarations
uint8_t value = 0;
uint16_t index = 0;
int order = 0;

int state = 0;
uint16_t end_index = 0;
int key = 0;

void setup() {
  // Initialize serial communication, configure pins and set initial LED states
  Serial.begin(9600);
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

  SoftPWMBegin();
  SoftPWMSet(RECODRD_LED, 0);
  SoftPWMSet(TIMER_LED, 0);
  SoftPWMSet(NORMAL_LED, 0);
  SoftPWMSet(RELAY_LED, 0);
  SoftPWMSetFadeTime(RECODRD_LED, 500, 500);
  SoftPWMSetFadeTime(TIMER_LED, 500, 500);
  SoftPWMSetFadeTime(NORMAL_LED, 500, 500);
  SoftPWMSetFadeTime(RELAY_LED, 500, 500);
  SoftPWMSetPercent(RECODRD_LED, 50);
  SoftPWMSetPercent(TIMER_LED, 50);
  SoftPWMSetPercent(NORMAL_LED, 50);
  SoftPWMSetPercent(RELAY_LED, 50);

  // Start SoftPWM and configure LED fade and brightness
  flash.begin();
  for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
    pageBuffer[i] = 0;
  }
  for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
    data_buffer[i] = 0;
  }
  delay(500);

  // Reset LEDs to off
  SoftPWMSetPercent(RECODRD_LED, 0);
  SoftPWMSetPercent(TIMER_LED, 0);
  SoftPWMSetPercent(NORMAL_LED, 0);
  SoftPWMSetPercent(RELAY_LED, 0);
  delay(500);
  SoftPWMEnd(RECODRD_LED);
  SoftPWMEnd(TIMER_LED);
  SoftPWMEnd(NORMAL_LED);
  SoftPWMEnd(RELAY_LED);

  Serial.println(analogRead(POWER));
  if (analogRead(POWER) < 500) {
    SoftPWMSet(POWER_LED, 0);
    SoftPWMSetFadeTime(POWER_LED, 500, 500);
    SoftPWMSetPercent(POWER_LED, 50);
    delay(250);
    SoftPWMSetPercent(POWER_LED, 0);
    delay(250);
    SoftPWMSetPercent(POWER_LED, 50);
    delay(250);
    SoftPWMSetPercent(POWER_LED, 0);
    delay(250);
    SoftPWMSetPercent(POWER_LED, 50);
    delay(250);
    SoftPWMSetPercent(POWER_LED, 0);
    delay(250);
  }

  pinMode(RECODRD_LED, OUTPUT);
  pinMode(TIMER_LED, OUTPUT);
  pinMode(NORMAL_LED, OUTPUT);
  pinMode(RELAY_LED, OUTPUT);
  digitalWrite(RECODRD_LED, 0);
  digitalWrite(TIMER_LED, 0);
  digitalWrite(NORMAL_LED, 1);
  digitalWrite(RELAY_LED, 0);
}

void loop() {
  switch (state) {
    // Control logic based on current state
    // In State 0
    case 0:
      {
        // Normal LED on, others off
        // Relay off

        // Default state: NORMAL LED on, others off, relay off
        // KEY1 for 5 seconds: Clear flash memory and transition to state 1
        // KEY2: Transition to state 2
        // KEY3: Manual relay control
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
          if ((button_time_end - button_time_start) >= 5000) {
            digitalWrite(RECODRD_LED, 1);
            digitalWrite(TIMER_LED, 1);
            digitalWrite(NORMAL_LED, 0);
            digitalWrite(RELAY_LED, 0);
            key = 0;

            flash.eraseChip();
            // flash.writeWord(INDEX_NUM_ADDR, 0);
            state = 1, index = 0, order = 0, value = 0, end_index = 0;
            for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
              pageBuffer[i] = 0;
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
              end_index = flash.readWord(INDEX_NUM_ADDR);
              // Serial.println(end_index);
              times[0] = millis();
              times[1] = millis();
            }
          }
        } else if (digitalRead(KEY3) == 0) {
          while (digitalRead(KEY3) == 0) {
            analogWrite(jdq, map(analogRead(METER), 0, 1023, 0, 255));
            analogWrite(RELAY_LED, map(analogRead(METER), 0, 1023, 0, 255));
          }
        } else {
          key = 0;
        }
        break;
      }

    // In State 1: "Learning" state
    case 1:
      {
        // "Learning" state: Recording relay states at regular intervals
        // Exit to state 0 when KEY1 pressed or upon reaching the MAX_INDEX
        if (millis() >= times[0])  // Learning
        {
          times[0] += 10;
          if (digitalRead(KEY3) == 0) {
            // Serial.println("key3");
            pageBuffer[order] =
              map(analogRead(METER), 0, 1023, 0, 255);  // analogRead(A1) >> 2;
            // Serial.println(pageBuffer[order]);
            analogWrite(jdq, (int)pageBuffer[order]);
            analogWrite(RELAY_LED, (int)pageBuffer[order]);
          } else {
            pageBuffer[order] = 0;
            analogWrite(jdq, 0);
            analogWrite(RELAY_LED, 0);
          }
          order++;
          if (order >= SPI_PAGESIZE) {
            flash.writeByteArray(index, &pageBuffer[0], SPI_PAGESIZE);
            // Serial.print(timeArray[index]);
            order = 0;
            index += SPI_PAGESIZE;
          }
        }
        if (millis() >= times[1]) {
          digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
          times[1] += 100;
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
              for (int i = order; i < SPI_PAGESIZE; i++) {
                pageBuffer[i] = 0;
              }
              flash.writeByteArray(index, &pageBuffer[0], SPI_PAGESIZE);
              index += SPI_PAGESIZE;
            }
            // Serial.println(index);
            flash.writeWord(INDEX_NUM_ADDR, index);
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
        break;
      }

    // In State 2: "Trigger" state
    case 2:
      {
        // "Trigger" state: Replay recorded relay states at regular intervals
        // Exit to state 0 when KEY1 pressed or upon reaching the end of the list
        if (millis() >= times[0])  // Trigger
        {
          times[0] += 10;
          if (order == 0) {
            flash.readByteArray(index, &data_buffer[0], SPI_PAGESIZE);
          }
          analogWrite(RELAY_LED, (int)data_buffer[order]);
          analogWrite(jdq, (int)data_buffer[order]);
          // Serial.println(data_buffer[order]);
          order++;
          if (order >= SPI_PAGESIZE) {
            order = 0;
            index += SPI_PAGESIZE;
          }
        }
        if (millis() >= times[1]) {
          digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
          times[1] += 100;
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
      break;
  }
}