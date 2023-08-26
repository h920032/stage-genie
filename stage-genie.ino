#include <SPIMemory.h>

// Define component pin assignments
#define RECODRD_LED 26  // IO18
#define TIMER_LED 20    // IO6
#define NORMAL_LED 21   // IO7
#define RELAY_LED 27    // IO19

#define RECODRD_LED_CHANNEL 0
#define TIMER_LED_CHANNEL 1
#define NORMAL_LED_CHANNEL 2
#define RELAY_LED_CHANNEL 3
#define MOS_CHANNEL 4

#define KEY1 13   // IO1
#define KEY2 5    // IO2
#define KEY3 6    // IO3
#define METER 18  // IO4
#define POWER 19  // IO5
#define MOS 16    // IO10  // Relay control pin
#define INDEX_NUM_ADDR 65536
#define MAX_RECORD 64000
#define SPI_PAGESIZE 256

SPIFlash flash;
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
uint16_t record_size = 0;
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

  ledcSetup(RECODRD_LED_CHANNEL, 10000, 8);
  ledcSetup(TIMER_LED_CHANNEL, 10000, 8);
  ledcSetup(NORMAL_LED_CHANNEL, 10000, 8);
  ledcSetup(RECODRD_LED_CHANNEL, 10000, 8);
  ledcSetup(MOS_CHANNEL, 10000, 8);
  ledcAttachPin(RECODRD_LED, 0);
  ledcAttachPin(TIMER_LED, 1);
  ledcAttachPin(NORMAL_LED, 2);
  ledcAttachPin(RELAY_LED, 3);
  ledcAttachPin(MOS, 4);

  for (int i = 0; i <= 255; i++) {
    ledcWrite(RECODRD_LED_CHANNEL, i);
    ledcWrite(TIMER_LED_CHANNEL, i);
    ledcWrite(NORMAL_LED_CHANNEL, i);
    ledcWrite(RELAY_LED_CHANNEL, i);
    delay(20);
  }

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
  for (int i = 255; i >= 0; i--) {
    ledcWrite(RECODRD_LED_CHANNEL, i);
    ledcWrite(TIMER_LED_CHANNEL, i);
    ledcWrite(NORMAL_LED_CHANNEL, i);
    ledcWrite(RELAY_LED_CHANNEL, i);
    delay(20);
  }

  int fadeDelay = 2000 / 256;
  // Serial.println(analogRead(POWER));
  if (analogRead(POWER) < 500) {
    for (int i = 0; i < 3; i++) {
      // Repeat the fade up and fade down three times
      for (int brightness = 0; brightness <= 255; brightness++) {  // Fade up
        ledcWrite(0, brightness);
        delay(fadeDelay);
      }
      for (int brightness = 255; brightness >= 0; brightness--) {  // Fade down
        ledcWrite(0, brightness);
        delay(fadeDelay);
      }
    }
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
          // flash.writeWord(INDEX_NUM_ADDR, 0);
          state = 1, idx = 0, order = 0, value = 0, record_size = 0;
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
            record_size = flash.readWord(INDEX_NUM_ADDR);
            times[0] = millis();
            times[1] = millis();
          }
        }
      } else if (digitalRead(KEY3) == 0) {
        while (digitalRead(KEY3) == 0) {
          // analogWrite(MOS, map(analogRead(METER), 0, 1023, 0, 255));
          // analogWrite(RELAY_LED, map(analogRead(METER), 0, 1023, 0, 255));
          ledcWrite(RECODRD_LED_CHANNEL,
                    map(analogRead(METER), 0, 1023, 0, 255));
          ledcWrite(MOS_CHANNEL, map(analogRead(METER), 0, 1023, 0, 255));
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
          ledcWrite(RELAY_LED_CHANNEL, (int)pageBuffer[order]);
          ledcWrite(MOS_CHANNEL, (int)pageBuffer[order]);
        } else {
          pageBuffer[order] = 0;
          // analogWrite(MOS, 0);
          // analogWrite(RELAY_LED, 0);
          ledcWrite(RELAY_LED_CHANNEL, 0);
          ledcWrite(MOS_CHANNEL, 0);
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
            for (int i = order; i < SPI_PAGESIZE; i++) {
              pageBuffer[i] = 0;
            }
            flash.writeByteArray(idx, &pageBuffer[0], SPI_PAGESIZE);
            idx += SPI_PAGESIZE;
          }
          flash.writeWord(INDEX_NUM_ADDR, idx);
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
          flash.readByteArray(idx, &data_buffer[0], SPI_PAGESIZE);
        }
        // analogWrite(RELAY_LED, (int)data_buffer[order]);
        // analogWrite(MOS, (int)data_buffer[order]);
        ledcWrite(RELAY_LED_CHANNEL, (int)pageBuffer[order]);
        ledcWrite(MOS_CHANNEL, (int)pageBuffer[order]);
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