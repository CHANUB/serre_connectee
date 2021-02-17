
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"          // Librairie des capteurs DHT


#define wifi_ssid "Rasberry-Serre"
#define wifi_password "ChangeMe"

#define mqtt_server "10.3.141.1"
#define mqtt_user "guest"  //s'il a été configuré sur Mosquitto
#define mqtt_password "guest" //idem

#define temperature_topic "sensor/tempint"  //Topic température
#define humidity_topic "sensor/humint"        //Topic humidité


//Buffer qui permet de décoder les messages MQTT reçus
char message_buff[100];

long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
long lastRecu = 0;
bool debug = true;  //Affiche sur la console si True

#define DHTPIN D4    // Pin sur lequel est branché le DHT
#define LED D2
// Dé-commentez la ligne qui correspond à votre capteur 

#define DHTTYPE DHT22         // DHT 22  

#define tps_acquisition 60000


//Création des objets
DHT dht(DHTPIN, DHTTYPE);     
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);     //Facultatif pour le debug
  
  
  pinMode(LED,OUTPUT);
  dht.begin();
       
  setup_wifi();           //On se connecte au réseau wifi
  client.setServer(mqtt_server, 1883);    //Configuration de la connexion au serveur MQTT
  client.setCallback(callback);  //La fonction de callback qui est executée à chaque réception de message   
  reconnect();
  
}

//Connexion au réseau WiFi
void setup_wifi() {
  delay(500);
  Serial.println();
  Serial.print("Connexion a ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi etablie ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}

//Reconnexion
void reconnect() {
  //Boucle jusqu'à obtenur une reconnexion
  while (!client.connected()) {
    //Serial.print("Connexion au serveur MQTT...");
    if (client.connect("DHT", mqtt_user, mqtt_password)) {
      
      Serial.print("KO, erreur : ");
      Serial.print(client.state());
      Serial.println(" On attend 2 secondes avant de reessayer");
      delay(2000);
    }
  }
}

void loop() {
 
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  //Envoi d'un message par minute
  if (now - lastMsg > 1000 * 10) {
    lastMsg = now;
    //Lecture de l'humidité ambiante
    float h = dht.readHumidity();
    // Lecture de la température en Celcius
    float t = dht.readTemperature();

    //Inutile d'aller plus loin si le capteur ne renvoie rien
   
    if ( isnan(t) || isnan(h)) {
      Serial.println("Echec de lecture ! Verifiez votre capteur DHT");
      return;
    }
    
  
    if ( debug ) {
      Serial.print("Temperature : ");
      Serial.print(t);
      Serial.print(" | Humidite : ");
      Serial.println(h);
    }  
    client.publish(temperature_topic, String(t).c_str(), true);   //Publie la température sur le topic temperature_topic
    client.publish(humidity_topic, String(h).c_str(), true);      //Et l'humidité
  }

  
}


void callback(char* topic, byte *payload, unsigned int length) {

  int i = 0;
  if ( debug ) {

  
Serial.println("-------Nouveau message du broker mqtt-----");
   Serial.print("Canal:");
   Serial.println(topic);
   Serial.print("donnee:");

   Serial.write(payload, length);
   Serial.println();
  
  if ((char)payload[0] == '1' ) {
    Serial.println("LED ON");
    digitalWrite(LED,HIGH); 
    Serial.println("ca marche?"); 
  } else if ((char)payload[0] == '0'){
     Serial.println("LED OFF");
     digitalWrite(LED,LOW); 
   }
   else {
    Serial.println("ben c'est nul ca marche pas");
   delay(5000);
  }
}
}
