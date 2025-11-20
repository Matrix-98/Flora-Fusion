// Pin Definitions for Relays
const int relayPin1 = 13;     // Pin for fan 1
const int relayPin2 = 12;     // Pin for fan 2
const int relayPin3 = 11;     // Pin for lights
const int relayPin4 = 10;     // Pin for pump

// Pin Definitions for Buttons
const int buttonPin1 = 3;     // Button 1 for both fans
const int buttonPin2 = 4;     // Button 2 for lights
const int buttonPin3 = 5;     // Button 3 for pump

// Variables to track actuator states
bool fanState = false;        // Fan state (both fans controlled by one button)
bool lightsState = false;     // Lights state
bool pumpState = false;       // Pump state

// Variables for button press logic
int buttonState1 = 0;
int lastButtonState1 = 0;

int buttonState2 = 0;
int lastButtonState2 = 0;

int buttonState3 = 0;
int lastButtonState3 = 0;

void setup() {
  // Initialize relay pins as output
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);

  // Initialize the button pins as input with internal pull-up resistor
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);

  // Start with all actuators turned off
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, LOW);
  digitalWrite(relayPin3, LOW);
  digitalWrite(relayPin4, LOW);

  // Begin serial communication for debugging
  Serial.begin(9600);
}

void loop() {
  // Read the current state of each button
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);
  buttonState3 = digitalRead(buttonPin3);

  // Button 1 - Toggle both fans
  if (buttonState1 == LOW && lastButtonState1 == HIGH) {
    delay(50);  // Debounce delay
    fanState = !fanState;
    digitalWrite(relayPin1, fanState ? HIGH : LOW);
    digitalWrite(relayPin2, fanState ? HIGH : LOW);
    Serial.println(fanState ? "Fans ON" : "Fans OFF");
  }

  // Button 2 - Toggle lights
  if (buttonState2 == LOW && lastButtonState2 == HIGH) {
    delay(50);  // Debounce delay
    lightsState = !lightsState;
    digitalWrite(relayPin3, lightsState ? HIGH : LOW);
    Serial.println(lightsState ? "Lights ON" : "Lights OFF");
  }

  // Button 3 - Toggle pump
  if (buttonState3 == LOW && lastButtonState3 == HIGH) {
    delay(50);  // Debounce delay
    pumpState = !pumpState;
    digitalWrite(relayPin4, pumpState ? HIGH : LOW);
    Serial.println(pumpState ? "Pump ON" : "Pump OFF");
  }

  // Save the current button state for the next loop
  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;
  lastButtonState3 = buttonState3;
}
