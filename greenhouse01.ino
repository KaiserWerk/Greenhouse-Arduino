#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"

// PINs
#define DHT_PIN 2 // A
#define TRIG_PIN 9 // D
#define ECHO_PIN 8 // D

const char* ssid     = "DerKaiserKreis";
const char* password = "30134535545372737722";
const String apiKey = "XpgZASSfFwnSah9GuzQjZewUDIKrps";
const String baseUrl = "http://192.168.178.215:47336";

float filterArray[20];
float waterLevel = 0.0;

DHT dht(DHT_PIN, DHT22);

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();
}

void loop() {
  // Bring up the WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(10000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float distance = getWaterLevelDistance();
  float waterLevel = getWaterLevel(distance);
 
  
  Serial.print("Luftfeuchtigkeit: ");
  Serial.print(humidity); 
  Serial.println(" %");
  Serial.print("Temperatur: ");
  Serial.print(temperature);
  Serial.println(" Grad Celsius");
  Serial.print("Wasserstand: ");
  Serial.print(waterLevel); 
  Serial.println(" %");

  WiFiClient client;

  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, baseUrl + "/api/v1/receive")) {  // HTTP
    http.addHeader("X-Greenhouse-Key", apiKey);
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    char buffer[60];
    printf(buffer, "{\"temperature\":%f,\"humidity\":%f,\"waterlevel\":%f}", temperature, humidity, waterLevel);
    int httpCode = http.POST(String(buffer));

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.println("WiFi is down");
  delay(20000);

  WiFi.forceSleepWake();
  delay(1);
}

float getWaterLevel(float distance) {
  return map(distance, 3.0, 55.0, 100.0, 0.0);
}

float getWaterLevelDistance() {
  // 1. TAKING MULTIPLE MEASUREMENTS AND STORE IN AN ARRAY
  for (int sample = 0; sample < 20; sample++) {
    filterArray[sample] = ultrasonicMeasure();
    delay(30); // to avoid untrasonic interfering
  }

  // 2. SORTING THE ARRAY IN ASCENDING ORDER
  for (int i = 0; i < 19; i++) {
    for (int j = i + 1; j < 20; j++) {
      if (filterArray[i] > filterArray[j]) {
        float swap = filterArray[i];
        filterArray[i] = filterArray[j];
        filterArray[j] = swap;
      }
    }
  }

  // 3. FILTERING NOISE
  // + the five smallest samples are considered as noise -> ignore it
  // + the five biggest  samples are considered as noise -> ignore it
  // ----------------------------------------------------------------
  // => get average of the 10 middle samples (from 5th to 14th)
  double sum = 0;
  for (int sample = 5; sample < 15; sample++) {
    sum += filterArray[sample];
  }

  return sum / 10;
}

float ultrasonicMeasure() {
  // generate 10-microsecond pulse to TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // measure duration of pulse from ECHO pin
  float duration_us = pulseIn(ECHO_PIN, HIGH);

  // calculate the distance
  float distance_cm = 0.017 * duration_us;

  return distance_cm;
}
