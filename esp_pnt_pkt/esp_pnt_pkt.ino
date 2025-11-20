void setup() {
  Serial.begin(9600);  // Start serial communication with the Uno
  Serial.println("ESP8266 Ready to Receive Data");
}

void loop() {
  if (Serial.available()) {
    String dataPacket = Serial.readStringUntil('\n');  // Read the incoming data from the Uno
    Serial.print("Received Data: ");
    Serial.println(dataPacket);  // Print the received data to the Serial Monitor
  }
}
