#include <Wire.h>
#include <LiquidCrystal.h>
#define I2C_ADDRESS 42

int motors[4] = { 40, 40, 40, 40 };
int num_of_laps = 0; // modify using encoder
int lap_num = 0; // the current lap num

// motors[0]  右前
// motors[1]  右后
// motors[2]  左前
// motors[3]  左后

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC
  // init lcd and encoder 

  // display lcd screen for selecting num of laps

  // use button press on the lcd to confirm num of laps

  // debug 
  num_of_laps = 1;
}

const int a = 1000;
const int b = 1000;

void loop() {
  delay(10);
  lap_num++;
  if (lap_num >= num_of_laps) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write("ha");
    Wire.endTransmission(I2C_ADDRESS);
    while(1);
  }
  // move forward a
  setMotorsForward();
  move("baffff");
  delay(a);
  
  // turn right b
  setMotorsTurnRight();
  move("baffff");
  delay(b);
  
  // move forward 2a
  setMotorsForward();
  move("baffff");
  delay(2 * a);

  // turn left b
  setMotorsTurnLeft();
  move("baffff");
  delay(b);
  
  // move forward a
  setMotorsForward();
  move("baffff");
  delay(a);
}

void setMotorsForward() {
  motors[0] = 40;
  motors[1] = 40;
  motors[2] = 40;
  motors[3] = 40;
}

void setMotorsTurnRight() {
  motors[0] = 30;
  motors[1] = 30;
  motors[2] = 50;
  motors[3] = 50;
}

void setMotorsTurnLeft() {
  motors[0] = 50;
  motors[1] = 50;
  motors[2] = 30;
  motors[3] = 30;
}

void move(char cmd[]) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(cmd);
  for (int i = 0; i < 4; i++) {
    Wire.write(abs(motors[i]));
    Wire.write(0);
  }
  Wire.endTransmission();
}
