#include <EEPROM.h>
#define LEDR 8
#define LEDG 6
#define LEDB 4
#define KEY1 A2
#define KEY2 A1
#define KEY3 A4
#define KEY4 A3
#define jdq 11
int KEY_NUM = 0;			//按键键值存放变量，不等于1说明有按键按下

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
long timer1 = 0;
int xiangmu = 0;
int index = 0;
int order = 0;
int value = 0;

void loop()
{
  if (digitalRead(KEY1) == 0)
  {
    delay(10);
    if (digitalRead(KEY1) == 0)
    {
      Serial.println("key1");
      if (xiangmu == 0)
      {
        for (int i = 0; i < 1024; i++) {
          EEPROM.write(i, 0);
        }
        xiangmu = 1;
        index = 0;
        order = 0;
        digitalWrite(LEDR, 0);
        digitalWrite(LEDG, 0);
        digitalWrite(LEDB, 0);
      }
      else
      {
        xiangmu = 0;
        digitalWrite(LEDR, 1);
        delay(3000);
        digitalWrite(LEDR, 0);
        digitalWrite(LEDG, 1);
        digitalWrite(LEDB, 0);
      }
    }
    while (digitalRead(KEY1) == 0);
  }
  if (xiangmu == 1) //学习
  {
    digitalWrite(LEDR, !digitalRead(LEDR));
    if (digitalRead(KEY3) == 0)
    {
      Serial.println("key3");
      value = value ^ (1 << (7 - order));
      Serial.print(value);
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
  if (digitalRead(KEY2) == 0 || index >= 1024)
  {
    delay(10);
    if (digitalRead(KEY2) == 0 || index >= 1024)
    {
      Serial.println("key2");
      if (xiangmu == 0)
      {
        xiangmu = 2;
        EEPROM.write(index, value);
        order = 0;
        value = 0;
        index = 0;
      }
    }
  }
  if (xiangmu == 2) //启动
  {
    digitalWrite(LEDG, !digitalRead(LEDG));
    if ((EEPROM.read(index) >> (7 - order)) & 1) {
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
      xiangmu = 0;
      digitalWrite(LEDR, 0);
      digitalWrite(LEDG, 1);
      digitalWrite(LEDB, 0);
    }
  }
  if (digitalRead(KEY4) == 0)
  {
    delay(10);
    if (digitalRead(KEY4) == 0) {
      Serial.println("key4");
      for (int i = 0; i < 1024; i++) {
        Serial.print(EEPROM.read(i));
      }
    }
  }
  delay(100);
}
