#define BUTTON_YES 32
#define BUTTON_NO 33

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_YES, INPUT_PULLUP);
  pinMode(BUTTON_NO, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(BUTTON_YES) == LOW) {
    Serial.println("YES Button Pressed");
    delay(300);
  }

  if (digitalRead(BUTTON_NO) == LOW) {
    Serial.println("NO Button Pressed");
    delay(300);
  }
}