#include <Wire.h>
#include <LiquidCrystal.h>
#include <MsTimer2.h>
#include <Encoder.h>
#define FREQ_CTRL 200
#define MAX_SPEED 70
#define MIN_SPEED -40
#define BASE_SPEED 50
#define I2C_ADDRESS 42

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Encoder myEncoder(8, 9);

enum lcd_display_values {
  DISPLAY_KP,
  DISPLAY_KI,
  DISPLAY_KD,
} display_state;

float encoder_reading = 0;


int motorsBase[4];
int motors[4];
float offset = 0;
int error = 0;
int errorSum = 0;
int prev_error = 0;
float kp, kd, ki;

unsigned char dataRaw[16];
unsigned int sensorData[8];
const int ratio[8] = { -30, -25, -13, -5, 5, 13, 25, 30 };

void timer_init() {
  MsTimer2::set((1000 / FREQ_CTRL), Timer2ISR);
  MsTimer2::start();
}

void setup() {
  Wire.begin();
  Serial.begin(9600);  // Start serial for output to PC 
  lcd.begin(16,2);
  for (int i = 0; i < 4; i++) {
    motors[i] = BASE_SPEED;
    motorsBase[i] = BASE_SPEED;
  }
  timer_init();
}

void Timer2ISR() {
  updateMotors();
}

int count = 0;
void loop() {
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
}
float newOffset = 0; 


// display and modify params of pid according to current display state
void display_and_modify() {
  char printmsg[16];
  lcd.setCursor(0, 0);
  encoder_reading = myEncoder.read();
  switch(display_state) {
    case DISPLAY_KP:
      kp += (float) encoder_reading / 100;
      dtostrf(kp, 16, 2, printmsg);
      lcd.print("K_P: ");
      lcd.print(kp);
      break;
    case DISPLAY_KI:
      ki += (float) encoder_reading / 10000;
      dtostrf(ki, 16, 4, printmsg);
      lcd.print("K_I: ");
      lcd.print(ki);
      break;
    case DISPLAY_KD:
      kd += (float) encoder_reading / 10;
      dtostrf(kd, 16, 2, printmsg);
      lcd.print("K_D: ");
      lcd.print(kd);
      break;
  }
}


void updateMotors() {
  calculateError();
  errorSum += error;
  if (sensorData[7] > sensorData[6]) {
    errorSum += 0.5 * (sensorData[7] - sensorData[6]);
  }
  // kp = 3.93;
  // ki = 0.0926;
  // kd = 2.1;

  kp = 5.2;
  ki = 0.019;
  kd = 7;

  offset = (float)kp * error + ki * errorSum + kd * (error - prev_error); 
  motors[0] = constrain(motorsBase[0] + map(-offset), MIN_SPEED, MAX_SPEED);  // 右前
  motors[1] = constrain(motorsBase[0] + map(-offset), MIN_SPEED, MAX_SPEED);  // 右后

  motors[2] = constrain(motorsBase[3] + map(+offset), MIN_SPEED, MAX_SPEED);  // 左前
  motors[3] = constrain(motorsBase[3] + map(+offset), MIN_SPEED, MAX_SPEED);  // 左后
  prev_error = error;
}

float map(float num) {
  int negative = MIN_SPEED - BASE_SPEED;
  int positive = BASE_SPEED - MAX_SPEED;
  if (num <= 0) {
    return 2.6 * num;
  }
  if (num > 0) {
    return num;
  }
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