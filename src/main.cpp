///////////////////////////////// Library /////////////////////////////////////////
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Adafruit_Sensor.h>

///////////////////////////////// Definition dan Password /////////////////////////////////////////
const char *ssid = "Mirza";
const char *password = "kucingmeong";
const char *thingsboardServer = "demo.thingsboard.io";
const char *token = "9T7XMXSxoF2RDizs6RiV";

const int DHT_PIN = 4;   
const int LIGHT_SENSOR_ADDRESS = 0x23;  
const int DHT_TYPE = DHT11;   
const int LIGHT_SENSOR_TYPE = BH1750::ONE_TIME_HIGH_RES_MODE;

DHT dht(DHT_PIN, DHT_TYPE);    
BH1750 lightMeter;            

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

///////////////////////////////// Setup /////////////////////////////////////////
void setup() {
  Serial.begin(9600);                
  Wire.begin(21, 22);                  
  dht.begin();                          
  lightMeter.begin();                   
  delay(100);                            

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  if (!tb.connect(thingsboardServer, token)) {
    Serial.println("Failed to connect to ThingsBoard");
    ESP.restart();
    while (1);
  }
}

///////////////////////////////// Loop /////////////////////////////////////////
void loop() {
  static unsigned long lastTempTime = 0;
  static unsigned long lastHumidityTime = 0;
  static unsigned long lastLightTime = 0;
  static bool isConnected = false; 
  static unsigned long lastConnectionAttempt = 0; 
  if (!isConnected) {
    if (millis() - lastConnectionAttempt >= 2000) {
      if (tb.connect(thingsboardServer, token)) {
        Serial.println("Connected to ThingsBoard");
        isConnected = true;
      } else {
        Serial.println("Failed to connect to ThingsBoard. Retrying...");
        lastConnectionAttempt = millis();
      }
    }
  }

  // Read temperature every 5 seconds
  if (millis() - lastTempTime >= 5000) {
    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
      tb.sendTelemetryFloat("temperature", temperature);
    } else {
      Serial.println("Error reading temperature!");
    }
    lastTempTime = millis();
  }

  // Read humidity every 5 seconds
  if (millis() - lastHumidityTime >= 5000) {
    float humidity = dht.readHumidity();
    if (!isnan(humidity)) {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
      tb.sendTelemetryFloat("humidity", humidity);
    } else {
      Serial.println("Error reading humidity!");
    }
    lastHumidityTime = millis();
  }

  // Read light intensity every 5 seconds
  if (millis() - lastLightTime >= 5000) {
    uint16_t lux = lightMeter.readLightLevel();
    if (!isnan(lux)) {
      Serial.print("Light: ");
      Serial.print(lux);
      Serial.println(" lux");
      tb.sendTelemetryFloat("light", lux);
    } else {
      Serial.println("Error reading light level!");
    }
    lastLightTime = millis();
  }

  // Process ThingsBoard events
  tb.loop();

  // Check if disconnected from ThingsBoard
  if (isConnected && !tb.connected()) {
    Serial.println("Disconnected from ThingsBoard. Reconnecting...");
    isConnected = false;
    lastConnectionAttempt = millis();
  }
}
