#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h> 
#include <PubSubClient.h> //Librairie pour la gestion Mqtt 
#include <DallasTemperature.h> // up
#include <OneWire.h> //up

#define ssid  "Rasberry-Serre"    //wifi
#define password "ChangeMe"

#define mqtt_server  "10.3.141.1"//Adresse IP du Broker Mqtt
#define mqtt_Port 1883 //port utilisé par le Broker 

#define humsol "sensor/humsol2"      //topic mqtt du capteur d'humidité du sol
#define vanne_topic "bouton/vanne2" //topic mqtt de la vanne
#define topic_temp "sensor/tempsonde2" //topic mqtt de la sonde de température


long tps=0;
float temp; //up

ESP8266WiFiMulti WiFiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

#define temps 60000 //temps d'acquisition

#define grove A0      // pin du capteur
#define vanne D2      // pin de la vanne
#define oneWireBus D4 //up

OneWire oneWire(oneWireBus); //up
DallasTemperature sensors(&oneWire); //up

#define debug true

void setup() {
   sensors.begin(); //up
   
  Serial.begin(9600);
  pinMode(vanne,OUTPUT);
  setup_wifi();
  setup_mqtt();
  delay(temps);
}
void loop() {
  reconnect();

  
    
  client.loop(); 
  
  
  if (millis()-tps>temps){
     tps=millis();
     
     
if ( debug ) {
      Serial.print("température :");
  Serial.println(temp);
      Serial.println(analogRead(grove));
}

    sensors.requestTemperatures(); //up
    temp = sensors.getTempCByIndex(0); //up
 
    client.publish(topic_temp, String(temp).c_str(), true); //up
  
    client.publish(humsol, String(analogRead(grove)).c_str(),true);
   }
   
}
void setup_wifi(){
  
  WiFiMulti.addAP(ssid, password);//connexion au wifi
  
  while ( WiFiMulti.run() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  
  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.print("MAC : ");
  Serial.println(WiFi.macAddress());
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}

void setup_mqtt(){
  client.setServer(mqtt_server, mqtt_Port);
  client.setCallback(callback);//Déclaration de la fonction de souscription
  reconnect();
}

//Callback doit être présent pour souscrire a un topic et de prévoir une action 
void callback(char* topic, byte *payload, unsigned int length) {
  
   Serial.println("-------Nouveau message du broker mqtt-----");
   Serial.print("Canal:");
   Serial.println(topic);
   Serial.print("donnee:");
      
   Serial.write(payload, length);
   Serial.println();
   
   if ((char)payload[0] == '1') {
     Serial.println("vanne ouverte");
      digitalWrite(vanne,HIGH); 
   } else if ((char)payload[0] == '0'){
     Serial.println("vanne fermée");
     digitalWrite(vanne,LOW); 
   }
   else { Serial.println("ben c'est nul ca marche pas");
   digitalWrite(vanne,LOW);}
   delay(2000);
 }
void reconnect(){
  while (!client.connected()) {
    Serial.println("Connection au serveur MQTT ...");
    if (client.connect("Vanne_2")) {
      Serial.println("MQTT connecté");
      client.subscribe(vanne_topic);      //souscription au topic l'électrovanne
      delay(2000);
    }
    else {
      Serial.print("echec, code erreur= ");
      Serial.println(client.state());
      Serial.println("nouvel essai dans 2s");
    delay(2000);
    }
  }
  
}
//Fonction pour publier un float sur un topic 
void mqtt_publish(String topic, float t){
  char top[topic.length()+1];
  topic.toCharArray(top,topic.length()+1);
  char t_char[50];
  String t_str = String(t);
  t_str.toCharArray(t_char, t_str.length() + 1);
  client.publish(top,t_char);
}
