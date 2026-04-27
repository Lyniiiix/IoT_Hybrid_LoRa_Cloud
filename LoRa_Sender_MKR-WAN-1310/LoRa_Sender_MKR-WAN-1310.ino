// ==========================================
// MKR WAN 1310 — LoRa P2P Sender
// ==========================================
// Capteur BME680 : Température + Humidité + Pression
// Envoie en LoRa P2P (868 MHz, SF12, BW125)
// Le M5Core reçoit et relaie vers Adafruit IO via WiFi
//
// Carte : Arduino MKR WAN 1310
// Librairies : MKRWAN, Adafruit BME680

#include <MKRWAN.h>
#include <Adafruit_BME680.h>

// ====================
// Capteur BME680 (I2C)
// ====================
Adafruit_BME680 bme;

// ====================
// LoRa Modem
// ====================
LoRaModem modem;

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 5000);
  delay(1000);
  Serial.println("--- MKR WAN 1310 STARTING ---");

  // --- Init modem LoRa (EU868) ---
  Serial.println("[DEB] Init Modem LoRa (EU868)...");
  if (!modem.begin(EU868)) {
    Serial.println("[ERREUR] Modem non détecté !");
    while (1);
  }
  Serial.println("[OK] Modem LoRa Prêt");
  
  Serial.print("DevEUI : ");
  Serial.println(modem.deviceEUI());
  delay(1000);

  // --- Init BME680 ---
  Serial.println("[DEB] Init BME680...");
  if (!bme.begin(0x76)) {
    Serial.println("Erreur BME680 ! Vérifiez câblage I2C.");
    // On ne bloque pas forcement si le BME manque, on peut quand meme tenter l'envoi
  } else {
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    Serial.println("[OK] BME680 Prêt");
  }

  // --- Configuration P2P ---
  modem.dataRate(0);        // SF12 BW125
  modem.setADR(false);
  modem.dutyCycle(false);

  Serial.println(">>> MKR PRÊT - Démarrage boucle d'envoi <<<");
}

void loop() {
  // --- Lecture BME680 ---
  if (!bme.performReading()) {
    Serial.println("Erreur lecture BME680 !");
    delay(5000);
    return;
  }

  float temperature = bme.temperature;
  float humidity    = bme.humidity;
  float pressure    = bme.pressure / 100.0; // Pa → hPa

  Serial.println("-------------------------------");
  Serial.print("Temp : "); Serial.print(temperature, 1); Serial.println(" C");
  Serial.print("Hum  : "); Serial.print(humidity, 1);    Serial.println(" %");
  Serial.print("Press: "); Serial.print(pressure, 1);    Serial.println(" hPa");

  // --- Payload texte (même format que Heltec + Pression en plus) ---
  // Format : "MKR|Temp=xx.x,Hum=yy.y,Press=zzz.z"
  // Préfixe "MKR|" pour que le M5Core sache la source
  char payload[48];
  snprintf(payload, sizeof(payload),
           "MKR|Temp=%.1f,Hum=%.1f,Press=%.1f",
           temperature, humidity, pressure);

  Serial.print("Envoi LoRa P2P : ");
  Serial.println(payload);

  // --- Envoi P2P sans join ---
  modem.beginPacket();
  modem.print(payload);
  int err = modem.endPacket(false); // false = non confirmé

  if (err > 0) {
    Serial.println("Paquet LoRa envoyé !");
  } else {
    Serial.println("Erreur envoi LoRa.");
  }

  // Pause 15 secondes (raisonnable pour duty cycle 868 MHz)
  delay(15000);
}