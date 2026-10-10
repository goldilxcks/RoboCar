#include    <Servo.h>

#define servo_PIN    9

Servo myservo;

void setup()
{
    myservo.attach(servo_PIN, 544 , 2400);
   // myservo.write(90);
}

void loop()
{
        myservo.write(180);    
        delay(3500);

       /* myservo.write(45);
        delay(3500);

        myservo.write(0);
        delay(3500);

        myservo.write(135);
        delay(3500);

        myservo.write(180);
        delay(3500);
        */
}
