const int left = A0;
void setup() {
  Serial.begin(9600);
}
void loop() {
  int leftValue = analogRead(left);
  Serial.println(leftValue);
  delay(1000);
}