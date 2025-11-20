#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>

#define SI7021_ADDR 0x40  // 7-bit I2C address of Si7021
#define BUTTON_PIN 6      // Pin for the push button
#define SENSOR_PIN A0     // Pin for the Soil Moisture Sensor

// Si7021 command codes
#define CMD_MEASURE_RH_WITHHOLD  0xE5  // Measure RH with Hold Master
#define CMD_MEASURE_TEMP_WITHHOLD 0xE3  // Measure Temperature with Hold Master

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

  // Display welcome screen for 10 seconds
  welcomeStartTime = millis();
  welcomeScreenShown = true;
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);  // Declare buttonState only once

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

  // Check for a button press (with debounce)
  if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > 20) {  // 20ms debounce for faster response
    screenState = (screenState + 1) % 3;  // Toggle between 3 screens (0, 1, 2)
    lastDebounceTime = millis();  // Update debounce time
    
    // Serial output for button press
    Serial.print("Button pressed, switching to Screen ");
    Serial.println(screenState);  // Print the current screen number
  }

  lastButtonState = buttonState;  // Save the button state for the next loop

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

  // No delays in loop, ensuring instant button responsiveness
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
