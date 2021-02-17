#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h> 
#include <PubSubClient.h> //Librairie pour la gestion Mqtt 

#define ssid  "Rasberry-Serre"    //wifi
#define password "ChangeMe"

#define mqtt_server  "10.3.141.1"//Adresse IP du Broker Mqtt
#define mqtt_Port 1883 //port utilisé par le Broker 

#define humsol "sensor/humsol1"      //topic mqtt du capteur d'humidité du sol
#define vanne_topic "bouton/vanne1" //topic mqtt de la vanne

#define tps_acquisition 60000

long tps=0;

ESP8266WiFiMulti WiFiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

#define temps 5000 //temps d'acquisition

#define grove A0      // pin du capteur
#define vanne D2      // pin de la vanne

#define debug true

void setup() {
  Serial.begin(9600);
  pinMode(vanne,OUTPUT);
  setup_wifi();
  setup_mqtt();
  
}
void loop() {
  reconnect();
  client.loop(); 
  
  
  
  if (millis()-tps>temps){
     tps=millis();
     
     
if ( debug ) {
     
      Serial.println(analogRead(grove));
}
     client.publish(humsol, String(analogRead(grove)).c_str(),true);
   }
   delay(tps_acquisition);
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
   else Serial.println("ben c'est nul ca marche pas");
   delay(temps);
 }
void reconnect(){
  while (!client.connected()) {
    Serial.println("Connection au serveur MQTT ...");
    if (client.connect("Vanne_1")) {
      Serial.println("MQTT connecté");
      client.subscribe(vanne_topic);        //souscription au topic l'électrovanne
      delay(temps);
    }
    else {
      Serial.print("echec, code erreur= ");
      Serial.println(client.state());
      Serial.println("nouvel essai dans 2s");
    delay(temps);
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
