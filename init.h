

// PWM control pin
#define PWM1_PIN 5
#define PWM2_PIN 6
// 74HCT595N chip pin
#define SHCP_PIN 2  // The displacement of the clock
#define EN_PIN 7    // Can make control
#define DATA_PIN 8  // Serial data
#define STCP_PIN 4  // Memory register clock

#define L_PIN A0
#define C_PIN A1
#define R_PIN A2

#define servo_PIN  9

#define TRIGGER_PIN  12
#define ECHO_PIN  13

const uint8_t LeftUpBack = 128;
const uint8_t LeftUpForw = 64;
const uint8_t LeftDownForw = 32;
const uint8_t LeftDownBack = 16;
const uint8_t RightDownBack = 8;
const uint8_t RightDownForw = 1;
const uint8_t RightUpForw = 4;
const uint8_t RightUpBack = 2;

const uint8_t Forward    = LeftUpForw | LeftDownForw | RightDownForw | RightUpForw;       // forward
const uint8_t Backward   = LeftUpBack | LeftDownBack | RightDownBack | RightUpBack;       // back
const uint8_t Turn_Left  = LeftUpBack | LeftDownForw | RightUpForw | RightDownBack;       // left translation
const uint8_t Turn_Right = LeftUpForw | LeftDownBack | RightUpBack | RightDownForw;       // Right translation
const uint8_t Top_Left = LeftDownForw | RightUpForw;                                      // Upper left mobile
const uint8_t Bottom_Left = LeftUpBack | RightDownBack;                                   // Lower left mobile
const uint8_t Top_Right = LeftUpForw | RightDownForw;                                     // Upper right mobile
const uint8_t Bottom_Right = LeftDownBack | RightUpBack;                                  // The lower right move
const uint8_t Stop = 0;                                                                   // stop
const uint8_t ContraClockwise = LeftUpBack | LeftDownBack | RightDownForw | RightUpForw;  // Counterclockwise rotation

const uint8_t Clockwise = LeftUpForw | LeftDownForw | RightDownBack | RightUpBack;        // Rotate clockwise

bool Toggle = true;

uint16_t L_min_bias, L_max_bias;
uint16_t C_min_bias, C_max_bias;
uint16_t R_min_bias, R_max_bias;

int16_t L_min, C_min, R_min;
int16_t L_max, C_max, R_max;

struct FullSensorData {
  uint8_t L;
  uint8_t C;
  uint8_t R;
  uint8_t state;
};

enum State { DRIVE_STRAIGHT,
             TURN_LEFT,
             TURN_RIGHT,
             MOVE_LEFT,
             MOVE_RIGHT,
             CORR_LEFT,
             CORR_RIGHT,
             SMALL_CORR_LEFT,
             SMALL_CORR_RIGHT,
             UNKNOWN,
             DRIVE_STOP,
             BACKWARD,
             FULL_WHITE,
             FULL_BLACK };

class Queue
{
  private:
    static const int MAX_QUEUE_SIZE = 5; // максимальний розмір черги
    uint8_t data[MAX_QUEUE_SIZE]; // масив для зберігання даних
    int front; // індекс початку черги
    int rear; // індекс кінця черги
  public:
    Queue(): front(0), rear(0) {} // конструктор, який ініціалізує чергу

    // Функція, яка додає елемент в чергу
    void enqueue(uint8_t element) {
      if((rear + 1) % MAX_QUEUE_SIZE == front) {
        // якщо черга повна, видаляємо перший елемент
        front = (front + 1) % MAX_QUEUE_SIZE;
      }
      data[rear] = element;
      rear = (rear + 1) % MAX_QUEUE_SIZE;
    }

    // Функція, яка друкує останні елементи черги
    void printQueue() {
      int i = front;
      while(i != rear) {
        Serial.print(data[i]); Serial.print(" ");
        i = (i + 1) % MAX_QUEUE_SIZE;
      }
    }
};

uint16_t L_base = 0;
uint16_t C_base = 0;
uint16_t R_base = 0;

State prev_State = UNKNOWN;
State prev_cmd_state = UNKNOWN;

uint8_t speedH = 250;  // 250
uint8_t speedM = 190;  //230
uint8_t speedL = 180;

State state = DRIVE_STRAIGHT;

int16_t loopCount=0;;
uint8_t L, C, R;

uint32_t tstart;

Queue queueL, queueC, queueR;
uint64_t sumL = 0;
uint64_t sumR = 0;
Servo myservo;
                                              

int16_t ax, ay, az;
int16_t gx, gy, gz;

GKalman filterGZ(600, 250, 0.6); 
GKalman filterGX(600, 250, 0.6); //  
GKalman filterGY(600, 250, 0.6); //

uint32_t timer = 0;
float timeStep = 0.1;

uint32_t cnt = 0;
uint32_t sumForward = 0;
uint32_t sumLeft = 0;
uint32_t sumRight = 0;
uint32_t sumBlack = 0;
uint32_t sumWhite = 0;
uint32_t startRotate;