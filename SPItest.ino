#include <SPIMemory.h>

#define MAX_LEN 1000
#define SPI_PAGESIZE 256

SPIFlash flash;
uint8_t pageBuffer[SPI_PAGESIZE];
uint8_t data_buffer[SPI_PAGESIZE];

bool flag = true;
unsigned long times[2] = {0, 0};  // Array to record running times
uint8_t value = 0;
uint16_t index = 0;
uint16_t index_run = 0;
int order = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Initialising"));
  pinMode(A0, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  digitalWrite(5, 0);
  pinMode(3, OUTPUT);
  // digitalWrite(4, 0);
  flash.begin();

  for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
    pageBuffer[i] = 0;
  }
  for (uint16_t i = 0; i < SPI_PAGESIZE; ++i) {
    data_buffer[i] = 0;
  }
  flash.eraseChip();
  times[0] = millis();
            times[1] = millis();
}



void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println(F("Start"));
  if (index < MAX_LEN) {
    if (millis() >= times[0])  // Learning
    {
      times[0] += 10;
      if (digitalRead(A0) == 0) {
        // Serial.println("key3");
        pageBuffer[order] =
            map(analogRead(A1), 0, 1023, 0, 255);  // analogRead(A1) >> 2;
        Serial.println(pageBuffer[order]);
        analogWrite(3, (int)pageBuffer[order]);
      } else {
        pageBuffer[order] = 0;
        analogWrite(3, 0);
      }
      order++;
      if (order >= SPI_PAGESIZE) {
        // EEPROM.write(index, value);
        // timeArray[index] = value;
        flash.writeByteArray(index, &pageBuffer[0], SPI_PAGESIZE);
        // Serial.print(timeArray[index]);
        order = 0;
        index += SPI_PAGESIZE;
        // writeIntIntoEEPROM(EEPROM_SIZE - 3, index);
      }
    }
  }

  if (index >= MAX_LEN && index_run < MAX_LEN) {
    if (millis() >= times[0])  // Trigger
    {
      times[0] += 10;
      if (order == 0) {
        flash.readByteArray(index_run, &data_buffer[0], SPI_PAGESIZE);
      }
      analogWrite(3, (int)data_buffer[order]);
      Serial.println(data_buffer[order]);
      order++;
      if (order >= SPI_PAGESIZE) {
        order = 0;
        index_run += SPI_PAGESIZE;
      }
    }
  }

  //   while (flag) {
  //     flash.eraseChip();
  //     unsigned long write_start_time = millis();
  //     for (unsigned int i = 0; i < MAX_LEN; i += SPI_PAGESIZE) {
  //       if (flash.writeByteArray(i, &pageBuffer[0], SPI_PAGESIZE)) {
  //         Serial.println(i);
  //       }
  //     }
  //     unsigned long write_end_time = millis();
  //     unsigned long read_start_time = millis();
  //     for (unsigned int i = 0; i < MAX_LEN; i += SPI_PAGESIZE) {
  //       //   flash.readStr(i, outputString);
  //       //   Serial.println(outputString);
  //       Serial.println(i);
  //       uint8_t data_buffer[SPI_PAGESIZE];
  //       flash.readByteArray(i, &data_buffer[0], SPI_PAGESIZE);
  //     }
  //     unsigned long read_end_time = millis();
  //     Serial.print("Write time: ");
  //     Serial.println(write_end_time - write_start_time);
  //     Serial.print("Read time: ");
  //     Serial.println(read_end_time - read_start_time);

  //     flag = !flag;
  //   }
}