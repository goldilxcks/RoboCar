#include <stdint.h>
#include "ADC.h"
void Motor(int Dir, int Speed_LEFT, int Speed_RIGHT) 
{
 if (Speed_LEFT != 0 || Speed_RIGHT != 0) 
  {
    digitalWrite(EN_PIN, LOW);
    analogWrite(PWM1_PIN, 255);
    analogWrite(PWM2_PIN, 255);

    digitalWrite(STCP_PIN, LOW);
    shiftOut(DATA_PIN, SHCP_PIN, MSBFIRST, Dir);
    digitalWrite(STCP_PIN, HIGH);
 
    delay(30);
 }

  digitalWrite(EN_PIN, LOW);
  analogWrite(PWM1_PIN, Speed_RIGHT);
  analogWrite(PWM2_PIN, Speed_LEFT);

  digitalWrite(STCP_PIN, LOW);
  shiftOut(DATA_PIN, SHCP_PIN, MSBFIRST, Dir);
  digitalWrite(STCP_PIN, HIGH);
}

void turnToAngle(float targetAngle) 
{
  const float errorTolerance = 0.7; // допустима похибка в градусах
 
  float currentYaw = getYaw(false);
  float initialError = abs(targetAngle - currentYaw);

  while (abs(targetAngle - currentYaw) > errorTolerance) 
  {
    float error = targetAngle - currentYaw;
    float errorPercentage = abs(error) / initialError;

    Serial.print("Error="); Serial.print(error);
    // Визначаємо напрямок повороту
    uint8_t direction = (error < 0) ? Clockwise : ContraClockwise;

    // Визначаємо швидкість обертання з урахуванням відсотка від кута повороту
    uint8_t speed;
    if (errorPercentage > 0.7) 
    {
      speed = 255;
    } 
    else if (errorPercentage > 0.4) 
    {
      speed = speedM;
    } 
    else 
    {
      speed = 50;
    }

    Serial.print(" Speed="); Serial.print(speed);
    // Виконуємо поворот
    Motor(direction, speed, speed);

    // Оновлюємо поточний кут
    currentYaw = getYaw(false);
    Serial.print(" Yaw=");  Serial.println(currentYaw); 
  }

  // Зупиняємо робота
  Motor(Clockwise, 0, 0);
}

uint8_t RXpack_func(uint8_t m_command )  //Receive data
{
  byte RX_package[3] = {0};
  
    if(Serial.available() > 0)
    {
        delay(1);                                           // delay 1MS
        if(Serial.readBytes(RX_package, 3))
        {
            if (RX_package[0] == 0xA5 && RX_package[2] == 0x5A)     // The header and tail of the packet are verified
            {
                m_command = RX_package[1];
            }
        }
    }
    return m_command;
}

void clearSerialBuffer() {
    while(Serial.available() > 0) {
        Serial.read(); // Читаємо та ігноруємо дані в буфері
    }
}
