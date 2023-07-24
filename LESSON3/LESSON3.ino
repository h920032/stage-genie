

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

void setup()
{
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
        for (int i = 0; i < 512; i++) {
          EEPROM.write(i, 0);
        }
        xiangmu = 1;
        index = 0;
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
      EEPROM.write(index, 1);
      digitalWrite(jdq, 0);
      digitalWrite(LEDB, 1);
    }
    else
    {
      EEPROM.write(index, 2);
      digitalWrite(jdq, 1);
      digitalWrite(LEDB, 0);
    }
    index++;
  }
  if (digitalRead(KEY2) == 0)
  {
    delay(10);
    if (digitalRead(KEY2) == 0)
    {
      Serial.println("key2");
      if (xiangmu == 0)
      {
        xiangmu = 2;
        index = 0;
      }
    }
  }
  if (xiangmu == 2) //启动
  {
    digitalWrite(LEDG,!digitalRead(LEDG));
    if (EEPROM.read(index) == 2)
    {
      digitalWrite(jdq, 1);
      digitalWrite(LEDB, 0);
    }
    else if (EEPROM.read(index) == 1)
    {
      digitalWrite(jdq, 0);
      digitalWrite(LEDB, 1);
    }
    else if (EEPROM.read(index) == 0)
    {
      xiangmu = 0;
      digitalWrite(LEDR, 0);
      digitalWrite(LEDG, 1);
      digitalWrite(LEDB, 0);
    }
    index ++;
  }
  if (digitalRead(KEY4) == 0)
  {
    delay(10);
    if (digitalRead(KEY4) == 0)
    {
      Serial.println("key4");
    }
  }
  delay(1000);
}
