#include "DHT.h"

// PINs
#define DHT_PIN 7 // D
#define TRIG_PIN 9 // D
#define ECHO_PIN 8 // D

float filterArray[20];
//float waterLevel = 0.0;

DHT dht(DHT_PIN, DHT22);

void setup() {
  Serial.begin(9600);
  //pinMode(TRIG_PIN, OUTPUT);
  //pinMode(ECHO_PIN, INPUT);
  dht.begin();
}

void loop() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    humidity = -1.0;
  }
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    temperature = -1.0;
  }
  float distance = 34.8;// getWaterLevelDistance();
  if (isnan(distance)) {
    distance = -1.0;
  }
  float waterLevel = getWaterLevel(distance);
 
//  Serial.print("Luftfeuchtigkeit: ");
//  Serial.print(humidity); 
//  Serial.println("%");
//  Serial.print("Temperatur: ");
//  Serial.print(temperature);
//  Serial.println("°C");
//  Serial.print("Wasserstand: ");
//  Serial.print(waterLevel); 
//  Serial.println("%");

  Serial.print("{\"air_temperature\":");
  Serial.print(temperature);
  Serial.print(",\"humidity\":");
  Serial.print(humidity);
  Serial.print(",\"water_level\":");
  Serial.print(waterLevel);
  Serial.println("}");
  
  delay(20000);
}

float getWaterLevel(float distance) {
  return map(distance, 3.0, 55.0, 100.0, 0.0);
}

float getWaterLevelDistance() {
  for (int sample = 0; sample < 20; sample++) {
    filterArray[sample] = ultrasonicMeasure();
    delay(30); // to avoid untrasonic interfering
  }

  for (int i = 0; i < 19; i++) {
    for (int j = i + 1; j < 20; j++) {
      if (filterArray[i] > filterArray[j]) {
        float swap = filterArray[i];
        filterArray[i] = filterArray[j];
        filterArray[j] = swap;
      }
    }
  }

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
  return 0.017 * duration_us;
}
