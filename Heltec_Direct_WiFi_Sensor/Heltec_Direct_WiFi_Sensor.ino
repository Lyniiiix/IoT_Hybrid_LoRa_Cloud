// ==========================================
// CODE POUR LA SOUTENANCE - HELTEC V3 DIRECT
// ==========================================
// Ce code fait exactement 3 choses :
// 1. Il lit le capteur bleu (DHT11) sur la Pin 2.
// 2. Il affiche la Température et l'Humidité en gros sur l'écran OLED.
// 3. Il se connecte au WiFi (iPhone) et envoie les données sur Adafruit IO.
// (Pas besoin de LoRa ici, c'est autonome !)

#include <WiFi.h>
#include <MQTTPubSubClient.h> // Librairie : ArduinoMqttClient
#include <U8g2lib.h>          // Librairie : U8g2 par Oliver Kraus
#include <DHT.h>              // Librairie : DHT sensor library par Adafruit

// --- 1. CREDENTIALS (voir arduino_secrets.h) ---
#include "arduino_secrets.h"

const char* ssid     = SECRET_WIFI_SSID;
const char* password = SECRET_WIFI_PASSWORD;

// --- 2. IDENTIFIANTS ADAFRUIT IO ---
#define IO_USERNAME SECRET_AIO_USERNAME
#define IO_KEY      SECRET_AIO_KEY
// Assure-toi que ces flux existent exactement avec ces noms sur Adafruit :
#define FEED_TEMP   SECRET_FEED_TEMP
#define FEED_HUM    SECRET_FEED_HUM

// --- 3. CONFIGURATION MATERIELLE ---
// Capteur DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Ecran OLED Heltec V3
// Clock: 18, Data: 17, Reset: 21
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 17, /* reset=*/ 21);

WiFiClient wifiClient;
MQTTPubSubClient mqttClient;

void setup() {
  Serial.begin(115200);
  
  // --- Allumage Physique de l'écran Heltec V3 ---
  // Activer le régulateur interne Vext (Pin 36 sur V3)
  pinMode(36, OUTPUT); 
  digitalWrite(36, LOW); 
  delay(100);

  // Reset manuel de l'écran (Pin 21)
  pinMode(21, OUTPUT); 
  digitalWrite(21, LOW); delay(10); 
  digitalWrite(21, HIGH); delay(50);

  // Initialisation écran et capteur
  u8g2.begin();
  dht.begin();

  // --- Connexion WiFi ---
  Serial.println("Connexion au WiFi...");
  WiFi.begin(ssid, password);
  
  // Affichage attente sur l'écran
  while (WiFi.status() != WL_CONNECTED) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    u8g2.drawStr(0, 30, "Connexion WiFi...");
    u8g2.sendBuffer();
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connecte !");

  // --- Connexion serveur Adafruit ---
  if (wifiClient.connect("io.adafruit.com", 1883)) {
    mqttClient.begin(wifiClient);
    if (mqttClient.connect("HeltecSoutenance", IO_USERNAME, IO_KEY)) {
      Serial.println("Adafruit Connecte !");
    }
  }
}

void loop() {
  mqttClient.update(); // Maintient la connexion avec Adafruit

  // 1. Lecture du capteur
  float temperature = dht.readTemperature();
  float humidite = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidite)) {
    
    // --- Affichage Console ---
    Serial.print("DHT11 | Temp: ");
    Serial.print(temperature, 1);
    Serial.print(" C | Hum: ");
    Serial.print(humidite, 1);
    Serial.println(" %");

    // 2. Affichage sur l'écran OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr); // Police "Gras" en taille 14
    
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature);
    u8g2.drawStr(25, 25, tempStr); // Centré en haut
    
    char humStr[16];
    snprintf(humStr, sizeof(humStr), "%.1f %%", humidite);
    u8g2.drawStr(25, 55, humStr); // Centré en bas
    
    u8g2.sendBuffer();

    // 3. Envoi vers Adafruit IO (Toutes les 10 secondes)
    static unsigned long chrono = 0;
    if (millis() - chrono > 10000) { 
      mqttClient.publish(FEED_TEMP, String(temperature, 1).c_str());
      mqttClient.publish(FEED_HUM, String(humidite, 1).c_str());
      chrono = millis();
      Serial.println("Donnees envoyees sur Adafruit !");
    }
    
  } else {
    Serial.println("Erreur: Impossible de lire le capteur DHT11 !");
  }

  delay(2000); // Petite pause avant de relire le capteur
}
