void setup() {
  Serial.begin(9600);  // Start serial communication with the Uno
  Serial.println("ESP8266 Ready to Receive Data");
}

void loop() {
  if (Serial.available()) {
    String dataPacket = Serial.readStringUntil('\n');  // Read the incoming data from the Uno
    Serial.print("Received Data: ");
    Serial.println(dataPacket);  // Print the received data to the Serial Monitor

    // Check if the data contains only numbers and commas (no other characters)
    if (dataPacket.indexOf(",") != -1 && dataPacket.indexOf("C") == -1 && dataPacket.indexOf("%") == -1) { 
      // Parse the data as floats
      float temperature, humidity, light, moisture;

      // Attempt to parse the incoming comma-separated data into floats
      int valuesRead = sscanf(dataPacket.c_str(), "%f,%f,%f,%f", &temperature, &humidity, &light, &moisture);
      
      // Check if the data was parsed successfully
      if (valuesRead == 4) {
        // If parsing is successful, print the parsed values to the Serial Monitor for debugging
        Serial.print("Parsed Data - Temperature: ");
        Serial.print(temperature);
        Serial.print(" C | Humidity: ");
        Serial.print(humidity);
        Serial.print(" % | Light: ");
        Serial.print(light);
        Serial.print(" % | Moisture: ");
        Serial.println(moisture);
      } else {
        Serial.println("Error parsing data.");
      }
    } else {
      Serial.println("Non-numeric or invalid data received, skipping.");
    }
  }
}
