#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WLAN_SSID "SSID"
#define WLAN_PASS "PWD"

#define mqtt_server "0.0.0.0" //Fill in real mqtt ip

#define topicToSubscribe "vhome/patrick/light2" //MQTT Path -> Find in Icloud Notes

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

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
Serial.begin(9600);
setup_wifi();
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
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
