#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6tAvoUOJ6"
#define BLYNK_TEMPLATE_NAME "TrafficLights"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>

// Define pins for seven segment display
#define CLK 27
#define DIO 14

TM1637Display display(CLK, DIO);

// Define traffic light pins for East-West (EW) and South (S)
#define RED_EW 23
#define YELLOW_EW 19
#define GREEN_EW 18

#define RED_S 12
#define YELLOW_S 13
#define GREEN_S 5

// Define sensor pins
#define SENSOR1 25
#define SENSOR2 26

// Blynk Auth Token
char auth[] = "PWGjZvkLXp8oqHEeYJR3YM2wgwqRd13z"; // Replace with your Blynk Auth Token
char ssid[] = "Anz"; // Replace with your WiFi SSID
char pass[] = "12345678"; // Replace with your WiFi password

// Durations in seconds
int redDuration = 30;
int yellowDuration = 2; // Duration for yellow light
int greenDuration = 30;

int sensor1State = 0;
int sensor2State = 0;

unsigned long previousMillis = 0;
unsigned long interval = 1000;

int counter = redDuration;
bool yellowTransition = false; // Flag to indicate yellow transition

// Blynk virtual pins
#define V0 V0 // Button for manual red EW
#define V1 V1 // Button for manual green EW
#define V2 V2 // Button for manual red S
#define V3 V3 // Button for manual green S 

void setup() {
  Serial.begin(115200);

  // Initialize traffic light pins
  pinMode(RED_EW, OUTPUT);
  pinMode(YELLOW_EW, OUTPUT);
  pinMode(GREEN_EW, OUTPUT);

  pinMode(RED_S, OUTPUT);
  pinMode(YELLOW_S, OUTPUT);
  pinMode(GREEN_S, OUTPUT);

  // Initialize sensor pins
  pinMode(SENSOR1, INPUT);
  pinMode(SENSOR2, INPUT);

  // Initialize display
  display.setBrightness(0x0f);

  // Set initial state of traffic lights
  digitalWrite(RED_EW, HIGH);
  digitalWrite(YELLOW_EW, LOW);
  digitalWrite(GREEN_EW, LOW);

  digitalWrite(RED_S, LOW);
  digitalWrite(YELLOW_S, LOW);
  digitalWrite(GREEN_S, HIGH);

  display.showNumberDec(counter);

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();

  unsigned long currentMillis = millis();

  // Check sensors
  sensor1State = digitalRead(SENSOR1);
  sensor2State = digitalRead(SENSOR2);

  // Adjust durations based on sensor input
  if (sensor1State == HIGH && sensor2State == HIGH) {
    redDuration = 20;
    greenDuration = 40;
  } else if (sensor1State == HIGH || sensor2State == HIGH) {
    redDuration = 25;
    greenDuration = 35;
  } else {
    redDuration = 30;
    greenDuration = 30;
  }

  // Update the countdown every second
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    counter--;

    if (counter < 0) {
      if (yellowTransition) {
        // End yellow transition
        yellowTransition = false;

        // Switch traffic light states after yellow light
        if (digitalRead(RED_EW) == HIGH) {
          // EW red -> green, S green -> red
          digitalWrite(YELLOW_EW, LOW);
          digitalWrite(RED_EW, LOW);
          digitalWrite(GREEN_EW, HIGH);
          digitalWrite(GREEN_S, LOW);
          digitalWrite(RED_S, HIGH);
          counter = greenDuration;
        } else {
          // EW green -> red, S red -> green
          digitalWrite(YELLOW_EW, LOW);
          digitalWrite(GREEN_EW, LOW);
          digitalWrite(RED_EW, HIGH);
          digitalWrite(RED_S, LOW);
          digitalWrite(GREEN_S, HIGH);
          counter = redDuration;
        }
      } else {
        // Start yellow transition
        yellowTransition = true;
        counter = yellowDuration;

        // Turn on yellow lights
        if (digitalRead(RED_EW) == HIGH) {
          digitalWrite(YELLOW_EW, HIGH);
        } else {
          digitalWrite(GREEN_EW, LOW);
          digitalWrite(YELLOW_EW, HIGH);
        }
      }
    }

    // Update display
    display.showNumberDec(counter);
  }

  // Update Blynk display
  Blynk.virtualWrite(V4, counter); // Counter display
}

// Blynk button handlers for manual control
BLYNK_WRITE(V0) {
  int value = param.asInt();
  digitalWrite(RED_EW, value);
}

BLYNK_WRITE(V1) {
  int value = param.asInt();
  digitalWrite(GREEN_EW, value);
}

BLYNK_WRITE(V2) {
  int value = param.asInt();
  digitalWrite(RED_S, value);
}

BLYNK_WRITE(V3) {
  int value = param.asInt();
  digitalWrite(GREEN_S, value);
}
