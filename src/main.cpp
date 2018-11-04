#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define mqtt_server "10.72.16.1" //Fill in real mqtt ip

#define topicToSubscribe "vhome/patrick/light2" //MQTT Path -> Find in Icloud Notes

long randNumber;
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void turnOnRelay(){ //Sends A00101A2 to turn on Relais Board
  Serial.write(0xA0);
  Serial.write(0x01);
  Serial.write(0x01);
  Serial.write(0xA2);
  Serial.flush();
}
void turnOffRelay(){ //Sends A00101A1 to turn off Relais Board
  Serial.write(0xA0);
  Serial.write(0x01);
  Serial.write(0x00);
  Serial.write(0xA1);
  Serial.flush();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
      turnOnRelay();
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    turnOffRelay();
  }

}

void setup() {
Serial.begin(9600);
wifiManager.autoConnect("Setup-ESP","ESPRuleTheWorld"); //Implemented Wifi Manager
Serial.println("Starting AutoCOnnect");
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
randomSeed(analogRead(0));
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client" + randNumber)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(topicToSubscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
  reconnect();
}
client.loop();

long now = millis();
if (now - lastMsg > 2000) {
  client.publish("outTopic", msg);
}
}
