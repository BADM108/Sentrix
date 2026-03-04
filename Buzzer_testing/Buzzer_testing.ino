#define BUZZER_PIN 27

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  digitalWrite(BUZZER_PIN, HIGH); // ON
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);  // OFF
  delay(1000);
}