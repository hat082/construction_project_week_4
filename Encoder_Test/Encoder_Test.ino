#define outputA 8
#define outputB 9

int counter = 0;
int aState;
int aLastState;

void setup() {
  pinMode(outputA, INPUT_PULLUP);
  pinMode(outputB, INPUT_PULLUP);

  Serial.begin(9600);
  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA);
}

void loop() {
  // aState = digitalRead(outputA);  // Reads the "current" state of the outputA
  // // If the previous and the current state of the outputA are different, that means a Pulse has occured
  // if (aState == HIGH && aLastState == LOW) {
  //   // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
  //   // if (digitalRead(outputB) != aState) {
  //   //   counter++;
  //   // } else {
  //   //   counter--;
  //   // }
  //   // Serial.print("Position: ");
  //   // Serial.println(counter);
  //   Serial.println(digitalRead(outputB));
  // }
  // aLastState = aState;  // Updates the previous state of the outputA with the current state

  // counter += digitalRead(outputA) - digitalRead(outputB);
  if (digitalRead(outputA) > digitalRead(outputB))
  {
    counter++;
  }
  else if (digitalRead(outputA) < digitalRead(outputB))
  {
    counter--;
  }
  else {
    
  }
  Serial.println(counter);
}
