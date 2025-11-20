#include <AdafruitIO_WiFi.h>
#include <ESP8266WiFi.h>  // ESP8266 WiFi library

#define IO_USERNAME    "Florafusion"
#define IO_KEY         "aio_fNgV40OkZv0tv7JQl1eNJmShHSC3"
#define WIFI_SSID      "The Rat Network"
#define WIFI_PASSWORD  "AR2001arr"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASSWORD);  // Connect to Adafruit IO

// Create Adafruit IO feeds for each sensor
AdafruitIO_Feed *temperatureFeed;
AdafruitIO_Feed *humidityFeed;
AdafruitIO_Feed *lightFeed;
AdafruitIO_Feed *moistureFeed;

void setup() {
  Serial.begin(9600);  // Start serial communication with the Arduino Uno
  Serial.println("ESP8266 Ready to Receive Data");

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to Adafruit IO
  io.connect();

  // Debugging: print Adafruit IO connection status
  while (io.status() != AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to Adafruit IO!");

  // Initialize the feeds for each sensor
  temperatureFeed = io.feed("temperature");  // Ensure the feed names match your Adafruit IO setup
  humidityFeed = io.feed("humidity");
  lightFeed = io.feed("Light Intensity");  // Corrected feed name
  moistureFeed = io.feed("Soil moisture"); // Corrected feed name
}

void loop() {
  io.run();  // Run the Adafruit IO loop

  if (Serial.available()) {
    String dataPacket = Serial.readStringUntil('\n');  // Read the incoming data from the Uno
    Serial.print("Received Data: ");
    Serial.println(dataPacket);  // Print the received data to the Serial Monitor

    // Parse the data as floats
    float temperature, humidity, light, moisture;

    // Attempt to parse the incoming comma-separated data into floats
    int valuesRead = sscanf(dataPacket.c_str(), "%f,%f,%f,%f", &temperature, &humidity, &light, &moisture);

    if (valuesRead == 4) {
      unsigned long currentMillis = millis();

      // Send data every 10 seconds (7 times per minute)
      static unsigned long lastSendTime = 0;
      if (currentMillis - lastSendTime >= 10000) {  // 10-second interval
        // Send sensor data to Adafruit IO
        Serial.println("Sending sensor data to Adafruit IO...");

        // Upload the parsed data to Adafruit IO feeds
        temperatureFeed->save(temperature);
        humidityFeed->save(humidity);
        lightFeed->save(light);
        moistureFeed->save(moisture);

        // Print to serial for debugging
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" C | Humidity: ");
        Serial.print(humidity);
        Serial.print(" % | Light: ");
        Serial.print(light);
        Serial.print(" % | Moisture: ");
        Serial.println(moisture);

        // Update the last send time
        lastSendTime = currentMillis;
      }
    } else {
      Serial.println("Error parsing data.");
    }
  }
}
