#include <Arduino.h>

// Define component pin assignments
#define RECODRD_LED 8  // IO18
#define TIMER_LED 6    // IO6
#define NORMAL_LED 7   // IO7
#define RELAY_LED 9    // IO19

// #define RECODRD_LED_CHANNEL 0
// #define TIMER_LED_CHANNEL 1
// #define NORMAL_LED_CHANNEL 2
// #define RELAY_LED_CHANNEL 3
// #define MOS_CHANNEL 4

#define KEY1 1   // IO1
#define KEY2 2    // IO2
#define KEY3 3    // IO3
#define METER 4  // IO4
#define POWER 5  // IO5
#define MOS 10    // IO10  // Relay control pin
#define INDEX_NUM_ADDR 65536
#define MAX_RECORD 64000
#define SPI_PAGESIZE 256
#define SPI_SECTORSIZE 4096

const uint32_t NVM_Offset = 0x290000;

uint8_t pageBuffer[SPI_SECTORSIZE];
uint8_t data_buffer[SPI_SECTORSIZE];

unsigned long times[2] = {0, 0};  // Store timing data for operations
unsigned long button_time_start = 0;
unsigned long button_time_end = 0;

// State variable declarations
uint8_t value = 0;
uint16_t idx = 0;
int order = 0;

int state = 0;
uint32_t record_size = 0;
int key = 0;

void setup() {
  // Initialize serial communication, configure pins and set initial LED states
  Serial.begin(9600);

  pinMode(MOS, OUTPUT);
  digitalWrite(MOS, 0);
  pinMode(RECODRD_LED, OUTPUT);
  pinMode(TIMER_LED, OUTPUT);
  pinMode(NORMAL_LED, OUTPUT);
  pinMode(RELAY_LED, OUTPUT);
  pinMode(KEY1, INPUT_PULLUP);
  pinMode(KEY2, INPUT_PULLUP);
  pinMode(KEY3, INPUT_PULLUP);

  // ledcSetup(RECODRD_LED_CHANNEL, 10000, 8);
  // ledcSetup(TIMER_LED_CHANNEL, 10000, 8);
  // ledcSetup(NORMAL_LED_CHANNEL, 10000, 8);
  // ledcSetup(RECODRD_LED_CHANNEL, 10000, 8);
  // ledcSetup(MOS_CHANNEL, 10000, 8);
  // ledcAttachPin(RECODRD_LED, 0);
  // ledcAttachPin(TIMER_LED, 1);
  // ledcAttachPin(NORMAL_LED, 2);
  // ledcAttachPin(RELAY_LED, 3);
  // ledcAttachPin(MOS, 4);

  for (int i = 0; i <= 255; i++) {
    analogWrite(RECODRD_LED, i);
    analogWrite(TIMER_LED, i);
    analogWrite(NORMAL_LED, i);
    analogWrite(RELAY_LED, i);
    delay(2);
  }

  // Start SoftPWM and configure LED fade and brightness
  for (uint16_t i = 0; i < SPI_SECTORSIZE; ++i) {
    pageBuffer[i] = 0;
  }
  for (uint16_t i = 0; i < SPI_SECTORSIZE; ++i) {
    data_buffer[i] = 0;
  }
  // delay(500);

  // Reset LEDs to off
  for (int i = 255; i >= 0; i--) {
    analogWrite(RECODRD_LED, i);
    analogWrite(TIMER_LED, i);
    analogWrite(NORMAL_LED, i);
    analogWrite(RELAY_LED, i);
    delay(2);
  }

  // int fadeDelay = 2000 / 256;
  // // Serial.println(analogRead(POWER));
  // if (analogRead(POWER) < 500) {
  //   for (int i = 0; i < 3; i++) {
  //     // Repeat the fade up and fade down three times
  //     for (int brightness = 0; brightness <= 255; brightness++) {  // Fade up
  //       analogWrite(0, brightness);
  //       delay(fadeDelay);
  //     }
  //     for (int brightness = 255; brightness >= 0; brightness--) {  // Fade down
  //       analogWrite(0, brightness);
  //       delay(fadeDelay);
  //     }
  //   }
  // }

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
    case 0: {
      // Normal LED on, others off
      // Relay off

      // Default state: NORMAL LED on, others off, relay off
      // KEY1 for 5 seconds: Clear flash memory and transition to state 1
      // KEY2: Transition to state 2
      // KEY3: Manual relay control
      digitalWrite(MOS, 0);
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

          for (uint32_t i = 0; i <= INDEX_NUM_ADDR; i +=  SPI_SECTORSIZE) {
          ESP.flashEraseSector((NVM_Offset+i)/SPI_SECTORSIZE);
          }
          // flash.writeWord(INDEX_NUM_ADDR, 0);
          state = 1, idx = 0, order = 0, value = 0, record_size = 0;
          for (uint16_t i = 0; i < SPI_SECTORSIZE; ++i) {
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
          if (state == 0) {
            state = 2, order = 0, idx = 0, value = 0;
            // record_size = flash.readWord(INDEX_NUM_ADDR);
            ESP.flashRead(NVM_Offset+INDEX_NUM_ADDR, (uint32_t*)&record_size, sizeof(record_size));
            times[0] = millis();
            times[1] = millis();
          }
        }
      } else if (digitalRead(KEY3) == 0) {
        while (digitalRead(KEY3) == 0) {
          // analogWrite(MOS, map(analogRead(METER), 0, 1023, 0, 255));
          // analogWrite(RELAY_LED, map(analogRead(METER), 0, 1023, 0, 255));
          analogWrite(RECODRD_LED,
                    map(analogRead(METER), 0, 1023, 0, 255));
          analogWrite(MOS, map(analogRead(METER), 0, 1023, 0, 255));
        }
      } else {
        key = 0;
      }
      break;
    }

    // In State 1: "Learning" state
    case 1: {
      // "Learning" state: Recording relay states at regular intervals
      // Exit to state 0 when KEY1 pressed or upon reaching the MAX_RECORD
      if (millis() >= times[0])  // Learning
      {
        times[0] += 10;
        if (digitalRead(KEY3) == 0) {
          pageBuffer[order] =
              map(analogRead(METER), 0, 1023, 0, 255);  // analogRead(A1) >> 2;
          // analogWrite(MOS, (int)pageBuffer[order]);
          // analogWrite(RELAY_LED, (int)pageBuffer[order]);
          analogWrite(RELAY_LED, (int)pageBuffer[order]);
          analogWrite(MOS, (int)pageBuffer[order]);
        } else {
          pageBuffer[order] = 0;
          // analogWrite(MOS, 0);
          // analogWrite(RELAY_LED, 0);
          analogWrite(RELAY_LED, 0);
          analogWrite(MOS, 0);
        }
        order++;
        if (order >= SPI_SECTORSIZE) {
          ESP.flashWrite(NVM_Offset+(uint32_t)idx, (uint32_t*)&pageBuffer[0], SPI_SECTORSIZE);
          order = 0;
          idx += SPI_SECTORSIZE;
        }
      }
      if (millis() >= times[1]) {
        digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
        times[1] += 100;
      }
      if (digitalRead(KEY1) == 0 || idx >= MAX_RECORD) {
        if (key != 1) {
          button_time_start = millis();
          button_time_end = millis();
          key = 1;
        } else {
          button_time_end = millis();
        }
        if ((button_time_end - button_time_start) >= 10 || idx >= MAX_RECORD) {
          key = 0;
          digitalWrite(TIMER_LED, 1);
          digitalWrite(RELAY_LED, 0);
          if (idx < MAX_RECORD) {
            for (int i = order; i < SPI_SECTORSIZE; i++) {
              pageBuffer[i] = 0;
            }
            ESP.flashWrite(NVM_Offset+(uint32_t)idx, (uint32_t*)&pageBuffer[0], SPI_SECTORSIZE);
            idx += SPI_SECTORSIZE;
          }
          ESP.flashWrite(NVM_Offset+INDEX_NUM_ADDR, (uint32_t*)&idx, sizeof(idx));
          state = 0, order = 0, value = 0, idx = 0;
          record_size = 0;
          while (digitalRead(KEY1) == 0)
            ;
        }
      } else {
        key = 0;
      }
      break;
    }

    // In State 2: "Trigger" state
    case 2: {
      // "Trigger" state: Replay recorded relay states at regular intervals
      // Exit to state 0 when KEY1 pressed or upon reaching the end of the list
      if (millis() >= times[0])  // Trigger
      {
        times[0] += 10;
        if (order == 0) {
          ESP.flashRead(NVM_Offset+idx, (uint32_t*)&data_buffer[0], SPI_SECTORSIZE);
        }
        // analogWrite(RELAY_LED, (int)data_buffer[order]);
        // analogWrite(MOS, (int)data_buffer[order]);
        analogWrite(RELAY_LED, (int)pageBuffer[order]);
        analogWrite(MOS, (int)pageBuffer[order]);
        order++;
        if (order >= SPI_SECTORSIZE) {
          order = 0;
          idx += SPI_SECTORSIZE;
        }
      }
      if (millis() >= times[1]) {
        digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
        times[1] += 100;
      }
      if (digitalRead(KEY1) == 0 || idx >= record_size) {
        if (key != 1) {
          button_time_start = millis();
          button_time_end = millis();
          key = 1;
        } else {
          button_time_end = millis();
        }
        if ((button_time_end - button_time_start) >= 10 || idx >= record_size) {
          key = 0;
          state = 0, order = 0, value = 0, idx = 0;
          record_size = 0;
          digitalWrite(TIMER_LED, 1);
          digitalWrite(RELAY_LED, 0);
          digitalWrite(MOS, 0);
          while (digitalRead(KEY1) == 0)
            ;
        }
      } else {
        key = 0;
      }
    } break;
  }
}