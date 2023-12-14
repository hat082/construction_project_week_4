#define outputA 2
#define outputB 3

int counter = 0;

void setup() {
  pinMode(outputA, INPUT_PULLUP);
  pinMode(outputB, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  // counter += digitalRead(outputA) - digitalRead(outputB);
  if (digitalRead(outputA) > digitalRead(outputB)) {
    counter++;
  }
  else if (digitalRead(outputA) < digitalRead(outputB)) {
    counter--;
  }
  else {
  }
  Serial.println(counter);
}
