#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 5
#define DHTTYPE AM2301

DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 1, 1);
IPAddress local_IP(192, 168, 1, 101);

void setup() {
  Serial.begin(9600);
  dht.begin();
  setupWIFI();
  setupMQTT();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.flush();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[✗] Disconnected from WiFi");
    while(WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      delay(2000);
    }
    return;
  } else if (!mqttClient.loop()) {
    Serial.println("[✗] Disconnected from MQTT server");
    while(!mqttClient.connected()) {
      mqttClient.connect("ESP8266Client", "pi", "raspberry");
      delay(2000);
    }
    return;
  } else if (isnan(h) || isnan(t)) {
    Serial.println("[✗] Failed to retrieve data from DHT22 device");
    delay(2000);
    return;
  }
  
  String dataT = "[!] Temperature: " + String(t) + "*C", dataH = "[!] Humidity: " + String(h) + "%";
  Serial.println(dataT);
  Serial.println(dataH);

  Serial.println("[.] Sending message to MQTT server");
  String message = "{\"temperature\":\"" + String(t) + "\",\"humidity\":\"" + String(h) + "\"}";
  if (!mqttClient.publish("ESIEA/grp2", message.c_str(), true)) {
    Serial.println("[✗] Failed to publish a message to MQTT server");
    delay(2000);
    return;
  }
  
  delay(2000);
}

void setupWIFI() {
  // Wifi configuration
  delay(3000);
  Serial.print("[.] Try to connect to Raspberry via Wifi");
  WiFi.disconnect();
  WiFi.begin("TheLabIOT", "Yaay!ICanTalkNow");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.config(local_IP, gateway, subnet);
  Serial.print("\n[✓] Connected, IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}
void setupMQTT() {
  // MQTT configuration
  Serial.print("[.] Try to connect to MQTT server");
  mqttClient.setServer("192.168.1.4", 1883);
  while (!mqttClient.connected()) {
    mqttClient.connect("esp8266", "pi", "raspberry");
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n[✓] Connected to MQTT server");
}
