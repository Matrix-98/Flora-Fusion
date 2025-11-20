#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>

#define SI7021_ADDR 0x40  // 7-bit I2C address of Si7021

// Si7021 command codes
#define CMD_MEASURE_RH_WITHHOLD  0xE5  // Measure RH with Hold Master
#define CMD_MEASURE_TEMP_WITHHOLD 0xE3  // Measure Temperature with Hold Master

// Initialize the LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize the BH1750 light sensor
BH1750 lightMeter;

void setup() {
  Serial.begin(9600);
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
}

void loop() {
  // Read data from sensors
  float humidity = readHumidity();
  float temperature = readTemperature();
  uint16_t light = readLightIntensity();

  // Display the readings on LCD for 3 seconds
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 1);
  lcd.print("%");

  // Serial Monitor output
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.print(" C | Humidity: ");
  Serial.print(humidity, 1);
  Serial.print(" % | Light: ");
  Serial.print(light);
  Serial.println(" lux");

  delay(3000);  // Wait for 3 seconds before updating the display
  
  // Update the LCD with light intensity for 3 seconds
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light: ");
  lcd.print(light);
  lcd.print(" lux");

  // Serial Monitor output for light intensity
  Serial.print("Light: ");
  Serial.print(light);
  Serial.println(" lux");

  delay(3000);  // Wait for 3 seconds before the next loop
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

// Read light intensity from the BH1750 sensor
uint16_t readLightIntensity() {
  return lightMeter.readLightLevel();  // Returns light level in lux
}
