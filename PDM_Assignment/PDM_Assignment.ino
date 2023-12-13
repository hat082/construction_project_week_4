#include <Wire.h>
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#include <PinChangeInterrupt.h>
#define I2C_ADDRESS 42

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

enum robotStateDef {
  SELECT_LAPS,
  EXECUTE_TASK,
}robotState;

volatile int encoderPinA = A0;                   // Rotary encoder Pin A
int encoderPinB = A1;                             // Rotary encoder Pin B
volatile int pinAStateCurrent = LOW;             // Current state of Pin A
volatile int pinAStateLast = pinAStateCurrent;   // Last read value of Pin A

int button = 8;

int motors[4] = { 40, 40, 40, 40 };
int num_of_laps = 0; // modify using encoder
int lap_num = 0; // the current lap num

void pin_init() {
  pinMode(button, INPUT_PULLUP);
  pinMode(encoderPinA, INPUT_PULLUP);             // Set encoderPinA as input
  pinMode(encoderPinB, INPUT_PULLUP);             // Set encoderPinB as input
  attachPinChangeInterrupt(digitalPinToPCINT(button), options, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), update_num_of_laps, CHANGE);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC
  pin_init();
  lcd.begin(16, 2);
  robotState = SELECT_LAPS;
  // buzzer init

  // display lcd screen for selecting num of laps

  // use button press on the lcd to confirm num of laps

  // debug 
  num_of_laps = 3;
  // move("bafffr"); // 左后 右后 左前 右前
  // while(1);
}

const int a = 1000;
const int b = 1710;

void loop() {
  delay(10);
  
  switch (robotState) {
    case SELECT_LAPS:
      lcd.setCursor(0, 0);
      lcd.print("Laps: ");
      lcd.print(num_of_laps);
      lcd.setCursor(0, 1); 
      lcd.print("Press button to confirm..")
    case EXECUTE_TASK:
      delay(1000);
      executeTask();
      robotState = SELECT_LAPS;
      break;
  }
}

// start executing task at the touch of the button
void options() {
  switch(robotState) {
    case SELECT_LAPS:
      robotState = EXECUTE_TASK;
  }
}

// updates rotary encoder values if there are any detected
void update_num_of_laps() {
  pinAStateCurrent = digitalRead(encoderPinA);   // Read the current state of Pin A

  // If there is a minimal movement of 1 step
  if ((pinAStateLast == HIGH) && (pinAStateCurrent == LOW)) {
    if (digitalRead(encoderPinB) == HIGH) {      // If encoderPinB is HIGH
      num_of_laps = constrain(num_of_laps + 1, 0, 10);
    } else {
      num_of_laps = constrain(num_of_laps - 1, 0, 10);
    }
  }
  pinAStateLast = pinAStateCurrent;              // Store the latest read value in the current state variable
}

void executeTask() {
  if (lap_num >= num_of_laps) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write("ha");
    Wire.endTransmission(I2C_ADDRESS);
  }
  // move forward a
  setMotorsForward();
  move("bafffr");
  delay(a);
  
  // turn right b
  setMotorsTurnRight();
  move("bafffr");
  delay(b);
  
  // Wire.beginTransmission(I2C_ADDRESS);
  // Wire.write("ha");
  // Wire.endTransmission(I2C_ADDRESS);
  // while(1);

  
  // move forward 2a
  setMotorsForward();
  move("bafffr");
  delay(2 * a - 100);

  // turn left b
  setMotorsTurnLeft();
  move("bafffr");
  delay(b + 130);
  
  // move forward a
  setMotorsForward();
  move("bafffr");
  delay(a);
  lap_num++; 
}

void setMotorsForward() {
  motors[0] = 40;
  motors[1] = 40;
  motors[2] = 40;
  motors[3] = 40;
}

void setMotorsTurnRight() {
  motors[0] = 60;
  motors[1] = 10;
  motors[2] = 60;
  motors[3] = 10;
}

void setMotorsTurnLeft() {
  motors[0] = 10;
  motors[1] = 60;
  motors[2] = 10;
  motors[3] = 60;
}

void move(char cmd[]) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(cmd);
  for (int i = 0; i < 4; i++) {
    Wire.write(motors[i]);
    Wire.write(0);
  }
  Wire.endTransmission();
}
