#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <credentials.h>

const char* mqtt_topic = "soil/moisture";

WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

// SSL Certificate (cannot find it)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"... Your HiveMQ Cloud CA Certificate ...\n" \
"-----END CERTIFICATE-----\n";

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to Wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection to HiveMQ Cloud...");

    // Create a client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect with username and password
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
} 

void setup() {
  Serial.begin(115200);
  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 10000) {  // Send data every 10 seconds
    lastMsg = now;
    
    // Read soil moisture
    int soilMoisture = analogRead(A0);
    
    // Convert to string and publish
    String msg = String(soilMoisture);
    client.publish(mqtt_topic, msg.c_str());
    
    Serial.print("Published soil moisture to HiveMQ Cloud: ");
    Serial.println(soilMoisture);
  }

}
