#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 19
#define LED_PIN 2

float velocity = 0;
float threshold_velocity = 2.0; // m/s (tune it)
unsigned long prevTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  if(!accel.begin()) {
    Serial.println("No ADXL345 detected");
    while(1);
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display not found");
    while(1);
  }
  display.clearDisplay();
  display.display();

  Serial.println("System Ready");
}

void loop() {
  sensors_event_t event; 
  accel.getEvent(&event);

  unsigned long currentTime = millis();
  float dt = (currentTime - prevTime) / 1000.0; // seconds
  prevTime = currentTime;

  // Approximate velocity (basic integration)
  velocity += event.acceleration.x * dt;
  velocity = constrain(velocity, -5, 5); // limit drift

  // Velocity Alert
  if (abs(velocity) > threshold_velocity) {
    digitalWrite(LED_PIN, HIGH);
    displayAlert(velocity);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Ultrasonic sensor
  float distance = getDistance();
  controlBuzzer(distance);

  delay(100);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void controlBuzzer(float distance) {
  int delayTime;
  if (distance > 100) {
    noTone(BUZZER_PIN);
    return;
  } else if (distance > 80) delayTime = 1000;
  else if (distance > 40) delayTime = 500;
  else delayTime = 200;

  tone(BUZZER_PIN, 1000);
  delay(delayTime / 2);
  noTone(BUZZER_PIN);
}

void displayAlert(float velocity) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Speed: ");
  display.print(velocity, 2);
  display.println(" m/s");
  display.println("ALERT: Speed Limit!");
  display.display();
}