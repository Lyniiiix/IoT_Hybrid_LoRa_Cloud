
#include <WiFi.h>
#include <MQTTPubSubClient.h>
#include <U8g2lib.h>
#include <DHT.h>

// --- CREDENTIALS (voir arduino_secrets.h) ---
#include "arduino_secrets.h"

const char* ssid     = SECRET_WIFI_SSID;
const char* password = SECRET_WIFI_PASSWORD;

#define IO_USERNAME SECRET_AIO_USERNAME
#define IO_KEY      SECRET_AIO_KEY
#define FEED_TEMP   SECRET_FEED_LORA_TEMP
#define FEED_HUM    SECRET_FEED_LORA_HUM

// --- CAPTEUR DHT11 (Bleu) ---
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- ECRAN OLED (Heltec V3) ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 21, 18, 17);

WiFiClient wifiClient;
MQTTPubSubClient mqttClient;

void setup() {
  Serial.begin(115200);
  
  // Power ON Screen (VEXT)
  pinMode(21, OUTPUT); digitalWrite(21, LOW); 
  delay(10); digitalWrite(21, HIGH); delay(10);
  pinMode(45, OUTPUT); digitalWrite(45, LOW); 
  delay(100);

  u8g2.begin();
  dht.begin();

  Serial.println("Connexion WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    u8g2.drawStr(0, 30, "Connexion WiFi...");
    u8g2.sendBuffer();
    delay(500);
  }
  Serial.println("WiFi OK");

  if (wifiClient.connect("io.adafruit.com", 1883)) {
    mqttClient.begin(wifiClient);
    mqttClient.connect("HeltecSimple", IO_USERNAME, IO_KEY);
  }
}

void loop() {
  mqttClient.update();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    // ---- AFFICHAGE ECRAN ----
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr); // Gros texte
    
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f C", t);
    u8g2.drawStr(30, 25, tempStr); // Centré
    
    char humStr[16];
    snprintf(humStr, sizeof(humStr), "%.1f %%", h);
    u8g2.drawStr(30, 50, humStr); // Centré
    
    u8g2.sendBuffer();

    // ---- ENVOI ADAFRUIT ----
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 10000) { // Toutes les 10 secondes
      mqttClient.publish(FEED_TEMP, String(t, 1).c_str());
      mqttClient.publish(FEED_HUM, String(h, 1).c_str());
      lastUpdate = millis();
      Serial.println("Donnees envoyees !");
    }
  } else {
    Serial.println("Erreur DHT11 !");
  }

  delay(2000);
}
