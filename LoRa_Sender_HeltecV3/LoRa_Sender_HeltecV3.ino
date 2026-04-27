// ==========================================
// HELTEC V3 — LoRa P2P Sender + OLED Diaporama
// ==========================================
// Slide 0 : Température + Humidité (grande police)
// Slide 1 : Statut LoRa (Send OK / FAIL + freq)
// Slide 2 : Infos système (SF, freq, projet)
//
// Carte : Heltec WiFi LoRa 32(V3)
// Libs  : RadioLib, DHT, U8g2

#include <SPI.h>
#include <RadioLib.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <U8g2lib.h>

// ──────────────────────── DHT11 ────────────────────────
#define DHTPIN  2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ──────────────────────── OLED ─────────────────────────
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 18, 17, 21);

// ──────────────────────── LoRa SX1262 ──────────────────
#define LORA_SCK   9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_CS    8
#define LORA_RST  12
#define LORA_BUSY 13
#define LORA_DIO1 14

SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Paramètres LoRa
#define LORA_FREQ     868.0
#define LORA_BW       125.0
#define LORA_SF       12
#define LORA_CR       5
#define LORA_TX_POWER 17

// ──────────────────────── ÉTAT ─────────────────────────
float   lastTemp   = NAN;
float   lastHum    = NAN;
bool    lastSendOK = false;
int     lastState  = 0;
bool    loraOK     = false;  // false si init LoRa a échoué
uint8_t slideIdx   = 0;

#define SLIDE_COUNT    3
#define SLIDE_INTERVAL 3000  // 3 secondes par slide
#define SEND_INTERVAL  10000 // Envoi LoRa toutes les 10s

unsigned long lastSlideMs = 0;
unsigned long lastSendMs  = 0;

// ─────────────────────── AFFICHAGE ──────────────────────
void showSlide() {
    u8g2.clearBuffer();

    switch (slideIdx) {

        // ── Slide 0 : Valeurs capteur ──
        case 0:
            u8g2.setFont(u8g2_font_6x10_tr);
            u8g2.drawStr(0, 10, "DHT11  [ 1/3 ]");
            u8g2.drawHLine(0, 13, 128);

            if (!isnan(lastTemp) && !isnan(lastHum)) {
                char tStr[16], hStr[16];
                snprintf(tStr, sizeof(tStr), "%.1f C", lastTemp);
                snprintf(hStr, sizeof(hStr), "%.1f %%", lastHum);
                u8g2.setFont(u8g2_font_ncenB18_tr);
                u8g2.drawStr(4, 38, tStr);
                u8g2.drawStr(4, 62, hStr);
            } else {
                u8g2.setFont(u8g2_font_8x13_tr);
                u8g2.drawStr(0, 35, "Lecture...");
            }
            break;

        // ── Slide 1 : Statut LoRa ──
        case 1:
            u8g2.setFont(u8g2_font_6x10_tr);
            u8g2.drawStr(0, 10, "LoRa TX  [ 2/3 ]");
            u8g2.drawHLine(0, 13, 128);

            u8g2.setFont(u8g2_font_8x13_tr);
            if (!loraOK) {
                u8g2.drawStr(0, 30, "> LoRa OFF");
                u8g2.setFont(u8g2_font_6x10_tr);
                u8g2.drawStr(0, 46, "Init echouee");
            } else if (lastSendOK) {
                u8g2.drawStr(0, 30, "> Send OK !");
                u8g2.setFont(u8g2_font_6x10_tr);
                u8g2.drawStr(0, 46, "868MHz  SF12  BW125");
                u8g2.drawStr(0, 58, "TX: 17dBm  CR: 5");
            } else {
                char errMsg[24];
                snprintf(errMsg, sizeof(errMsg), "> FAIL (%d)", lastState);
                u8g2.drawStr(0, 30, errMsg);
                u8g2.setFont(u8g2_font_6x10_tr);
                u8g2.drawStr(0, 46, "868MHz  SF12  BW125");
                u8g2.drawStr(0, 58, "TX: 17dBm  CR: 5");
            }
            break;

        // ── Slide 2 : Infos système ──
        case 2:
            u8g2.setFont(u8g2_font_6x10_tr);
            u8g2.drawStr(0, 10, "Systeme  [ 3/3 ]");
            u8g2.drawHLine(0, 13, 128);
            u8g2.drawStr(0, 27, "Heltec WiFi LoRa V3");
            u8g2.drawStr(0, 40, "RadioLib + SX1262");
            u8g2.drawStr(0, 53, "IoT Monitoring 2026");
            break;
    }

    u8g2.sendBuffer();
}

// ──────────────────────── SETUP ────────────────────────
void setup() {
    Serial.begin(115200);
    delay(1500);

    // ── OLED Power (Vext = GPIO36 LOW) ──
    pinMode(36, OUTPUT); digitalWrite(36, LOW); delay(100);
    pinMode(21, OUTPUT); digitalWrite(21, LOW); delay(10);
    digitalWrite(21, HIGH); delay(50);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    u8g2.drawStr(0, 20, "Heltec LoRa TX");
    u8g2.drawStr(0, 40, "Demarrage...");
    u8g2.sendBuffer();

    // ── DHT11 ──
    dht.begin();
    Serial.println("DHT11 OK");

    // ── LoRa SX1262 ──
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, 0x12, LORA_TX_POWER, 8);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("LoRa FAIL: "); Serial.println(state);
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_8x13_tr);
        u8g2.drawStr(0, 30, "LoRa FAIL!");
        char err[16]; snprintf(err, sizeof(err), "Code: %d", state);
        u8g2.drawStr(0, 50, err);
        u8g2.sendBuffer();
        // Pas de while(1) — on continue sans LoRa
        loraOK = false;
    } else {
        loraOK = true;
        Serial.println("LoRa OK");
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    u8g2.drawStr(0, 20, "Heltec LoRa TX");
    u8g2.drawStr(0, 40, "Pret !");
    u8g2.sendBuffer();
    delay(1000);

    lastSlideMs = millis();
    lastSendMs  = millis(); // Premier envoi dans 10s (pas immédiat)
    slideIdx    = 0;
    showSlide(); // Afficher slide 0 tout de suite
}

// ──────────────────────── LOOP ─────────────────────────
void loop() {
    unsigned long now = millis();

    // ── Envoi LoRa toutes les 10s ──
    if (now - lastSendMs >= SEND_INTERVAL) {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t) && !isnan(h)) { lastTemp = t; lastHum = h; }

        if (loraOK && !isnan(lastTemp) && !isnan(lastHum)) {
            char payload[32];
            snprintf(payload, sizeof(payload), "Temp=%.1f,Hum=%.1f", lastTemp, lastHum);
            Serial.print("Sending: "); Serial.println(payload);
            lastState  = radio.transmit(payload);
            lastSendOK = (lastState == RADIOLIB_ERR_NONE);
            Serial.println(lastSendOK ? "Send OK" : "Send FAIL");
        }
        lastSendMs = now;
    }

    // ── Changement de slide toutes les 3s ──
    if (now - lastSlideMs >= SLIDE_INTERVAL) {
        slideIdx = (slideIdx + 1) % SLIDE_COUNT;
        lastSlideMs = now;
        showSlide();
    }
}
