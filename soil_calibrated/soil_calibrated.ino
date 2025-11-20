// Soil moisture sensor calibrated with real soil readings

const int SENSOR_PIN = A0;

// Final calibration values based on your measurements:
const int SOIL_DRY_VALUE = 847;   // dry soil (your measured average)
const int SOIL_WET_VALUE = 250;   // wet soil (your measured average)

void setup() {
  Serial.begin(9600);
  while (!Serial) {;}
  Serial.println("Soil Moisture Sensor - Final Soil Calibration");
  Serial.print("SOIL_DRY_VALUE = ");
  Serial.print(SOIL_DRY_VALUE);
  Serial.print(" , SOIL_WET_VALUE = ");
  Serial.println(SOIL_WET_VALUE);
}

void loop() {
  int rawValue = analogRead(SENSOR_PIN);

  // Map soil dry → wet into 0%–100%
  int moisturePercent = map(rawValue, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print("   Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println(" %");

  delay(500);
}
