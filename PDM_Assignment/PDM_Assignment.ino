#include <Wire.h>
#include <LiquidCrystal.h>
#define I2C_ADDRESS 42

int motors[4] = { 40, 40, 40, 40 };
int num_of_laps = 0; // modify using encoder
int lap_num = 0; // the current lap num


void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC
  // init lcd and encoder 

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
  if (lap_num >= num_of_laps) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write("ha");
    Wire.endTransmission(I2C_ADDRESS);
    while(1);
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
