/* Get data from MQTT and control LED
   Example receiving JSON: {"device": "red","trigger": "on"}
   by Yaser Ali Husen

   Youtube Channel: https://www.youtube.com/@YaserAliHusen/videos
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io"; //change with your MQTT broker

// GPIO untuk LED
const int red = 18;
const int yellow = 19;
const int green = 21; 

// MQTT Topic
const char* topic_pub = "yaser/ESP_Pub";
const char* topic_sub = "yaser/ESP_Sub";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  //WiFi.begin(ssid, password);
  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  // Konversi payload ke string
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Received message: ");
  Serial.println(message);

  // Jika pesan adalah "status", kirim status LED
  if (message == "status") {
    Serial.println("Received 'status' request. Sending status...");
    check_stat();
    return; // Keluar dari fungsi agar tidak parsing JSON
  }

  DynamicJsonDocument doc(200);
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  String device = doc["device"].as<String>();
  String trigger = doc["trigger"].as<String>();

  Serial.println("Turn " + trigger + " " + device);
  Serial.println("-----------------------");

  bool state = (trigger == "on");

  if (device == "red") digitalWrite(red, state);
  else if (device == "yellow") digitalWrite(yellow, state);
  else if (device == "green") digitalWrite(green, state);
  else if (device == "all") {
    digitalWrite(red, state);
    digitalWrite(yellow, state);
    digitalWrite(green, state);
  }

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected!");
      client.subscribe(topic_sub);
      //check_stat();
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds...");
      delay(5000);
    }
  }
}

void check_stat() {
  DynamicJsonDocument doc(300);
  JsonObject json = doc.to<JsonObject>();

  json["red"] = digitalRead(red);
  json["yellow"] = digitalRead(yellow);
  json["green"] = digitalRead(green);

  char JSONmessageBuffer[100];
  serializeJson(doc, JSONmessageBuffer, sizeof(JSONmessageBuffer));

  Serial.println("Sending MQTT message:");
  Serial.println(JSONmessageBuffer);

  if (client.publish(topic_pub, JSONmessageBuffer)) {
    Serial.println("Message sent successfully.");
  } else {
    Serial.println("Error sending message.");
  }
  Serial.println("-------------");
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.subscribe(topic_sub);

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);

  digitalWrite(red, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(green, LOW);
}

void loop() {
  delay(1000);
  if (!client.connected()) reconnect();
  client.loop();
}
