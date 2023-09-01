#include <SPIMemory.h>

// Define component pin assignments
#define RECODRD_LED PA9
#define TIMER_LED PA11
#define NORMAL_LED PA10
#define RELAY_LED PA8
#define KEY1 PA1
#define KEY2 PA2
#define KEY3 PA3
#define METER PB0
#define POWER PB1
#define MOS PB10  // Relay control pin
#define idx_NUM_ADDR 65536
#define MAX_idx 64000
#define SPI_PAGESIZE 256

SPIFlash flash(PA4);
uint8_t pageBuffer[SPI_PAGESIZE];
uint8_t data_buffer[SPI_PAGESIZE];

unsigned long times[2] = {0, 0};  // Store timing data for operations
unsigned long button_time_start = 0;
unsigned long button_time_end = 0;

// State variable declarations
uint8_t value = 0;
uint16_t idx = 0;
int order = 0;

int state = 0;
uint16_t max_record_num = 0;
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

  // SoftPWMBegin();
  // SoftPWMSet(RECODRD_LED, 0);
  // SoftPWMSet(TIMER_LED, 0);
  // SoftPWMSet(NORMAL_LED, 0);
  // SoftPWMSet(RELAY_LED, 0);
  // SoftPWMSetFadeTime(RECODRD_LED, 500, 500);
  // SoftPWMSetFadeTime(TIMER_LED, 500, 500);
  // SoftPWMSetFadeTime(NORMAL_LED, 500, 500);
  // SoftPWMSetFadeTime(RELAY_LED, 500, 500);
  // SoftPWMSetPercent(RECODRD_LED, 50);
  // SoftPWMSetPercent(TIMER_LED, 50);
  // SoftPWMSetPercent(NORMAL_LED, 50);
  // SoftPWMSetPercent(RELAY_LED, 50);

  // Start SoftPWM and configure LED fade and brightness
  flash.begin();
  for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
    pageBuffer[i] = 0;
  }
  for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
    data_buffer[i] = 0;
  }
  delay(500);

  // // Reset LEDs to off
  // SoftPWMSetPercent(RECODRD_LED, 0);
  // SoftPWMSetPercent(TIMER_LED, 0);
  // SoftPWMSetPercent(NORMAL_LED, 0);
  // SoftPWMSetPercent(RELAY_LED, 0);
  // delay(500);
  // SoftPWMEnd(RECODRD_LED);
  // SoftPWMEnd(TIMER_LED);
  // SoftPWMEnd(NORMAL_LED);
  // SoftPWMEnd(RELAY_LED);

  Serial.println(analogRead(POWER));
  // if (analogRead(POWER) < 500) {
  //   SoftPWMSet(POWER_LED, 0);
  //   SoftPWMSetFadeTime(POWER_LED, 500, 500);
  //   SoftPWMSetPercent(POWER_LED, 50);
  //   delay(250);
  //   SoftPWMSetPercent(POWER_LED, 0);
  //   delay(250);
  //   SoftPWMSetPercent(POWER_LED, 50);
  //   delay(250);
  //   SoftPWMSetPercent(POWER_LED, 0);
  //   delay(250);
  //   SoftPWMSetPercent(POWER_LED, 50);
  //   delay(250);
  //   SoftPWMSetPercent(POWER_LED, 0);
  //   delay(250);
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

          flash.eraseChip();
          // flash.writeWord(idx_NUM_ADDR, 0);
          state = 1, idx = 0, order = 0, value = 0, max_record_num = 0;
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
          if (state == 0) {
            state = 2, order = 0, idx = 0, value = 0;
            max_record_num = flash.readWord(idx_NUM_ADDR);
            times[0] = millis();
            times[1] = millis();
          }
        }
      } else if (digitalRead(KEY3) == 0) {
        while (digitalRead(KEY3) == 0) {
          analogWrite(MOS, map(analogRead(METER), 0, 1023, 0, 255));
          analogWrite(RELAY_LED, map(analogRead(METER), 0, 1023, 0, 255));
        }
      } else {
        key = 0;
      }
      break;
    }

    // In State 1: "Learning" state
    case 1: {
      // "Learning" state: Recording relay states at regular intervals
      // Exit to state 0 when KEY1 pressed or upon reaching the MAX_idx
      if (millis() >= times[0])  // Learning
      {
        times[0] += 10;
        if (digitalRead(KEY3) == 0) {
          pageBuffer[order] =
              map(analogRead(METER), 0, 1023, 0, 255);  // analogRead(A1) >> 2;
          analogWrite(MOS, (int)pageBuffer[order]);
          analogWrite(RELAY_LED, (int)pageBuffer[order]);
        } else {
          pageBuffer[order] = 0;
          analogWrite(MOS, 0);
          analogWrite(RELAY_LED, 0);
        }
        order++;
        if (order >= SPI_PAGESIZE) {
          flash.writeByteArray(idx, &pageBuffer[0], SPI_PAGESIZE);
          order = 0;
          idx += SPI_PAGESIZE;
        }
      }
      if (millis() >= times[1]) {
        digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
        times[1] += 100;
      }
      if (digitalRead(KEY1) == 0 || idx >= MAX_idx) {
        if (key != 1) {
          button_time_start = millis();
          button_time_end = millis();
          key = 1;
        } else {
          button_time_end = millis();
        }
        if ((button_time_end - button_time_start) >= 10 || idx >= MAX_idx) {
          key = 0;
          digitalWrite(TIMER_LED, 1);
          digitalWrite(RELAY_LED, 0);
          if (idx < MAX_idx) {
            for (int i = order; i < SPI_PAGESIZE; i++) {
              pageBuffer[i] = 0;
            }
            flash.writeByteArray(idx, &pageBuffer[0], SPI_PAGESIZE);
            idx += SPI_PAGESIZE;
          }
          flash.writeWord(idx_NUM_ADDR, idx);
          state = 0, order = 0, value = 0, idx = 0;
          max_record_num = 0;
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
          flash.readByteArray(idx, &data_buffer[0], SPI_PAGESIZE);
        }
        analogWrite(RELAY_LED, (int)data_buffer[order]);
        analogWrite(MOS, (int)data_buffer[order]);
        order++;
        if (order >= SPI_PAGESIZE) {
          order = 0;
          idx += SPI_PAGESIZE;
        }
      }
      if (millis() >= times[1]) {
        digitalWrite(TIMER_LED, !digitalRead(TIMER_LED));
        times[1] += 100;
      }
      if (digitalRead(KEY1) == 0 || idx >= max_record_num) {
        if (key != 1) {
          button_time_start = millis();
          button_time_end = millis();
          key = 1;
        } else {
          button_time_end = millis();
        }
        if ((button_time_end - button_time_start) >= 10 || idx >= max_record_num) {
          key = 0;
          state = 0, order = 0, value = 0, idx = 0;
          max_record_num = 0;
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