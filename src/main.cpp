#define SSID SSID
#define PASSWORD PASSWORD
#define MQTT_BROKER_ADDRESS 192.168.1.85
#define MQTT_BROKER_PORT 1883
#define MQTT_USER MQTT_USER
#define MQTT_PASSWORD MQTT_PASSWORD

#include <WiFi.h>
#include <PubSubClient.h>

// Network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// MQTT Broker details
const char* mqtt_server = "MQTT_BROKER_ADDRESS";
const int mqtt_port = MQTT_BROKER_PORT;
const char* mqtt_user = "MQTT_USER"; 
const char* mqtt_password = "MQTT_PASSWORD"; 

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsgTime = 0;
float gas_usage = 0.0;
int pulseCountSinceLastMsg = 0;
const int pulseInputPin = 32;

const unsigned long msgInterval = 3600000; 

void setup_wifi() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void IRAM_ATTR pulseCounter() { 
  pulseCountSinceLastMsg++;
}

void setup() {
  pinMode(pulseInputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pulseInputPin), pulseCounter, RISING);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsgTime > msgInterval) {
    // Calculate the gas usage based on pulses since the last message
    gas_usage += pulseCountSinceLastMsg * 0.01; // Assuming each pulse is 0.01 m^3 of gas usage

    char msg[50];
    snprintf(msg, 50, "%f m^3", gas_usage);
    client.publish("home/energy/gasUsage", msg);
    Serial.print("Published ");
    Serial.print(msg);
    Serial.println(" to MQTT broker.");

    // Reset pulse count and last message time
    pulseCountSinceLastMsg = 0;
    lastMsgTime = now;
  }
}
