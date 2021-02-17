
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"          // Librairie des capteurs DHT


#define wifi_ssid "Rasberry-Serre"
#define wifi_password "ChangeMe"

#define mqtt_server "10.3.141.1"
#define mqtt_user "guest"  //s'il a été configuré sur Mosquitto
#define mqtt_password "guest" //idem

#define temperature_topic "sensor/tempext"  //Topic température
#define humidity_topic "sensor/humext"        //Topic humidité
#define light_topic "sensor/light"  // topic luminosité

//Buffer qui permet de décoder les messages MQTT reçus
char message_buff[100];

long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
long lastRecu = 0;
bool debug = true;  //Affiche sur la console si True

#define DHTPIN D4    // Pin sur lequel est branché le DHT
#define lightpin A0


#define DHTTYPE DHT22         // DHT 22  




//Création des objets
DHT dht(DHTPIN, DHTTYPE);     
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);     
 
  pinMode(lightpin,OUTPUT);     
  setup_wifi();           //On se connecte au réseau wifi
  client.setServer(mqtt_server, 1883);    //Configuration de la connexion au serveur MQTT
   
  dht.begin();
}

//Connexion au réseau WiFi
void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Connexion a ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connexion WiFi etablie ");
  Serial.print("=> Addresse IP : ");
  Serial.println(WiFi.localIP());
}

//Reconnexion
void reconnect() {
  //Boucle jusqu'à obtenur une reconnexion
  while (!client.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
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
      Serial.println("Echec de lecture ! Verifiez le capteur ");
      return;
    }
    
  
    if ( debug ) {
      Serial.print("Temperature : ");
      Serial.print(t);
      Serial.print(" | Humidite : ");
      Serial.println(h);
      Serial.println(analogRead(lightpin)); // Renvoi les valeurs de la photorésistance
    }  
    client.publish(temperature_topic, String(t).c_str(), true);   //Publie la température sur le topic temperature_topic
    client.publish(humidity_topic, String(h).c_str(), true);      //Et l'humidité
    client.publish(light_topic, String(analogRead(lightpin)).c_str(),true); //et la lumière?
  }
delay(10000);
}
