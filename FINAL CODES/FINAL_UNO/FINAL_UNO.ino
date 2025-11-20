#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>

#define SI7021_ADDR 0x40  // 7-bit I2C address of Si7021
#define BUTTON_PIN 6      // Pin for the push button
#define SENSOR_PIN A0     // Pin for the Soil Moisture Sensor

// Si7021 command codes
#define CMD_MEASURE_RH_WITHHOLD  0xE5  // Measure RH with Hold Master
#define CMD_MEASURE_TEMP_WITHHOLD 0xE3  // Measure Temperature with Hold Master

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

// Initialize the LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize the BH1750 light sensor
BH1750 lightMeter;

// Soil Moisture Sensor Calibration Values
const int SOIL_DRY_VALUE = 847;   // dry soil (your measured average)
const int SOIL_WET_VALUE = 250;   // wet soil (your measured average)

int screenState = 0;  // Track the current screen (0 = Temp/Humidity, 1 = Light, 2 = Soil Moisture)
unsigned long lastSensorUpdate = 0;  // Time of last sensor update
const unsigned long sensorUpdateInterval = 5000;  // 5 seconds delay between updates
unsigned long lastScreenChange = 0;  // Time of last screen change
const unsigned long screenCycleInterval = 15000;  // 15 seconds interval for automatic screen change
unsigned long lastDebounceTime = 0;  // To handle debouncing
bool lastButtonState = HIGH;  // Button is pulled up, so it starts HIGH

// Time for welcome screen
unsigned long welcomeStartTime = 0;
bool welcomeScreenShown = false;

// Variables for receiving commands from ESP8266
String receivedCommand = "";
bool commandReceived = false;

void setup() {
  Serial.begin(9600);  // Start serial communication with the ESP8266
  Wire.begin();  // Join I2C bus as master
  
  // Initialize LCD
  lcd.begin(16, 2);  // Initialize LCD with 16 columns and 2 rows
  lcd.backlight();  // Turn on the backlight
  
  // Initialize the BH1750 light sensor
  lightMeter.begin();
  
  // Reset the Si7021 sensor
  Wire.beginTransmission(SI7021_ADDR);
  Wire.write(0xFE);  // Reset command
  Wire.endTransmission();
  delay(15);  // Allow time for reset

  // Check if the Si7021 sensor is present
  if (checkSensor()) {
    Serial.println("Si7021 sensor found.");
  } else {
    Serial.println("Si7021 sensor not found.");
  }

  // Set the button pin as input with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

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
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);
  digitalWrite(relayPin3, HIGH);
  digitalWrite(relayPin4, HIGH);

  // Display welcome screen for 10 seconds
  welcomeStartTime = millis();
  welcomeScreenShown = true;
}

void loop() {
  // Check for incoming commands from ESP8266
  receiveCommandFromESP();
  
  // Process any received commands
  if (commandReceived) {
    processCommand(receivedCommand);
    commandReceived = false;
    receivedCommand = "";
  }

  // Read the current state of each button
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);
  buttonState3 = digitalRead(buttonPin3);

  // Button 1 - Toggle both fans
  if (buttonState1 == LOW && lastButtonState1 == HIGH && (millis() - lastDebounceTime) > 20) {  // 20ms debounce for faster response
    fanState = !fanState;
    digitalWrite(relayPin1, fanState ? LOW : HIGH);
    digitalWrite(relayPin2, fanState ? LOW : HIGH);
    Serial.println(fanState ? "Fans ON" : "Fans OFF");
    lastDebounceTime = millis();  // Update debounce time
  }

  // Button 2 - Toggle lights
  if (buttonState2 == LOW && lastButtonState2 == HIGH && (millis() - lastDebounceTime) > 20) {  // 20ms debounce for faster response
    lightsState = !lightsState;
    digitalWrite(relayPin3, lightsState ? LOW : HIGH);
    Serial.println(lightsState ? "Lights ON" : "Lights OFF");
    lastDebounceTime = millis();  // Update debounce time
  }

  // Button 3 - Toggle pump
  if (buttonState3 == LOW && lastButtonState3 == HIGH && (millis() - lastDebounceTime) > 20) {  // 20ms debounce for faster response
    pumpState = !pumpState;
    digitalWrite(relayPin4, pumpState ? LOW : HIGH);
    Serial.println(pumpState ? "Pump ON" : "Pump OFF");
    lastDebounceTime = millis();  // Update debounce time
  }

  // Save the current button state for the next loop
  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;
  lastButtonState3 = buttonState3;

  // Display the welcome screen if not already shown
  if (welcomeScreenShown) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WELCOME TO");
    lcd.setCursor(0, 1);
    lcd.print("FLORA-FUSION");

    if (millis() - welcomeStartTime > 10000) {  // 10 seconds delay for welcome screen
      welcomeScreenShown = false;
      lcd.clear();  // Clear the LCD after the welcome screen
    }
    return;  // Skip the rest of the loop to show the welcome screen
  }

  // Automatically switch screen every 15 seconds
  if (millis() - lastScreenChange >= screenCycleInterval) {
    screenState = (screenState + 1) % 3;  // Cycle between 0, 1, 2 for 3 screens
    lastScreenChange = millis();  // Update the last screen change timestamp
  }

  // Read data from sensors only if it's time for an update
  if (millis() - lastSensorUpdate >= sensorUpdateInterval) {
    float humidity = readHumidity();
    float temperature = readTemperature();
    int light = readLightIntensity();  // Read light as percentage
    int moisturePercent = readSoilMoisture();

    // Update the sensor data timestamp
    lastSensorUpdate = millis();

    // Display the current data depending on the screen state
    lcd.clear();  // Clear the LCD before updating
    switch (screenState) {
      case 0:  // Temperature and Humidity screen
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temperature, 1);
        lcd.print("C");
        
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity, 1);
        lcd.print("%");
        break;

      case 1:  // Light Intensity screen (now in percentage)
        lcd.setCursor(0, 0);
        lcd.print("Light: ");
        lcd.print(light);
        lcd.print(" %");
        break;

      case 2:  // Soil Moisture screen
        lcd.setCursor(0, 0);
        lcd.print("Soil Moisture:");
        lcd.setCursor(0, 1);
        lcd.print(moisturePercent);
        lcd.print(" %");
        break;
    }

    // Serial Monitor output showing all data at once
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" C | Humidity: ");
    Serial.print(humidity, 1);
    Serial.print(" % | Light: ");
    Serial.print(light);
    Serial.print(" % | Soil Moisture: ");
    Serial.println(moisturePercent);

    // Send the sensor data to ESP8266 via Serial (as a comma-separated string)
    String dataPacket = String(temperature) + "," + String(humidity) + "," + String(light) + "," + String(moisturePercent);
    Serial.println(dataPacket);  // Send the data packet to ESP8266
  }
}

// Function to receive commands from ESP8266
void receiveCommandFromESP() {
  if (Serial.available()) {
    String incoming = Serial.readStringUntil('\n');
    incoming.trim(); // Remove any extra whitespace or newlines
    
    // Check if this is a relay command (starts with "Relay")
    if (incoming.startsWith("Relay")) {
      receivedCommand = incoming;
      commandReceived = true;
      Serial.print("Command received: ");
      Serial.println(incoming);
    }
  }
}

// Function to process commands from ESP8266
void processCommand(String command) {
  if (command == "Relay Fan: ON") {
    fanState = true;
    digitalWrite(relayPin1, LOW);  // Turn ON fan 1
    digitalWrite(relayPin2, LOW);  // Turn ON fan 2
    Serial.println("Fans turned ON via ESP command");
  }
  else if (command == "Relay Fan: OFF") {
    fanState = false;
    digitalWrite(relayPin1, HIGH);  // Turn OFF fan 1
    digitalWrite(relayPin2, HIGH);  // Turn OFF fan 2
    Serial.println("Fans turned OFF via ESP command");
  }
  else if (command == "Relay Light: ON") {
    lightsState = true;
    digitalWrite(relayPin3, LOW);  // Turn ON lights
    Serial.println("Lights turned ON via ESP command");
  }
  else if (command == "Relay Light: OFF") {
    lightsState = false;
    digitalWrite(relayPin3, HIGH);  // Turn OFF lights
    Serial.println("Lights turned OFF via ESP command");
  }
  else if (command == "Relay Pump: ON") {
    pumpState = true;
    digitalWrite(relayPin4, LOW);  // Turn ON pump
    Serial.println("Pump turned ON via ESP command");
  }
  else if (command == "Relay Pump: OFF") {
    pumpState = false;
    digitalWrite(relayPin4, HIGH);  // Turn OFF pump
    Serial.println("Pump turned OFF via ESP command");
  }
}

// Check if sensor is present on I2C
bool checkSensor() {
  Wire.beginTransmission(SI7021_ADDR);
  return Wire.endTransmission() == 0;  // If no error, sensor is present
}

// Read raw data from the Si7021 sensor
uint16_t readRaw(uint8_t cmd) {
  Wire.beginTransmission(SI7021_ADDR);
  Wire.write(cmd);
  Wire.endTransmission();
  delay(20);  // Wait for measurement to complete

  Wire.requestFrom(SI7021_ADDR, (uint8_t)2);  // Request 2 bytes of data
  return Wire.available() == 2 ? (Wire.read() << 8) | Wire.read() : 0;
}

// Read humidity from the Si7021 sensor
float readHumidity() {
  uint16_t raw = readRaw(CMD_MEASURE_RH_WITHHOLD);
  return ((125.0 * raw) / 65536.0) - 6.0;  // Convert to percentage
}

// Read temperature from the Si7021 sensor
float readTemperature() {
  uint16_t raw = readRaw(CMD_MEASURE_TEMP_WITHHOLD);
  return ((175.72 * raw) / 65536.0) - 46.85;  // Convert to Celsius
}

// Read light intensity from the BH1750 sensor and convert to percentage
int readLightIntensity() {
  uint16_t lux = lightMeter.readLightLevel();  // Returns light level in lux

  // Map the lux value from 0-2000 to 0-100%
  int lightPercentage = map(lux, 0, 2000, 0, 100);
  lightPercentage = constrain(lightPercentage, 0, 100);  // Ensure it is within the 0-100% range

  return lightPercentage;
}

// Read soil moisture percentage
int readSoilMoisture() {
  int rawValue = analogRead(SENSOR_PIN);  // Read the raw value from the soil sensor

  // Map soil dry → wet into 0%–100%
  int moisturePercent = map(rawValue, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);  // Ensure it is within the 0-100% range
  return moisturePercent;
}