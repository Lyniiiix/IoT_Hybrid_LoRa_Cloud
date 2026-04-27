// ==========================================
// M5Core — LoRa P2P Receiver (SAFE VERSION)
// VERSION : 2.0 - Stabilisée & Blindée 🛡️
// ==========================================
// - Filtre les paquets corrompus
// - Empêche la saturation graphique
// - Gère les crashs de parsing
//
// Librairies : M5Unified, LoRa, PubSubClient, WebServer

#include <M5Unified.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// -----------------------
// Configuration Matérielle
// -----------------------
#define CS_PIN    5
#define RST_PIN   13
#define IRQ_PIN   34
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_SCLK 18

const char* ssid     = SECRET_WIFI_SSID;
const char* password = SECRET_WIFI_PASSWORD;
#define IO_USERNAME SECRET_AIO_USERNAME
#define IO_KEY      SECRET_AIO_KEY
#define FEED_TEMP   "lyniiiix/feeds/lora-temperature"
#define FEED_HUM    "lyniiiix/feeds/lora-humidite"
#define FEED_PRESS  "lyniiiix/feeds/lora-pression"
#define FEED_RSSI   "lyniiiix/feeds/lora-rssi"
#define FEED_SNR    "lyniiiix/feeds/lora-snr"

WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);
WebServer    server(80);

// Couleurs
#define COL_BG      0x000F
#define COL_ACCENT  0x73FF
#define COL_TEXT    0xFFFF
#define COL_OK      0x07E0
#define COL_ERR     0xF800

// -----------------------
// Variables d'état
// -----------------------
float  g_temp = NAN, g_hum = NAN, g_press = NAN;
int    g_rssi = 0;
float  g_snr  = 0.0;
int    g_rssi_min = 0, g_rssi_max = -200;
long   g_rssi_sum = 0;
int    g_pkts = 0, g_pkts_heltec = 0, g_pkts_mkr = 0;
String g_source = "none";

uint32_t lastDisplayUpdate = 0;
#define DISPLAY_DEBOUNCE_MS 1000 // Raffraîchir max 1x par seconde

// -----------------------
// Affichage Sécurisé
// -----------------------
void drawUI() {
    M5.Display.fillScreen(COL_BG);
    M5.Display.setTextColor(COL_ACCENT);
    M5.Display.setFont(&fonts::FreeSansBold12pt7b);
    M5.Display.drawCenterString("LoRa GATEWAY", 160, 10);
    M5.Display.drawFastHLine(0, 40, 320, COL_ACCENT);
    
    M5.Display.setFont(&fonts::FreeSans9pt7b);
    M5.Display.setTextColor(COL_TEXT);
    M5.Display.setCursor(10, 55);
    M5.Display.print("IP : ");
    M5.Display.setTextColor(COL_OK);
    M5.Display.println(WiFi.localIP().toString());
    
    M5.Display.setTextColor(COL_TEXT);
    M5.Display.setCursor(10, 75);
    M5.Display.print("MQTT : ");
    M5.Display.setTextColor(mqttClient.connected() ? COL_OK : COL_ERR);
    M5.Display.println(mqttClient.connected() ? "ON" : "OFF");
}

void updateDisplayData() {
    uint32_t now = millis();
    if (now - lastDisplayUpdate < DISPLAY_DEBOUNCE_MS) return;
    lastDisplayUpdate = now;

    M5.Display.fillRect(0, 100, 320, 140, COL_BG);
    M5.Display.drawFastHLine(0, 100, 320, 0x3186);

    M5.Display.setFont(&fonts::FreeSansBold12pt7b);
    M5.Display.setTextColor(COL_ACCENT);
    M5.Display.setCursor(10, 130);
    M5.Display.print("Src : ");
    M5.Display.println(g_source);

    M5.Display.setFont(&fonts::FreeSans12pt7b);
    M5.Display.setTextColor(COL_TEXT);
    M5.Display.setCursor(20, 160);
    M5.Display.printf("%.1f C | %.1f %%", g_temp, g_hum);
    
    if (!isnan(g_press)) {
        M5.Display.setCursor(20, 185);
        M5.Display.printf("P: %.1f hPa", g_press);
    }

    M5.Display.setFont(&fonts::FreeSans9pt7b);
    M5.Display.setTextColor(0xBDD7);
    M5.Display.setCursor(10, 220);
    M5.Display.printf("RSSI: %d  SNR: %.1f", g_rssi, g_snr);
    M5.Display.setCursor(10, 235);
    M5.Display.printf("Pkts: H=%d M=%d", g_pkts_heltec, g_pkts_mkr);
}

// -----------------------
// Connexions
// -----------------------
void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) { delay(100); }
    if (WiFi.status() == WL_CONNECTED) drawUI();
}

void connectMQTT() {
    if (mqttClient.connected()) return;
    mqttClient.setServer("io.adafruit.com", 1883);
    if (mqttClient.connect("M5CoreGateway", IO_USERNAME, IO_KEY)) {
        drawUI();
    }
}

void handleData() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    int avg = (g_pkts > 0) ? (int)(g_rssi_sum / g_pkts) : 0;
    String json = "{";
    json += "\"temp\":"      + (isnan(g_temp)  ? String("null") : String(g_temp, 1)) + ",";
    json += "\"hum\":"       + (isnan(g_hum)   ? String("null") : String(g_hum, 1)) + ",";
    json += "\"press\":"     + (isnan(g_press) ? String("null") : String(g_press, 1)) + ",";
    json += "\"source\":\""  + g_source + "\",";
    json += "\"rssi\":"      + String(g_rssi) + ",";
    json += "\"snr\":"       + String(g_snr, 1) + ",";
    json += "\"rssi_min\":"  + String(g_rssi_min) + ",";
    json += "\"rssi_max\":"  + String(g_rssi_max) + ",";
    json += "\"rssi_avg\":"  + String(avg) + ",";
    json += "\"packets\":"   + String(g_pkts) + ",";
    json += "\"pkt_heltec\":" + String(g_pkts_heltec) + ",";
    json += "\"pkt_mkr\":"   + String(g_pkts_mkr);
    json += "}";
    server.send(200, "application/json", json);
}

// -----------------------
// Setup
// -----------------------
void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Speaker.setVolume(0);
    M5.Speaker.end();

    M5.Display.setBrightness(80);
    M5.Display.fillScreen(COL_BG);
    M5.Display.drawCenterString("SAFE BOOT...", 160, 100);

    Serial.begin(115200);

    SPI.begin(LORA_SCLK, LORA_MISO, LORA_MOSI, -1);
    LoRa.setPins(CS_PIN, RST_PIN, IRQ_PIN);
    if (!LoRa.begin(868E6)) {
        Serial.println("LoRa FAIL");
    } else {
        LoRa.setSpreadingFactor(12);
        LoRa.setSignalBandwidth(125E3);
        Serial.println("LoRa READY");
    }

    connectWiFi();
    server.on("/data", handleData);
    server.begin();
    connectMQTT();
}

// -----------------------
// Loop (Blitée & Sécurisée)
// -----------------------
void loop() {
    M5.update();
    
    // Checks périodiques 
    static uint32_t lastCheck = 0;
    if (millis() - lastCheck > 10000) {
        lastCheck = millis();
        if (WiFi.status() != WL_CONNECTED) connectWiFi();
        if (!mqttClient.connected()) connectMQTT();
    }
    
    mqttClient.loop();
    server.handleClient();

    // Réception LoRa sécurisée
    int pSize = LoRa.parsePacket();
    if (pSize > 0 && pSize < 200) { // On ignore les paquets géants ou corrompus
        String payload = "";
        payload.reserve(pSize + 1);
        while (LoRa.available()) payload += (char)LoRa.read();
        
        g_rssi = LoRa.packetRssi();
        g_snr  = LoRa.packetSnr();
        g_pkts++;
        g_rssi_sum += g_rssi;
        if (g_pkts == 1 || g_rssi < g_rssi_min) g_rssi_min = g_rssi;
        if (g_rssi > g_rssi_max) g_rssi_max = g_rssi;

        Serial.print("LoRa RX: "); Serial.println(payload);

        bool isMKR = payload.startsWith("MKR|");
        g_source   = isMKR ? "MKR WAN 1310" : "Heltec V3";
        if (isMKR) g_pkts_mkr++; else g_pkts_heltec++;
        
        String data = isMKR ? payload.substring(4) : payload;
        
        // Parsing robuste
        int tIdx = data.indexOf("Temp=");
        if (tIdx != -1) {
            int end = data.indexOf(',', tIdx);
            g_temp = data.substring(tIdx + 5, end == -1 ? data.length() : end).toFloat();
        }
        int hIdx = data.indexOf("Hum=");
        if (hIdx != -1) {
            int end = data.indexOf(',', hIdx);
            g_hum = data.substring(hIdx + 4, end == -1 ? data.length() : end).toFloat();
        }
        int pIdx = data.indexOf("Press=");
        if (pIdx != -1) {
            g_press = data.substring(pIdx + 6).toFloat();
        }

        // MQTT Push
        if (!isnan(g_temp))  mqttClient.publish(FEED_TEMP,  String(g_temp, 1).c_str());
        if (!isnan(g_hum))   mqttClient.publish(FEED_HUM,   String(g_hum, 1).c_str());
        if (!isnan(g_press)) mqttClient.publish(FEED_PRESS, String(g_press, 1).c_str());
        mqttClient.publish(FEED_RSSI, String(g_rssi).c_str());
        mqttClient.publish(FEED_SNR,  String(g_snr, 1).c_str());

        updateDisplayData();
    }
}