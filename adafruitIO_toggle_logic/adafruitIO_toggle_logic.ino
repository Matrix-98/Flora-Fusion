#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Wi-Fi credentials
#define WLAN_SSID       "The Rat Network"   // Replace with your Wi-Fi SSID
#define WLAN_PASS       "AR2001arr"        // Replace with your Wi-Fi Password

// Adafruit IO credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                                 // Use 8883 for SSL
#define AIO_USERNAME    "Florafusion"                        // Replace with your Adafruit IO Username
#define AIO_KEY         "aio_fNgV40OkZv0tv7JQl1eNJmShHSC3"   // Replace with your Adafruit IO Key

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup feeds for controlling LED and relays (used for testing purposes, no pin control here)
Adafruit_MQTT_Subscribe fanFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay Fan");
Adafruit_MQTT_Subscribe lightFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay Light");
Adafruit_MQTT_Subscribe pumpFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay Pump");

void MQTT_connect();

void setup() {
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());

  // Setup MQTT subscriptions for Relay Fan, Relay Light, and Relay Pump feeds
  mqtt.subscribe(&fanFeed);
  mqtt.subscribe(&lightFeed);
  mqtt.subscribe(&pumpFeed);
}

void loop() {
  // Ensure the connection to the MQTT server is alive
  MQTT_connect();

  // This is the 'wait for incoming subscription packets' busy subloop
  // Wait 2000 milliseconds while we wait for data from the subscription feed.
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(2000))) {
    if (subscription == &fanFeed) {
      //Serial.print(F("Got Relay Fan: "));
      //Serial.println((char *)fanFeed.lastread);
      char *value = (char *)fanFeed.lastread;

      // Check feed value and print to Serial Monitor
      if (String(value) == "ON") {
        Serial.println("Relay Fan: ON");
      }
      if (String(value) == "OFF") {
        Serial.println("Relay Fan: OFF");
      } else {
        //Serial.println("Relay Fan: Unknown value");
      }
    }

    if (subscription == &lightFeed) {
      //Serial.print(F("Got Relay Light: "));
      //Serial.println((char *)lightFeed.lastread);
      char *value = (char *)lightFeed.lastread;

      // Check feed value and print to Serial Monitor
      if (String(value) == "ON") {
        Serial.println("Relay Light: ON");
      }
      if (String(value) == "OFF") {
        Serial.println("Relay Light: OFF");
      } else {
        //Serial.println("Relay Light: Unknown value");
      }
    }

    if (subscription == &pumpFeed) {
      //Serial.print(F("Got Relay Pump: "));
      ///Serial.println((char *)pumpFeed.lastread);
      char *value = (char *)pumpFeed.lastread;

      // Check feed value and print to Serial Monitor
      if (String(value) == "ON") {
        Serial.println("Relay Pump: ON");
      }
      if (String(value) == "OFF") {
        Serial.println("Relay Pump: OFF");
      } else {
        //Serial.println("Relay Pump: Unknown value");
      }
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
