#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>

const char* ssid = "Metalab Secure";
const char* password = "********";
#define BROKER_ADDR IPAddress(10,20,30,97)
// #define SENSOR_PIN D3
#define SENSOR_PIN 25

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

HABinarySensor sensor("doorbell", "power", false);

unsigned alarmo = 0;
unsigned long lastReadAt = millis();
unsigned long lastRingAt = millis();

void setup() {
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Verbindung, YEAH!");

  device.setName("Doorbell Detector");
  device.setSoftwareVersion("1.0.0");
  sensor.setName("Doorbell");

  mqtt.begin(BROKER_ADDR);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED || !mqtt.isConnected()) {
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Verbindung, YEAH!");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    mqtt.begin(BROKER_ADDR);
  }

  mqtt.loop();

  if (millis() - lastReadAt > 1) { // 1ms interval
    if (digitalRead(SENSOR_PIN) == LOW) {
      if (alarmo < 20) {
        alarmo++;
      } else if (alarmo == 20) {
        sensor.setState(true);
        lastRingAt = millis();
      }
    } else if (alarmo > 0) {
      alarmo--;
    } else if (millis() - lastRingAt > 5000) {
      sensor.setState(false);
      alarmo = 0;
    }
    Serial.println(alarmo);
    lastReadAt = millis();
  }
}
