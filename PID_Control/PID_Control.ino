#include <Wire.h>
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#define FREQ_CTRL 200
#define MAX_SPEED 70
#define MIN_SPEED -20
#define BASE_SPEED 40
#define I2C_ADDRESS 42

int motorsBase[4];
int motors[4];
float offset = 0;
int error = 0;
int errorSum = 0;
int errorOld = 0;
float kp, kd, ki;

unsigned char dataRaw[16];
unsigned int sensorData[8];
const int ratio[8] = { -60, -35, -15, -5, 5, 15, 35, 60 };

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC
                       // timer_init();
  for (int i = 0; i < 4; i++) {
    motors[i] = BASE_SPEED;
    motorsBase[i] =BASE_SPEED;
  } 
}

void loop() {
  delay(10);
  move();
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
  readSensorData();
  updateMotors();
}

void updateMotors() {
  calculateError();
  errorSum += error;
  // kp = 4.00;
  // ki = 0.085;
  // kd = 0;

  kp = 12;
  ki = 0.08;
  kd = 0;

  offset = (float)kp * error - kd * abs(error - errorOld) + ki * errorSum;
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