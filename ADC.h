#include "HardwareSerial.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;
volatile bool mpuFlag = false;  

float lastYaw = 0.0;
int fullRotations = 0;

void dmpReady() 
{
  mpuFlag = true;
}

struct SensorReadings 
{
  uint8_t C;
  uint8_t L;
  uint8_t R;
};

SensorReadings d;
GKalman filterC(600, 250, 0.6); 
GKalman filterL(600, 250, 0.6); 
GKalman filterR(600, 250, 0.6); 


uint8_t fifoBuffer[45];    

void setupADC() {
  ADCSRA = 0;                            // clear ADCSRA register
  ADCSRB = 0;                            // clear ADCSRB register
  ADMUX |= (1 << REFS1) | (1 << REFS0);  // set internal reference voltage
  ADMUX |= (1 << ADLAR);                 // left align ADC value to 8 bits from ADCH register

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles
  ADCSRA |= (1 << ADPS2);  // 16 prescaler for 76.9 KHz

  ADCSRA |= (1 << ADEN);  // enable ADC
}

uint8_t readADC(uint8_t numChannel) 
{
  // Select the ADC input channel
  ADMUX = (ADMUX & 0xF8) | (numChannel & 0x07);

  // Start a new conversion
  ADCSRA |= (1 << ADSC);

  // Wait for the conversion to complete
  while (ADCSRA & (1 << ADSC))    ;

  // Return the ADC value
  return ADCH;
}

SensorReadings getAverageReadings() 
{
  uint16_t SL = 0;
  uint16_t SC = 0;
  uint16_t SR = 0;

  SensorReadings d;

    SL = readADC(1);
    SC = readADC(0);
    SR = readADC(2);

  d.C =  filterC.filtered(SL);
  d.L =  filterL.filtered(SC);
  d.R =  filterR.filtered(SR); 

  return d;
}

float correctedYaw(float yaw) {
  if (yaw - lastYaw > 180.0) {
    // Оберт проти годинникової стрілки
    fullRotations--;
  } else if (lastYaw - yaw > 180.0) {
    // Оберт за годинниковою стрілкою
    fullRotations++;
  }
  lastYaw = yaw;
  return yaw + (fullRotations * 360.0);
}

float measureYaw() {
  Quaternion q; 
  VectorFloat gravity;
  float ypr[3];

  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  
  return degrees(ypr[0]); 
}

float getYaw(bool reset)
{ 
  static float offsetZ = 0.0;

  if(reset)
  {
    float val = 0;
    
    while(1)
      if (mpuFlag && mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) 
      {
        
        val =  measureYaw();
        Serial.print(":");
        
        mpuFlag = false;
        if (abs( val- offsetZ ) <= 1.0e-6 )
          break;
        offsetZ = val; 
    
      }
      Serial.println();
    Serial.print(" offsetZ="); Serial.println(offsetZ);

    return offsetZ;
  }
  else

  while(1)
  if (mpuFlag && mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) 
  {
    float yaw = correctedYaw(measureYaw() - offsetZ) ;
   // Serial.println(yaw);
    mpuFlag = false;
    return yaw;
  }
}

void setupMPU()
{
  Wire.begin();
  mpu.initialize();
  mpu.dmpInitialize();
  mpu.setDMPEnabled(true);

  attachInterrupt(1, dmpReady, RISING);

  Serial.print("Calibration start " );
  mpu.CalibrateAccel();
  mpu.CalibrateGyro();

  mpu.setXAccelOffset(-4243);
  mpu.setYAccelOffset(-630);
  mpu.setZAccelOffset(1732);
  mpu.setXGyroOffset(88);
  mpu.setYGyroOffset(-19);
  mpu.setZGyroOffset(-39);
  
  delay(1000);
  getYaw(true);
  
  Serial.println(" Calibration stop" );
}

