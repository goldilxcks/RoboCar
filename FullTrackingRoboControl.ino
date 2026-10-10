#include  <Servo.h>
#include "GyverFilters.h"
//#include "MPU6050.h"
//#include "Wire.h"

#include "init.h"
#include "tests.h"

void motorBeep() 
{
  Motor(Forward, 50, 50);
  delay(1000);
  Motor(Stop, 0, 0);
}
//=============================================================== S E T U P
void setup() 
{
  pinMode(SHCP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(STCP_PIN, OUTPUT);
  pinMode(PWM1_PIN, OUTPUT);
  pinMode(PWM2_PIN, OUTPUT);
  pinMode(A5, OUTPUT);
  Motor(Stop, 0, 0);
  pinMode(L_PIN, INPUT);
  pinMode(C_PIN, INPUT);
  pinMode(R_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  myservo.attach(servo_PIN);

  Serial.begin(115200);


  setupADC();

   // servoSet(0);
  for(int i=0; i < 20; i++)  { getAverageReadings(); } //Adjust filter
  motorBeep();
  delay(1000);
  Motor(DRIVE_STOP, 0, 0);
  Serial.println("Start system ");
  //setupMPU();
  servoSet(0);
  
  delay(1000);
  servoSet(90);
  //delay(2000);
  tstart = millis();

 // TestSensors();

  //testTurnToAngle();
  //  while(1);

  int DM = 80;
  int DX = 10;
 
  L_min = 72;  L_max = 254;  C_min = 62;  C_max = 254;  R_min = 53;  R_max = 254;  // Kalman

  L_min_bias = L_min + DM;
  L_max_bias = L_max - DX;
  C_min_bias = C_min + DM;
  C_max_bias = C_max - DX;
  R_min_bias = R_min + DM;
  R_max_bias = R_max - DX;
}

/////////////////////////////////////////////////////////////////// L O O P
void loop() 
{
  d = getAverageReadings();
  L = (d.L);
  C = (d.C);
  R = (d.R);

  cnt++;


  if (L < L_min_bias && C > C_max_bias && R < R_min_bias)
  {
    const uint32_t TTF = 20;
    sumForward++;
    if(sumForward > TTF)
    {
      startRotate = 0;
    prev_State = state;
    state = DRIVE_STRAIGHT;
    printState(state);
      sumForward = 0; sumLeft = 0; sumRight = 0; sumBlack = 0; sumWhite = 0;
      
      if( UFcheckDistance('<', 210 )) 
      {
      /*  clearSerialBuffer();
        delay(4000);
        Serial.println(" Direct dx "); 
        auto to_Left_or_Right  = RXpack_func(Forward);
        obstacleAvoidance(to_Left_or_Right);
        */
      }
  
    
  } 
  else 
  
  if (L > L_min_bias && C > C_max_bias && R < R_min_bias )  //SMALL_CORR_LEFT
  {
    const uint32_t TTLsmall = 10;
    sumLeft++;
    if (sumLeft > TTLsmall )
    {
      startRotate = millis();
    prev_State = state;
    state = SMALL_CORR_LEFT;
    printState(state);
     sumForward = 0; sumRight = 0; sumBlack = 0; sumWhite = 0;
    }
  } 
  else

  if (L > L_min_bias && C < C_max_bias && R < R_min_bias )  //CORR_LEFT
  {
    const uint32_t TTL = 20;
    sumLeft++;
    if (sumLeft > TTL )
    {
      prev_State = state;
      state = CORR_LEFT;
      printState(state);
      sumForward = 0; sumRight = 0; sumBlack = 0; sumWhite = 0;
    }
  } 
  else


  if (L < L_min_bias && C > C_max_bias && R > R_min_bias )  //SMALL_CORR_RIGHT
  {
    const uint32_t TTRSmall = 10;
    sumRight++;
    if (sumRight > TTRSmall )
    {
      startRotate = millis();
      prev_State = state;
      state = SMALL_CORR_RIGHT;
      printState(state);
      sumForward = 0; sumLeft = 0; sumBlack = 0; sumWhite = 0;

    }
  } 
  else

  if (L < L_min_bias && C < C_max_bias && R > R_min_bias )  // CORR_RIGHT
  {
   const uint32_t TTR = 20;
    sumRight++;
    if (sumRight > TTR )
    {
      prev_State = state;
      state = CORR_RIGHT;
      printState(state);
      sumForward = 0; sumLeft = 0; sumBlack = 0; sumWhite = 0;
    }
  }
  else

  if (L > L_max_bias && C > C_max_bias && R > R_max_bias)  // Full Black
  {
    const uint32_t TTB = 20;
    sumBlack++;
    if (sumBlack > TTB )
    {
     state = prev_State;
     printState(FULL_BLACK);
     sumForward = 0; sumLeft = 0; sumRight = 0;sumWhite = 0;
    }
  }
  else

  {
    const uint32_t TTW = 500000;
    sumWhite++;
    if(sumWhite < TTW) 
      return;

    printState(FULL_WHITE);
    
     if(startRotate > 0 && (millis() - startRotate) > 300)
     {
     // prev_State = state;
      if (state == SMALL_CORR_RIGHT || prev_State == SMALL_CORR_RIGHT) 
      {
         state = CORR_RIGHT;   printState(CORR_RIGHT);
      }
      if (state == SMALL_CORR_LEFT || prev_State == SMALL_CORR_LEFT) 
      {
        state = CORR_LEFT;   printState(CORR_LEFT);
      }
     }
  }

 if(prev_cmd_state != state)
 {
  prev_cmd_state=state;
  const float distance_a = 0.0013961419118888071;
  const float distance_b = 0.9742926153285707;
  float v = distance_a * pow(speedM, distance_b);
  float dTF = 1000 * (0.075) / v;
    printSum();

    switch (state) {

      case DRIVE_STRAIGHT:    Motor(Forward, speedH, speedH); break;         // SpeedLeft, SpeedRight
      case SMALL_CORR_RIGHT:  Motor(Forward, speedL, 5); break;
      case SMALL_CORR_LEFT:   Motor(Forward, 5, speedL); break;
      case CORR_RIGHT:        Motor(Clockwise, speedL, speedM);  break;
      case CORR_LEFT:         Motor(ContraClockwise, speedM, speedL); break;
      case MOVE_RIGHT:        Motor(Turn_Right, speedL, speedH);  break;
      case MOVE_LEFT:         Motor(Turn_Left, speedH, speedL); break;

      case TURN_RIGHT:
        Motor(Forward, speedM, speedM);
        delay(dTF);
        Motor(Clockwise, speedM, speedH); // SpeedLeft, SpeedRight
        waitStraightLine();
        
        break;

      case TURN_LEFT:
        Motor(Forward, speedM, speedM); //L R
        delay(dTF);
        Motor(ContraClockwise, speedH, speedM); // SpeedLeft, SpeedRight
        waitStraightLine();
     
        break;

      case DRIVE_STOP:
        Motor(Stop, 0, 0);
        delay(300);
        break;
    }
   
   }
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  M O T O R

void servoSet(uint8_t deg) // servo angle correction
{
  myservo.write(deg);
}

void printTime(uint32_t time)
{
  time-= tstart;
 // unsigned uint32_t time = millis(); // Отримання часу у мілісекундах
 uint32_t total_seconds = time / 1000; // Перетворення у секунди

 uint32_t minutes = total_seconds / 60; // Перетворення у хвилини
 uint32_t seconds = total_seconds % 60; // Виділення залишкових секунд
 uint32_t hundredths = (time % 1000) / 10; // Виділення сотих частин секунди

  Serial.print(minutes);  
  Serial.print(":");
  Serial.print(seconds);
  Serial.print(":");
  Serial.print(hundredths);
}

void printState(State s) 
{

  static State prevState = UNKNOWN;
  static int m = 0;
  if (s == prevState) return;


  switch (s) {
    case DRIVE_STRAIGHT:  Serial.print(" FORWARD: "); break;
    case TURN_LEFT:       Serial.print("  TURN_L: "); break;
    case TURN_RIGHT:      Serial.print("  TURN_R: "); break;
    case CORR_LEFT:       Serial.print("  CORR_L: "); break;
    case CORR_RIGHT:      Serial.print("  CORR_R: "); break;
    case SMALL_CORR_LEFT: Serial.print(" SCORR_L: "); break;
    case SMALL_CORR_RIGHT:Serial.print(" SCORR_R: "); break;
    case DRIVE_STOP:      Serial.print("    STOP: "); break;
    case UNKNOWN:         Serial.print(" UNKNOWN: "); break;
    case FULL_WHITE:      Serial.print("   WHITE: "); break;
    case FULL_BLACK:      Serial.print("   BLACK: "); break;
  }
  Serial.print(" <");Serial.print(m++); Serial.print(">: ");
  printTime(millis());

  Serial.print(" ::[L:");Serial.print(L);Serial.print(" C:");Serial.print(C); Serial.print(" R:");Serial.print(R); Serial.print(" cnt:");
  Serial.print(cnt);  cnt=0;
  print_LCR();
  if (s == FULL_WHITE || s == FULL_BLACK)
  {
   Serial.print(" SumL("); Serial.print((uint32_t)sumL); Serial.print(") SumR("); Serial.print((uint32_t)sumR); Serial.print(") ");
   
  }
  Serial.println("]");
  prevState = s;

}

void print_LCR()
{
  Serial.print("(L:(");  queueL.printQueue(); 
  Serial.print(") C:("); queueC.printQueue(); 
  Serial.print(") R:("); queueR.printQueue(); 
  Serial.print(") ");
}


void printSum()
{
   Serial.print("sumForward ");  Serial.print(sumForward);  Serial.print(" sumLeft ");  Serial.print(sumLeft);
   Serial.print(" sumRight ");   Serial.print(sumRight);    Serial.print(" sumBlack "); Serial.print(sumBlack);   
   Serial.print(" sumWhite ");   Serial.println(sumWhite);  
}

void waitStraightLine() 
{
  Serial.println("wait Line");
  while (1) {
    SensorReadings d = (getAverageReadings());
    L = d.L;
    C = d.C;
    R = d.R;
      if ( C > C_min_bias) 
      {
        state = DRIVE_STRAIGHT;
        return;
      }
  }
}

void calibrateSensors() {
  uint8_t dt = 71;
  uint8_t N = 2;
  SensorReadings d = { 0, 0, 0 };
  long int L = 0;
  long int C = 0;
  long int R = 0;
  //  zero line
  for (int i = 0; i < 255; i++) {
    d = getAverageReadings();
    L += d.L;
    C += d.C;
    R += d.R;
  }
  L_min = L >> 8;
  C_max = C >> 8;
  R_min = R >> 8;

  Motor(ContraClockwise, 255,255);
  delay(dt);
  Motor(Stop, 0, 0);  // shift left

  delay(3000);
  L = R = C = 0;
  for (int i = 0; i < 255; i++) {
    d = getAverageReadings();
    L += d.L;
    C += d.C;
    R += d.R;
  }

  C_min = C >> 8;
  R_max = R >> 8;
  Motor(Clockwise, 255, 255);
  delay(dt);
  Motor(Stop, 0, 0);
  delay(3000);  // shift right -restore zero line
  Motor(Clockwise, 255, 255);
  delay(dt);
  Motor(Stop, 0, 0);
  delay(3000);  // shift right

  L = R = C = 0;
  for (int i = 0; i < 255; i++) {
    d = getAverageReadings();
    L += d.L;
    C += d.C;
    R += d.R;
  }

  L_max = L >> 8;

  Motor(ContraClockwise, 255,255);
  delay(dt);
  Motor(Stop, 0, 0);  // shift left - restore zero line
  Serial.print("Calibration:");
  Serial.print("  L_min: ");
  Serial.print(L_min);
  Serial.print(", L_max: ");
  Serial.print(L_max);
  Serial.print(", C_min: ");
  Serial.print(C_min);
  Serial.print(", C_max: ");
  Serial.print(C_max);
  Serial.print(", R_min: ");
  Serial.print(R_min);
  Serial.print(", R_max: ");
  Serial.println(R_max);

  delay(5000);
}




bool UFcheckDistance(char GE, uint16_t dist)
{
        digitalWrite(TRIGGER_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIGGER_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_PIN, LOW);

    uint32_t duration = pulseIn(ECHO_PIN, HIGH, 4000);

    uint32_t distance = duration * 10 / 58; // in mm

    //  Serial.println(distance); 

        if (GE == '>')
        {  if (distance >= dist || duration == 0) 
           return true;  
          else  
           return false;
        }
        else
        {  if (distance <= dist && duration != 0) 
           return true;  
          else  
            return false;
        }
}


void obstacleAvoidance(uint8_t direction)
{
  Motor(Stop, 0, 0);
  delay(1500);
  if(direction == Turn_Left)
  {
     // Move LEFT
      Serial.println(" Obstacle move left");
      Motor(Turn_Left, speedH, speedH);          delay(850);
      Motor(Forward, speedH, speedH);            delay(2000);
      Motor(Turn_Right, speedH, speedH);         delay(1000);
  }

  if(direction == Turn_Right)
  {
    Serial.println(" Obstacle move right");
      Motor(Turn_Right, speedH, speedH);         delay(850);
      Motor(Forward, speedH, speedH);            delay(2000);
      Motor(Turn_Left, speedH, speedH);          delay(1000);
  }
    while(1)
    {
     // if (UFcheckDistance('>', 200) ) // Obstacle not present

        waitStraightLine();
         
        return;

    } 
  



} 

