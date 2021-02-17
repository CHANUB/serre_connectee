#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <DallasTemperature.h> // up
#include <OneWire.h> //up

#define oneWireBus D2 //up
Servo myservo; 
OneWire oneWire(oneWireBus); //up
DallasTemperature sensors(&oneWire); //up

// Paramètre WiFi, MQTT - WiFi, MQTT parameters
#define ssid  "Rasberry-Serre"    //wifi
#define password "ChangeMe"
#define mqtt_server  "10.3.141.1"//Adresse IP du Broker Mqtt
#define mqtt_port 1883 //port utilisé par le Broker
         
#define topic_moteur  "bouton/moteur"       // Topic MQTT pour le moteur
#define topic_temp "sensor/tempsonde1"

#define tps_acquisition 6000
int test = 1;
float temp; //up

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
 
  sensors.begin(); //up
  Serial.begin(9600);
  
  myservo.attach(D0);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
 String string;
 // Affiche le topic entrant - display incoming Topic
 Serial.print("Nouveau Message [");
 Serial.print(topic);
 Serial.print("] ");
 // décode le message - decode payload message
 for (int i = 0; i < length; i++) {
 string+=((char)payload[i]); 
 }
 // Affiche le message entrant - display incoming message
 Serial.print(string);
 
 // Conversion de la position en entier - convert position as an Integer
 int pos = string.toInt(); 
  Serial.print("Move the moteur to to ");

myservo.write(pos); 
 
 
 Serial.println(pos);
delay(15); 


 }
 



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Moteur")) {
      Serial.println("connected");
      client.subscribe(topic_moteur); 
   
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
  sensors.requestTemperatures(); //up
  temp = sensors.getTempCByIndex(0); //up
  client.publish(topic_temp, String(temp).c_str(), true); //up
  client.loop();
  Serial.printf("Message: %.2f \n", temp);
 delay(500);
}
