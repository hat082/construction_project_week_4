#include <Wire.h>
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#define FREQ_CTRL 200
#define MAX_SPEED 70
#define MIN_SPEED -40
#define BASE_SPEED 40
#define I2C_ADDRESS 42

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


int motorsBase[4];
int motors[4];
float offset = 0;
int error = 0;
int errorSum = 0;
int prev_error = 0;
float kp, kd, ki;

unsigned char dataRaw[16];
unsigned int sensorData[8];
const int ratio[8] = { -70, -40, -10, -5, 5, 10, 40, 70 };

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC 
  lcd.begin(16,2);
  for (int i = 0; i < 4; i++) {
    motors[i] = BASE_SPEED;
    motorsBase[i] =BASE_SPEED;
  }
}
int count = 0;
void loop() {
  if (count < 20) {
    count++;
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print(error);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(offset);
    lcd.print("                ");
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
float newOffset = 0; 
void updateMotors() {
  calculateError();
  errorSum += error;

  // kp = 3.93;
  // ki = 0.0926;
  // kd = 2.1;

  kp = 3.98;
  ki = 0.0896;
  kd = 1.6;




  offset = (float)kp * error + ki * errorSum + kd * (error - prev_error); 
  motors[0] = constrain(motorsBase[0] - offset, MIN_SPEED, MAX_SPEED);  // 右前
  motors[1] = constrain(motorsBase[0] - offset, MIN_SPEED, MAX_SPEED);  // 右后
  motors[3] = constrain(motorsBase[3] + offset, MIN_SPEED, MAX_SPEED);  // 左后
  motors[2] = constrain(motorsBase[3] + offset, MIN_SPEED, MAX_SPEED);  // 左前
  prev_error = error;
}

void calculateError() {
  error = 0;

  for (int i = 0; i < 8; i++) {
    error += sensorData[i] * ratio[i];
  }
  error /= 100;

  // if (sensorData[7] > sensorData[6]) {
  //   error += (sensorData[7] - sensorData[6]);
  // }
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