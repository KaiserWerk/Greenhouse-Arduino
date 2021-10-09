/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"

#ifndef STASSID
#define STASSID "DerKaiserKreis"
#define STAPSK  "30134535545372737722"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

DHT dht(2, DHT22);

void setup() {
  Serial.begin(115200);

  dht.begin();
}

void loop() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.println("WiFi is down");
  delay(20000);

  WiFi.forceSleepWake();
  delay(1);
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

  float humidity = dht.readHumidity(); //die Luftfeuchtigkeit auslesen und unter „Luftfeutchtigkeit“ speichern
  
  float temperature = dht.readTemperature();//die Temperatur auslesen und unter „Temperatur“ speichern
  
//  Serial.print("Luftfeuchtigkeit: "); //Im seriellen Monitor den Text und 
//  Serial.print(Luftfeuchtigkeit); //die Dazugehörigen Werte anzeigen
//  Serial.println(" %");
//  Serial.print("Temperatur: ");
//  Serial.print(Temperatur);
//  Serial.println(" Grad Celsius");

  WiFiClient client;

  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, "http://192.168.178.215:47336/api/v1/receive")) {  // HTTP


    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    char buffer[60];
    printf(buffer, "{\"temperature\":%f,\"humidity\":%f,\"waterlevel\":%d}", temperature, humidity, 1);
    int httpCode = http.POST(String(buffer));

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
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
}
