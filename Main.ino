#include <ESP8266WiFi.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ESP8266HTTPClient.h>

#define WIFI_SSID "VODAFONE_H268Q-0396"
#define WIFI_PASS "19081908"
#define UBIDOTS_TOKEN "BBUS-2z7z42VFse6sgm1xVqhm6PyJFqJrki"
#define DEVICE_LABEL "Panagiotis1012"

PulseOximeter pox;
WiFiClient client;

unsigned long lastSend = 0;
const unsigned long sendInterval = 10000; // κάθε 10 δευτερόλεπτα

void setup() {
  Serial.begin(115200);
  Wire.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Σύνδεση WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  if (!pox.begin()) {
    Serial.println("Αισθητήρας δεν ξεκίνησε.");
    while (1);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  Serial.println("Αισθητήρας ΟΚ.");
}

void loop() {
  pox.update();
  float hr = pox.getHeartRate();
  float spo2 = pox.getSpO2();

  Serial.print("HR: ");
  Serial.print(hr);
  Serial.print(" bpm\tSpO2: ");
  Serial.println(spo2);

  if (millis() - lastSend > sendInterval && hr > 10 && spo2 > 30) {
    lastSend = millis();

    String payload = "{\"heart_rate\":";
    payload += String(hr, 2);
    payload += ",\"spo2\":";
    payload += String(spo2, 2);
    payload += "}";

    HTTPClient http;
    String url = "http://industrial.api.ubidots.com/api/v1.6/devices/" + String(DEVICE_LABEL);
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Auth-Token", UBIDOTS_TOKEN);

    int code = http.POST(payload);
    Serial.print("Ubidots Response Code: ");
    Serial.println(code);
    http.end();
  }

  delay(100);
} 
