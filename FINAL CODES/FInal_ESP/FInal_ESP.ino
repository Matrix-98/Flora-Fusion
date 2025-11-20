#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>  // Adafruit IO Wi-Fi library
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#define IO_USERNAME    "Florafusion" // Use Your own username
#define IO_KEY         "aio_fNgV40OkZv0tv7JQl1eNJmShHSC3"      // use your api key
#define WIFI_SSID      "The Rat Network"          // enter your wifi 
#define WIFI_PASSWORD  "AR2001arr"               // enter your wifi password


// Adafruit IO client
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASSWORD);

// Create Adafruit IO feeds for each sensor
AdafruitIO_Feed *temperatureFeed;
AdafruitIO_Feed *humidityFeed;
AdafruitIO_Feed *lightFeed;
AdafruitIO_Feed *moistureFeed;

// Wi-Fi credentials for MQTT connection
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883  // Use 8883 for SSL
#define AIO_USERNAME    "Florafusion"  // Replace with your Adafruit IO Username
#define AIO_KEY         "aio_fNgV40OkZv0tv7JQl1eNJmShHSC3"  // Replace with your Adafruit IO Key

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup feeds for controlling LED and relays (used for testing purposes, no pin control here)
Adafruit_MQTT_Subscribe fanFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay Fan");
Adafruit_MQTT_Subscribe lightFeedRelay = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay Light");
Adafruit_MQTT_Subscribe pumpFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay Pump");

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

  // Setup MQTT subscriptions for Relay Fan, Relay Light, and Relay Pump feeds
  mqtt.subscribe(&fanFeed);
  mqtt.subscribe(&lightFeedRelay);
  mqtt.subscribe(&pumpFeed);
}

void loop() {
  io.run();  // Run the Adafruit IO loop

  // Ensure the connection to the MQTT server is alive
  MQTT_connect();

  // This is the 'wait for incoming subscription packets' busy subloop
  // Wait 2000 milliseconds while we wait for data from the subscription feed.
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(2000))) {
    if (subscription == &fanFeed) {
      char *value = (char *)fanFeed.lastread;
      if (String(value) == "ON") {
        Serial.println("Relay Fan: ON");
      }
      if (String(value) == "OFF") {
        Serial.println("Relay Fan: OFF");
      }
    }

    if (subscription == &lightFeedRelay) {
      char *value = (char *)lightFeedRelay.lastread;
      if (String(value) == "ON") {
        Serial.println("Relay Light: ON");
      }
      if (String(value) == "OFF") {
        Serial.println("Relay Light: OFF");
      }
    }

    if (subscription == &pumpFeed) {
      char *value = (char *)pumpFeed.lastread;
      if (String(value) == "ON") {
        Serial.println("Relay Pump: ON");
      }
      if (String(value) == "OFF") {
        Serial.println("Relay Pump: OFF");
      }
    }
  }

  // Read data from the Arduino Uno
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

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
