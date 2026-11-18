#include <WiFi.h>
#include <Wire.h>
#include <KiwisIoT.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

// -------------------------------
// WIFI CONFIGURATION
// -------------------------------
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// -------------------------------
// KIWISIOT CONFIGURATION
// -------------------------------
const char* topic = "YOUR_PANEL_TOPIC_ID";

KiwisIoT kiwisiot(ssid, password, topic);

// -------------------------------
// SENSOR CONFIGURATION
// -------------------------------
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define RAIN_PIN 34

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp;
BH1750 lightMeter;

// Sensor status
bool bmpReady = false;
bool lightReady = false;

// Rain threshold - calibrate for your module
const int RAIN_THRESHOLD = 2000;

// Data upload interval
unsigned long previousMillis = 0;
const unsigned long interval = 5000;

// -------------------------------
// SETUP
// -------------------------------
void setup() {

  Serial.begin(115200);

  pinMode(RAIN_PIN, INPUT);

  // Start sensors
  dht.begin();

  Wire.begin(21, 22);

  // Detect BMP280
  bmpReady = bmp.begin(0x76);

  if (!bmpReady) {
    bmpReady = bmp.begin(0x77);
  }

  // Detect BH1750
  lightReady = lightMeter.begin();

  // Start KiwisIoT
  kiwisiot.begin();

  Serial.println("SMART FARM WEATHER STATION");
  Serial.println("ESP32 System Started");
}

// -------------------------------
// MAIN LOOP
// -------------------------------
void loop() {

  kiwisiot.run();

  if (millis() - previousMillis >= interval) {

    previousMillis = millis();

    // Read temperature
    float temperature = dht.readTemperature();

    // Read humidity
    float humidity = dht.readHumidity();

    // Read rain sensor
    int rainRaw = analogRead(RAIN_PIN);

    // Rain status (calibrate threshold)
    int rainStatus =
      (rainRaw < RAIN_THRESHOLD) ? 1 : 0;

    // Read atmospheric pressure
    float pressure = -1;

    if (bmpReady) {
      pressure = bmp.readPressure() / 100.0F;
    }

    // Read light intensity
    float light = -1;

    if (lightReady) {
      light = lightMeter.readLightLevel();
    }

    // Check DHT sensor
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("DHT SENSOR ERROR");
      return;
    }

    // ---------------------------
    // SERIAL MONITOR OUTPUT
    // ---------------------------

    Serial.println("========================");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print("Rain Sensor: ");
    Serial.println(rainStatus ? "RAIN DETECTED" : "NO RAIN");

    Serial.print("Rain Raw Value: ");
    Serial.println(rainRaw);

    Serial.print("Light Intensity: ");
    Serial.print(light);
    Serial.println(" lux");

    // ---------------------------
    // SEND DATA TO KIWISIOT
    // ---------------------------

    kiwisiot.send("0", temperature);
    kiwisiot.send("1", humidity);

    if (bmpReady) {
      kiwisiot.send("2", pressure);
    }

    kiwisiot.send("3", rainStatus);

    if (lightReady) {
      kiwisiot.send("4", light);
    }

    Serial.println("Data transmission requested");
    Serial.println("========================");
  }
}