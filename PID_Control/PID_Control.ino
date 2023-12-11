#include <Wire.h>
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#define FREQ_CTRL 200
#define MAX_SPEED 70
#define MIN_SPEED 0
#define I2C_ADDRESS 42

int motorsBase[4] = { 20, 20, 20, 20 };
int motors[4] = { 0, 0, 0, 0 };
float offset = 0;
int error = 0;
int errorSum = 0;
int errorOld = 0;
float kp, kd, ki;

unsigned char dataRaw[16];
unsigned int sensorData[8];
const int ratio[8] = { -40, -25, -15, -5, 5, 15, 25, 40 };

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC
  // timer_init();
}

void loop() {
  delay(10);
  readSensorData();
  updateMotors();
  // if (motors[0] < 0) {
  //   move("barrff");
  // }
  // else if (motors[4] < 0) {
  //   move("baffrr");
  // }
  // else {
  move("baffff");
  // }

  // for (int i = 0; i < 8; i++) {
  //   Serial.print(sensorData[i]);
  //   Serial.print("\t");

  // }
  // Serial.println();

  // Serial.println(error);
  // Serial.print("\t");
  // Serial.print(motors[0]);
  // Serial.print("\t");
  // Serial.println(motors[4]);
}

void updateMotors() {
  calculateError();
  errorSum += error;
  // kp = 4.00;
  // ki = 0.085;
  // kd = 0;

  kp = 4.00;
  ki = 0.08;
  kd = 0;

  offset = (float)kp * error - kd * (error - errorOld) + ki * errorSum;
  motors[0] = constrain(motorsBase[0] - offset, MIN_SPEED, MAX_SPEED);  // 右前
  motors[1] = constrain(motorsBase[0] - offset, MIN_SPEED, MAX_SPEED);  // 右后
  motors[3] = constrain(motorsBase[3] + offset, MIN_SPEED, MAX_SPEED);  // 左后
  motors[2] = constrain(motorsBase[3] + offset, MIN_SPEED, MAX_SPEED);  // 左前
  errorOld = error;
}

void calculateError() {
  error = 0;
  for (int i = 0; i < 8; i++) {
    error += sensorData[i] * ratio[i];
  }
  error /= 100;
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
      //n = 0;
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