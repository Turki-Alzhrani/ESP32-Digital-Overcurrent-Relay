#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);

// ESP32 Pins
const int voltagePin = 35;
const int currentPin = 34;
const int relayPin = 26;
const int buttonPin = 27;

// Voltage divider
const float Rv1 = 10000.0;
const float Rv2 = 2150.0;

// Current measurement
const float Rshunt = 5.0;
const float currentCalibration = 0.853;

// Protection settings
const float tripCurrentmA = 60.0;
const int tripCountRequired = 3;
const unsigned long startupDelay = 2000;

// Relay active LOW
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

const int samples = 100;

// System state
bool tripped = true;
bool overcurrentFault = false;

float lastTripCurrent = 0.0;
int overcurrentCount = 0;

unsigned long motorStartTime = 0;
unsigned long lastPress = 0;

bool lastButtonState = HIGH;

float readMilliVolts(int pin) {
  float sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += analogReadMilliVolts(pin);
    delay(2);
  }

  return sum / samples;
}

void setRelay(bool on) {
  digitalWrite(relayPin, on ? RELAY_ON : RELAY_OFF);
}

void setup() {
  Serial.begin(115200);

  digitalWrite(relayPin, RELAY_OFF);
  pinMode(relayPin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  analogReadResolution(12);
  analogSetPinAttenuation(voltagePin, ADC_11db);
  analogSetPinAttenuation(currentPin, ADC_11db);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true) delay(1000);
  }

  display.setTextColor(SSD1306_WHITE);
  setRelay(false);
}

void loop() {

  // Reset button
  bool buttonState = digitalRead(buttonPin);

  if (lastButtonState == HIGH &&
      buttonState == LOW &&
      millis() - lastPress > 250) {

    lastPress = millis();

    if (tripped) {
      tripped = false;
      overcurrentFault = false;
      lastTripCurrent = 0.0;
      overcurrentCount = 0;

      motorStartTime = millis();
      setRelay(true);
    }
  }

  lastButtonState = buttonState;

  // Voltage reading
  float supplyADC =
      readMilliVolts(voltagePin) / 1000.0;

  float supplyVoltage =
      supplyADC * (Rv1 + Rv2) / Rv2;

  // Current reading
  float shuntADC =
      readMilliVolts(currentPin) / 1000.0;

  float shuntVoltage =
      shuntADC * currentCalibration;

  float currentmA =
      (shuntVoltage / Rshunt) * 1000.0;

  bool starting =
      !tripped &&
      (millis() - motorStartTime < startupDelay);

  // Overcurrent protection
  if (!tripped && !starting) {

    if (currentmA > tripCurrentmA) {
      overcurrentCount++;
    } else {
      overcurrentCount = 0;
    }

    if (overcurrentCount >= tripCountRequired) {
      lastTripCurrent = currentmA;
      overcurrentFault = true;
      tripped = true;

      setRelay(false);

      Serial.printf(
        "FAULT: OVERCURRENT | Trip current: %.1f mA\n",
        lastTripCurrent
      );
    }
  }

  float displayCurrent = tripped ? 0.0 : currentmA;

  display.clearDisplay();
  display.setTextSize(1);

  // Fault screen
  if (tripped && overcurrentFault) {

    display.setCursor(10, 0);
    display.println("PROTECTION TRIPPED");

    display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

    display.setCursor(0, 19);
    display.println("FAULT: OVERCURRENT");

    display.setCursor(0, 32);
    display.print("Trip: ");
    display.print(lastTripCurrent, 1);
    display.println(" mA");

    display.setCursor(0, 44);
    display.print("Limit: ");
    display.print(tripCurrentmA, 0);
    display.println(" mA");

    display.setCursor(0, 56);
    display.println("Press RESET");

  } else {

    display.setCursor(12, 0);
    display.println("DIGITAL RELAY");

    display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

    display.setCursor(0, 20);
    display.print("Voltage: ");
    display.print(supplyVoltage, 2);
    display.println(" V");

    display.setCursor(0, 34);
    display.print("Current: ");
    display.print(displayCurrent, 1);
    display.println(" mA");

    display.setCursor(0, 51);

    if (tripped) {
      display.println("Status: OFF");
    } else if (starting) {
      display.println("Status: STARTING");
    } else {
      display.println("Status: NORMAL");
    }
  }

  display.display();

  Serial.printf(
    "Voltage: %.2f V | Current: %.1f mA | %s\n",
    supplyVoltage,
    displayCurrent,
    tripped ? (overcurrentFault ? "OVERCURRENT" : "OFF") :
    (starting ? "STARTING" : "NORMAL")
  );

  delay(100);
}