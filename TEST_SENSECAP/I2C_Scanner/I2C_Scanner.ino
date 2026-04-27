/*
 * I2C Scanner MULTI-PINS - SenseCAP Indicator D1
 * Teste automatiquement plusieurs combinaisons SDA/SCL
 * pour trouver les bons pins I2C.
 *
 * Carte : ESP32S3 Dev Module (PSRAM OPI, Flash 8MB)
 */

#include <Wire.h>

// Paires SDA/SCL à tester
const int8_t candidates[][2] = {
    {39, 40},   // Hypothèse 1 (wiki Seeed)
    {40, 39},   // Hypothèse 1 inversé
    {20, 19},   // Hypothèse 2
    {19, 20},   // Hypothèse 2 inversé
    {38, 47},   // Hypothèse 3
    {47, 38},   // Hypothèse 3 inversé
    {21, 22},   // Hypothèse 4
    {22, 21},   // Hypothèse 4 inversé
};
const uint8_t N_CANDIDATES = sizeof(candidates) / sizeof(candidates[0]);

bool scanBus(int8_t sda, int8_t scl) {
    Wire.end();
    delay(50);
    Wire.begin(sda, scl, 100000);  // 100kHz, plus tolérant
    delay(50);

    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            found++;
        }
    }
    Wire.end();
    return found > 0;
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\n=== I2C Scanner Multi-Pins - SenseCAP Indicator ===");
    Serial.println("Test de " + String(N_CANDIDATES) + " combinaisons SDA/SCL...\n");

    for (uint8_t i = 0; i < N_CANDIDATES; i++) {
        int8_t sda = candidates[i][0];
        int8_t scl = candidates[i][1];

        Serial.printf("Test SDA=%2d SCL=%2d  -->  ", sda, scl);
        Serial.flush();

        Wire.end();
        delay(50);

        bool ok = Wire.begin(sda, scl, 100000);
        delay(50);

        uint8_t found = 0;
        for (uint8_t addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                found++;
            }
        }
        Wire.end();
        delay(50);

        if (found > 0) {
            Serial.printf("%d device(s) trouve(s) !\n", found);

            // Re-scan pour lister les adresses
            Wire.begin(sda, scl, 100000);
            delay(50);
            for (uint8_t addr = 1; addr < 127; addr++) {
                Wire.beginTransmission(addr);
                if (Wire.endTransmission() == 0) {
                    Serial.printf("    --> 0x%02X", addr);
                    if (addr == 0x20) Serial.print("  [PCA9535 - GPIO expander ecran]");
                    if (addr == 0x21) Serial.print("  [PCA9535 - addr alt]");
                    if (addr == 0x38) Serial.print("  [FT6336 - touch]");
                    if (addr == 0x48) Serial.print("  [FT6336 - touch addr alt]");
                    Serial.println();
                }
            }
            Wire.end();
        } else {
            Serial.println("rien.");
        }
    }

    Serial.println("\n=== Scan termine ===");
    Serial.println("Les pins avec des devices = les bons I2C du SenseCAP.");
    Serial.println("Note les dans le moniteur serie et communique les a Claude.");
}

void loop() {}
