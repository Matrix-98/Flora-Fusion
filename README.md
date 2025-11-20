FloraFusion - Smart Plant Monitoring System
============================================

PROJECT OVERVIEW
----------------
FloraFusion is an intelligent plant monitoring and automation system that provides 
real-time environmental monitoring and automated control of plant care systems.

Key Features:
- Real-time environmental monitoring (temperature, humidity, light, soil moisture)
- Cloud data logging and visualization via Adafruit IO
- Remote control via web dashboard
- Automated actuator control (fans, lights, water pump)
- Local data storage capabilities
- Multi-screen LCD display interface

HARDWARE COMPONENTS
-------------------

Microcontrollers:
- Arduino Uno (ATmega328P) - Primary controller
- ESP8266 WiFi Module - Cloud connectivity

Sensors:
- Temperature & Humidity: Si7021 (I2C)
- Light Intensity: BH1750 (I2C) 
- Soil Moisture: Capacitive Sensor (Analog)

Actuators:
- Cooling Fans (x2) - DC Fan via Relay
- Grow Lights - LED via Relay
- Water Pump - Submersible via Relay

Power Management:
- Input: 12V DC, 2A
- Voltage Regulation: 5V for logic, 12V for actuators

SOFTWARE REQUIREMENTS
---------------------

Arduino Libraries:
- Wire.h (I2C communication)
- LiquidCrystal_I2C.h (LCD display)
- BH1750.h (Light sensor)
- EEPROM.h (Optional data storage)
- SPI.h + SD.h (Optional SD card)

ESP8266 Libraries:
- ESP8266WiFi.h
- AdafruitIO_WiFi.h
- Adafruit_MQTT.h
- Adafruit_MQTT_Client.h

INSTALLATION & SETUP
--------------------

Hardware Connections:

Arduino Uno Pin Mapping:
A4 (SDA)    -> Si7021, BH1750, LCD (I2C Data)
A5 (SCL)    -> Si7021, BH1750, LCD (I2C Clock)
A0          -> Soil Moisture Sensor (Analog)
Pin 10      -> Relay 4 (Water Pump)
Pin 11      -> Relay 3 (Grow Lights) 
Pin 12      -> Relay 2 (Fan 2)
Pin 13      -> Relay 1 (Fan 1)
Pins 3-5    -> Push Buttons (Manual Control)
Pin 0 (RX)  -> ESP8266 TX
Pin 1 (TX)  -> ESP8266 RX

ESP8266 Connections:
RX   -> Arduino TX (Pin 1)
TX   -> Arduino RX (Pin 0)
VCC  -> 3.3V
GND  -> GND

Software Configuration:

1. Arduino IDE Setup:
   - Install required libraries
   - Select board: Arduino Uno
   - Set port: COMx (Windows) or /dev/ttyUSBx (Linux)

2. ESP8266 Configuration:
   - Update WiFi credentials in code
   - Set Adafruit IO username and key
   - Verify feed names match dashboard

3. Adafruit IO Setup:
   - Create account at io.adafruit.com
   - Create feeds with exact names:
     * temperature
     * humidity  
     * light-intensity
     * soil-moisture
     * relay-fan
     * relay-light
     * relay-pump

CODE STRUCTURE
--------------

Arduino Uno Main Functions:

void setup() {
  - Initialize serial communication (9600 baud)
  - Initialize I2C devices
  - Set up LCD display
  - Configure relay pins as outputs
  - Set up button inputs with pull-up resistors
  - Display welcome screen
}

void loop() {
  - Check manual button controls
  - Process commands from ESP8266
  - Read sensors every 5 seconds
  - Update LCD display (cycles every 15 seconds)
  - Send data to ESP8266
  - Log data to SD card (optional)
}

Key Functions:
- readTemperature(): Reads Si7021 temperature sensor
- readHumidity(): Reads Si7021 humidity sensor  
- readLightIntensity(): Reads BH1750 and converts to percentage
- readSoilMoisture(): Reads analog sensor and converts to percentage
- receiveCommandFromESP(): Listens for relay commands
- processCommand(): Executes relay control commands

ESP8266 Main Functions:

void setup() {
  - Connect to WiFi network
  - Initialize connection to Adafruit IO
  - Set up MQTT subscriptions for relay control
  - Initialize sensor data feeds
}

void loop() {
  - Maintain Adafruit IO connection
  - Check for incoming MQTT commands
  - Forward commands to Arduino via serial
  - Read sensor data from Arduino
  - Upload data to Adafruit IO every 10 seconds
}

DATA FLOW
---------

Sensor Data Flow:
1. Sensors read data every 5 seconds
2. Arduino processes and validates data
3. Data sent to ESP8266 as CSV: "temp,humidity,light,moisture"
4. ESP8266 publishes to Adafruit IO feeds
5. Real-time dashboard updates

Command Data Flow:
1. User toggles switch on Adafruit IO dashboard
2. MQTT message sent to ESP8266
3. ESP8266 forwards command to Arduino via serial
4. Arduino controls corresponding relay
5. Status feedback sent to cloud

DATA PACKET FORMATS
-------------------

Sensor Data (Arduino → ESP8266):
"25.5,60.2,75,45"
Format: "temperature,humidity,light,moisture"

Command Data (ESP8266 → Arduino):
"Relay Fan: ON"
"Relay Light: OFF" 
"Relay Pump: ON"

USE CASES
---------

1. Home Gardening Automation:
   - Automated light supplementation
   - Smart watering based on soil moisture
   - Temperature and humidity control

2. Greenhouse Management:
   - Multi-zone climate control
   - Resource optimization
   - Remote monitoring

3. Research Applications:
   - Precise environmental control
   - Data logging for experiments
   - Reproducible growing conditions

4. Vacation Plant Care:
   - Remote monitoring via mobile
   - Automated maintenance
   - Alert notifications

CALIBRATION GUIDE
-----------------

Soil Moisture Calibration:
1. Take reading in completely dry soil: SOIL_DRY_VALUE
2. Take reading in saturated soil: SOIL_WET_VALUE  
3. Update in code:
   const int SOIL_DRY_VALUE = 847;   // Your dry value
   const int SOIL_WET_VALUE = 250;   // Your wet value

Light Sensor Calibration:
- Adjust mapping in readLightIntensity():
  map(lux, 0, 2000, 0, 100) // 0-2000 lux to 0-100%

TROUBLESHOOTING
---------------

Common Issues:

1. Sensor Reading Errors:
   - Check I2C connections
   - Verify sensor addresses
   - Ensure proper power (3.3V for sensors)

2. WiFi Connection Issues:
   - Verify SSID and password
   - Check router settings
   - Monitor serial output for errors

3. Relay Control Problems:
   - Verify 12V power to relays
   - Check optocoupler isolation
   - Test with manual buttons first

4. Serial Communication:
   - Ensure baud rate matches (9600)
   - Check RX/TX crossover
   - Monitor both serial monitors

Debugging Commands:
- Add Serial.println() statements for debugging
- Use I2C scanner to detect devices
- Check WiFi.status() for connection issues

API DOCUMENTATION
-----------------

Adafruit IO Feeds:

Input Feeds (Sensor Data):
- temperature: float (Celsius)
- humidity: float (Percentage)
- light-intensity: integer (0-100%)
- soil-moisture: integer (0-100%)

Output Feeds (Relay Control):
- relay-fan: string ("ON"/"OFF")
- relay-light: string ("ON"/"OFF") 
- relay-pump: string ("ON"/"OFF")

Local Control:
- Button 1: Toggle both fans
- Button 2: Toggle grow lights
- Button 3: Toggle water pump

Display Modes:
- Screen 0: Temperature and Humidity
- Screen 1: Light Intensity  
- Screen 2: Soil Moisture

FUTURE ENHANCEMENTS
-------------------

Planned Features:
- Machine learning for predictive watering
- Mobile application with push notifications
- Solar power integration
- Additional sensors (pH, nutrients, CO2)
- Multi-zone support
- Voice control integration
- Camera integration for growth monitoring

EXPANSION CAPABILITIES:
- Additional actuators: CO2 injection, misting systems
- Weather data integration
- Distributed sensor networks
- Cloud database integration

SAFETY PRECAUTIONS
------------------

Electrical Safety:
- Use proper insulation for 12V connections
- Implement fuse protection for pump motor
- Use waterproof enclosures for outdoor components
- Ensure proper grounding

Water Safety:
- Use GFCI outlets near water sources
- Waterproof all electrical connections
- Regular inspection for leaks
- Pump safety shutoff mechanisms

Maintenance:
- Regular sensor calibration
- Periodic relay testing
- Software updates
- Component inspection

SUPPORT AND MAINTENANCE
-----------------------

Regular Maintenance Tasks:
- Monthly sensor calibration check
- Quarterly relay functionality test
- Software backup and updates
- Data log review and cleanup

Troubleshooting Resources:
- Serial monitor debugging
- Adafruit IO status page
- GitHub issue tracker
- Community forums

CONTACT INFORMATION
-------------------

Development Team: [Afm Abdur Rahman]
Repository: [[GitHub URL](https://github.com/Matrix-98/Flora-Fusion)]
Support: [Afmabdur2@gmail.com]

LICENSE
-------

This project is licensed under the MIT License.
See LICENSE file for details.

ACKNOWLEDGMENTS
---------------

- Adafruit for IO platform and libraries
- Arduino community for extensive documentation
- Open-source hardware manufacturers
- Contributors and testers

---
FloraFusion - Intelligent Plant Care System
Documentation Version: 1.0
Last Updated: [Current Date]
