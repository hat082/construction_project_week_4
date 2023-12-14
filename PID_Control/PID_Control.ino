#include <Wire.h>
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#include <PinChangeInterrupt.h>

#define FREQ_CTRL 200
#define MAX_SPEED 52
#define MIN_SPEED -40
#define BASE_SPEED 40
#define I2C_ADDRESS 42

enum lcd_display_values {
  DISPLAY_KP,
  DISPLAY_KI,
  DISPLAY_KD,
} display_state;


const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int encoder_reading = 10;                              // Rotary encoder reading (not motor encoder)

volatile int encoderPinA = A0;                   // Rotary encoder Pin A
int encoderPinB = A1;                             // Rotary encoder Pin B
volatile int pinAStateCurrent = LOW;             // Current state of Pin A
volatile int pinAStateLast = pinAStateCurrent;   // Last read value of Pin A

int button = 8;

int motors[4];
int motorsBase[4];
float offset = 0;
int error = 0;
int errorSum = 0;
int prev_error = 0;
float kp, kd, ki;

unsigned char dataRaw[16];
unsigned int sensorData[8];
const int ratio[8] = { -70, -30, -7, -3, 3, 7, 30, 70 };

void pin_init() {
  pinMode(button, INPUT_PULLUP);
  pinMode(encoderPinA, INPUT_PULLUP);             // Set encoderPinA as input
  pinMode(encoderPinB, INPUT_PULLUP);             // Set encoderPinB as input
  attachPinChangeInterrupt(digitalPinToPCINT(button), options, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC 
  lcd.begin(16,2);
  pin_init();
  for (int i = 0; i < 4; i++) {
    motors[i] = BASE_SPEED;
    motorsBase[i] = BASE_SPEED;
  }
}

int count = 0; // counter that counts the occurances of loops, so that the lcd is not updated too often
void loop() {
  encoder_reading = 0; // encoder value stays 0 until there is a interrupt caused by the turning of the encoder
  if (count < 20) {
    count++;
  }
  else {
    display_and_modify();
    count = 0;
  }

  delay(10);
  move();
  // for (int i = 0; i < 8; i++) {
  //   Serial.print(sensorData[i]);
  //   Serial.print("\t");

  // }
  // Serial.println();
  
  // Serial.print("\t");
  // Serial.print(motors[0]);
  // Serial.print("\t");
  // Serial.println(motors[4]);
  readSensorData();
  updateMotors();
}

// updates rotary encoder values if there are any detected
void updateEncoder() {
  pinAStateCurrent = digitalRead(encoderPinA);   // Read the current state of Pin A

  // If there is a minimal movement of 1 step
  if ((pinAStateLast == HIGH) && (pinAStateCurrent == LOW)) {
    if (digitalRead(encoderPinB) == HIGH) {      // If encoderPinB is HIGH
      encoder_reading = 1;
    } else {
      encoder_reading = -1;
    }
  }
  pinAStateLast = pinAStateCurrent;              // Store the latest read value in the current state variable
}

// change the display state using button
void options() {
  switch(display_state) {
    case DISPLAY_KP:
      display_state = DISPLAY_KI;
    case DISPLAY_KI:
      display_state = DISPLAY_KD;
    case DISPLAY_KD:
      display_state = DISPLAY_KP;
  }
}

// display and modify params of pid according to current display state
void display_and_modify() {
  char printmsg[16];
  lcd.setCursor(0, 0);
  switch(display_state) {
    case DISPLAY_KP:
      kp += (float) encoder_reading / 100;
      dtostrf(kp, 16, 2, printmsg);
      lcd.print(kp);
      break;
    case DISPLAY_KI:
      ki += (float) encoder_reading / 10000;
      dtostrf(ki, 16, 4, printmsg);
      lcd.print(kp);
      break;
    case DISPLAY_KD:
      kd += (float) encoder_reading / 10;
      dtostrf(kd, 16, 2, printmsg);
      lcd.print(kp);
      break;
  }
}

// udpate the motor speed according to a offset calculated using PID
void updateMotors() {
  calculateError();
  errorSum += error;

  kp = 3.93;
  ki = 0.0916;
  kd = 1.8;

  offset = (float)kp * error + ki * errorSum + kd * (error - prev_error); 
  motors[0] = constrain(motorsBase[0] - offset, MIN_SPEED, MAX_SPEED);  // 右前
  motors[1] = constrain(motorsBase[1] - offset, MIN_SPEED, MAX_SPEED);  // 右后
  motors[3] = constrain(motorsBase[2] + offset, MIN_SPEED, MAX_SPEED);  // 左后
  motors[2] = constrain(motorsBase[3] + offset, MIN_SPEED, MAX_SPEED);  // 左前
  prev_error = error;
}

// multiply the sensor data with ratio to obtain a singular value (error) that indicates the distance the black line is from the center
void calculateError() {
  error = 0;
  for (int i = 0; i < 8; i++) {
    error += sensorData[i] * ratio[i];
  }
  error /= 100;
}

// send commands to motors according to the speed set
void move() {
  Wire.beginTransmission(I2C_ADDRESS);
  if (motors[0] >= 0 && motors[3] >= 0) {
    Wire.write("bafffr");
  }
  else if (motors[0] < 0) {
    Wire.write("barrfr");
  }
  else if (motors[3] < 0) {
    Wire.write("baffrf");
  }

  for (int i = 0; i < 4; i++) {
    Wire.write(abs(motors[i]));
    Wire.write(0);
  }
  Wire.endTransmission();
}

// read the values from the 8 sensors and convert them to numbers between 1 to 100 (low if bright high if dark)
void readSensorData(void) {
  unsigned char n;            // Variable for counter value
  unsigned char dataRaw[16];  // Array for raw data from module

  // Request data from the module and store into an array
  n = 0;                      // Reset loop variable
  Wire.requestFrom(9, 16);    // Request 16 bytes from slave device #9 (IR Sensor)
  while (Wire.available()) {  // Loop until all the data has been read
    if (n < 16) {
      dataRaw[n] = Wire.read();  // Read a byte and store in raw data array
      n++;
    } else {
      Wire.read();  // Discard any bytes over the 16 we need
      // n = 0;
    }
  }

  // Loop through and covert two 8 bit values to one 16 bit value
  // Raw data formatted as "MSBs 10 9 8 7 6 5 4 3", "x x x x x x 2 1 LSBs"
  for (n = 0; n < 8; n++) {
    sensorData[n] = dataRaw[n * 2] << 2;    // Shift the 8 MSBs up two places and store in array
    sensorData[n] += dataRaw[(n * 2) + 1];  // Add the remaining bottom 2 LSBs

    // divide by 10
    sensorData[n] /= 10;
    sensorData[n] = 100 - sensorData[n];
  }
}