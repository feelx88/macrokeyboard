enum Status {
  RELEASED = 0,
  PRESSED = 1
};

class Key {
  public:
    int pin;
    char message;

    Status status;
    uint32_t lastClick;
};

uint32_t debounceDelay = 100;
int numKeys = 10;

Key keys[] = {
  {4, '0'},
  {5, '1'},
  {6, '2'},
  {7, '3'},
  {8, '4'},
  {9, '5'},
  {A0, '6'},
  {A1, '7'},
  {A2, '8'},
  {A3, '9'},
};

void setup() {
  for (int key = 0; key < numKeys; ++key) {
    pinMode(keys[key].pin, INPUT_PULLUP);
  }

  Serial.begin(9600);
}

void loop() {
  uint32_t now = millis();

  for (int key = 0; key < numKeys; ++key) {
    Key &currentKey = keys[key];

    if (now < currentKey.lastClick + debounceDelay) {
      continue;
    }

    int status = digitalRead(currentKey.pin);

    if (currentKey.status == Status::RELEASED && status == LOW) {
      currentKey.status = Status::PRESSED;
      currentKey.lastClick = now;
      Serial.println(String("press_") + String(currentKey.message));
    }

    if (currentKey.status == Status::PRESSED && status == HIGH) {
      currentKey.status = Status::RELEASED;
      currentKey.lastClick = now;
      Serial.println(String("release_") + String(currentKey.message));
    }
  }
}
