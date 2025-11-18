#include <Wire.h>

#define SI7021_ADDR 0x40  // 7-bit I2C address of Si7021

// Si7021 command codes
#define CMD_MEASURE_RH_WITHHOLD  0xE5  // Measure RH with Hold Master
#define CMD_MEASURE_TEMP_WITHHOLD 0xE3  // Measure Temperature with Hold Master

void setup() {
  Serial.begin(9600);
  Wire.begin();  // Join I2C bus as master
  
  // Reset the sensor
  Wire.beginTransmission(SI7021_ADDR);
  Wire.write(0xFE);  // Reset command
  Wire.endTransmission();
  delay(15);  // Allow time for reset

  // Check if the sensor is detected on I2C
  if (checkSensor()) {
    Serial.println("Si7021 sensor found.");
  } else {
    Serial.println("Si7021 sensor not found.");
  }
}

void loop() {
  // Read and display the humidity and temperature
  Serial.print("Humidity: ");
  Serial.print(readHumidity(), 2);
  Serial.println(" %RH");

  Serial.print("Temperature: ");
  Serial.print(readTemperature(), 2);
  Serial.println(" °C");

  delay(2000);  // Wait for 2 seconds before the next read
}

// Check if sensor is present on I2C
bool checkSensor() {
  Wire.beginTransmission(SI7021_ADDR);
  return Wire.endTransmission() == 0;  // If no error, sensor is present
}

// Read raw data from the sensor
uint16_t readRaw(uint8_t cmd) {
  Wire.beginTransmission(SI7021_ADDR);
  Wire.write(cmd);
  Wire.endTransmission();
  delay(20);  // Wait for measurement to complete

  Wire.requestFrom(SI7021_ADDR, (uint8_t)2);  // Request 2 bytes of data
  return Wire.available() == 2 ? (Wire.read() << 8) | Wire.read() : 0;
}

// Read humidity from the sensor
float readHumidity() {
  uint16_t raw = readRaw(CMD_MEASURE_RH_WITHHOLD);
  return ((125.0 * raw) / 65536.0) - 6.0;  // Convert to percentage
}

// Read temperature from the sensor
float readTemperature() {
  uint16_t raw = readRaw(CMD_MEASURE_TEMP_WITHHOLD);
  return ((175.72 * raw) / 65536.0) - 46.85;  // Convert to Celsius
}
