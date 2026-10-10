#include "motor.h"


void TestSensors() 
{
  while (1) {

    uint32_t sumL = 0;
    uint32_t sumC = 0;
    uint32_t sumR = 0;

    SensorReadings readings = { 0, 0, 0 };
 
      readings = getAverageReadings();
      sumL = readings.L;
      sumC = readings.C;
      sumR = readings.R;
 

    Serial.print("L: ");
    Serial.print(sumL);
    Serial.print(", C: ");
    Serial.print(sumC);
    Serial.print(", R: ");
    Serial.println(sumR);
    delay(1);
  }
}

void TestSensors2() 
{
  while (1) {

    uint32_t sumL = 0;
    uint32_t sumC = 0;
    uint32_t sumR = 0;
    const int K = 2;//8196 * 2;
    SensorReadings readings = { 0, 0, 0 };
    for(uint32_t i=0; i < K; i++)
    {
      readings = getAverageReadings();
      sumL += readings.L;
      sumC += readings.C;
      sumR += readings.R;
    }
    sumL/=K; sumC/=K; sumR/=K;// SumL=SumL/K;
    // sumL=sumL>>3 == sumL/8

    Serial.print("L: ");
    Serial.print(sumL);
    Serial.print(", C: ");
    Serial.print(sumC);
    Serial.print(", R: ");
    Serial.println(sumR);
    delay(500);
  }
}


void testTurnToAngle() 
{
  const int angles[] = {90, 0, -90, 0, 90, 0, -90, 0, 45, 0, -45, 0};
  const int testCount = sizeof(angles) / sizeof(angles[0]);
  Serial.println("Testing start!");
  getYaw(true);
  for (int i = 0; i < testCount; i++) 
  {
    int angle = angles[i];
    Serial.print("Testing turn to angle: ");
    Serial.print(angle);
    Serial.println(" degrees...");

    turnToAngle(angle);

    delay(3000);
  }
  Serial.println("Testing completed!");
}
