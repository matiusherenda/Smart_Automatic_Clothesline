#define KIPAS_A 2
#define KIPAS_B 3
#define DINAMO_A 4
#define DINAMO_B 5
#define LED 6
#define BUTTON_ON 8
#define BUTTON_OFF 9
#define RAINDROP A0
#define LDR A1
#define POTENTIO A2

int LDR_THRESHOLD = 1000; // Nilai threshold untuk sensor LDR
const int RAIN_THRESHOLD = 400; // Nilai threshold untuk sensor raindrop
const int BUTTON_PRESS_TIME = 3000; // Waktu minimum untuk masuk ke mode konfigurasi LDR
const int SENSOR_READ_INTERVAL = 2000; // Interval membaca sensor

int state = 0;
int lastSensorReadTime = 0;
int lastCondition = 0;
int currentCondition = 0;
int position = 0;
byte speed = 15; // Kecepatan motor

int ldrValue = 0;
int raindropValue = 0;

void checkCondition();
void turnOffMachine();
void configureLDRThreshold();

void setup() {
    pinMode(DINAMO_A, OUTPUT);
    pinMode(DINAMO_B, OUTPUT);
    pinMode(KIPAS_A, OUTPUT);
    pinMode(KIPAS_B, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(BUTTON_ON, INPUT_PULLUP);
    pinMode(BUTTON_OFF, INPUT_PULLUP);
    pinMode(LDR, INPUT);
    pinMode(RAINDROP, INPUT);
    pinMode(POTENTIO, INPUT);

    Serial.begin(9600);
}

void loop() {
  int buttonOnState = digitalRead(BUTTON_ON);
  int buttonOffState = digitalRead(BUTTON_OFF);

  if (buttonOffState == LOW) {
    turnOffMachine();
    state = 0;
  }

  if (buttonOnState == LOW) {
    unsigned long buttonPressStartTime = millis();

    while (digitalRead(BUTTON_ON) == LOW); // Tunggu tombol dilepaskan

    unsigned long buttonReleaseTime = millis();
    if (buttonReleaseTime - buttonPressStartTime >= BUTTON_PRESS_TIME) {
      configureLDRThreshold(); // Masuk ke mode konfigurasi jika tombol ditekan lebih dari 3 detik
    } else {
      state = 1; // Tetap di mode operasional
    }
  }

  if (state == 1) {
    checkCondition();
  }
}

void checkCondition() {
  digitalWrite(LED, HIGH); // Menyalakan LED untuk menunjukkan mesin beroperasi

  // Membaca sensor pada interval yang ditentukan
  unsigned long currentTime = millis();
  if ((currentTime - lastSensorReadTime) > SENSOR_READ_INTERVAL) {
    lastSensorReadTime = currentTime;
    raindropValue = analogRead(RAINDROP);
    ldrValue = analogRead(LDR);
    Serial.println(ldrValue);
    Serial.println(raindropValue);
  }

  // Menentukan kondisi saat ini berdasarkan nilai sensor
  if (ldrValue > LDR_THRESHOLD && raindropValue > RAIN_THRESHOLD) currentCondition = 1; // Terang dan kering
  else if (ldrValue < LDR_THRESHOLD && raindropValue < RAIN_THRESHOLD) currentCondition = 2; // Gelap dan basah
  else if (ldrValue > LDR_THRESHOLD && raindropValue < RAIN_THRESHOLD) currentCondition = 3; // Terang dan basah
  else if (ldrValue < LDR_THRESHOLD && raindropValue > RAIN_THRESHOLD) currentCondition = 4; // Gelap dan kering

  // Update sistem jika kondisi telah berubah
  if (lastCondition != currentCondition) {
    lastCondition = currentCondition;
    handleConditionChange();
  }
}

void handleConditionChange() {
  switch (currentCondition) {
    case 1:
      Serial.println("Terang kering");
      if (position == 0) {
        startMachine();
        position = 1;
      }
      stopMachine();
      break;

    case 2:
      Serial.println("Gelap basah");
      if (position == 1) {
        startMachine();
        position = 0;
      }
      activateFan();
      stopMachine();
      break;

    case 3:
      Serial.println("Terang basah");
      if (position == 1) {
        startMachine();
        position = 0;
      }
      activateFan();
      stopMachine();
      break;

    case 4:
      Serial.println("Gelap kering");
      if (position == 1) {
        startMachine();
        position = 0;
      }
      stopFan();
      stopMachine();
      break;
  }
}

void startMachine() {
  digitalWrite(DINAMO_A, speed);
  digitalWrite(DINAMO_B, 0);
}

void stopMachine() {
  digitalWrite(DINAMO_A, 0);
  digitalWrite(DINAMO_B, 0);
  delay(150);
}

void activateFan() {
  digitalWrite(KIPAS_A, HIGH);
  digitalWrite(KIPAS_B, LOW);
}

void stopFan() {
  digitalWrite(KIPAS_A, LOW);
  digitalWrite(KIPAS_B, LOW);
}

void turnOffMachine() {
  digitalWrite(LED, LOW);
  lastCondition = 0;
  if (position == 1) {
    startMachine();
    position = 0;
  }
  stopFan();
  stopMachine();
}

void configureLDRThreshold() {
  for (int i = 0; i < 3; i++) {
    blinkLED();
  }

  int buttonState = HIGH;
  while (buttonState == HIGH) {
    int potentiometerValue = analogRead(POTENTIO);
    Serial.println(potentiometerValue);
    LDR_THRESHOLD = map(potentiometerValue, 0, 1023, 0, 1023);
    buttonState = digitalRead(BUTTON_ON);
    Serial.println(buttonState);
  }
}

void blinkLED() {
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}