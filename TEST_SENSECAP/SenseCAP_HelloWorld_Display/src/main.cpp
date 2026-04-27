// SenseCAP Indicator D1 - Hello World
// Source officielle : wiki.seeedstudio.com/SenseCAP_Indicator_ESP32_Arduino/

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <PCA95x5.h>

#define GFX_BL 45

// Bus SPI software — CS géré par le PCA9535 via la lib PCA95x5
Arduino_DataBus *bus = new Arduino_SWSPI(
    GFX_NOT_DEFINED /* DC */,
    PCA95x5::Port::P04 /* CS */,
    41 /* SCK */,
    48 /* MOSI */,
    GFX_NOT_DEFINED /* MISO */);

// Panel RGB 480x480
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    18, 17, 16, 21,          /* DE, VSYNC, HSYNC, PCLK */
    4, 3, 2, 1, 0,           /* R0-R4 */
    10, 9, 8, 7, 6, 5,       /* G0-G5 */
    15, 14, 13, 12, 11,      /* B0-B4 */
    1, 10, 8, 50,            /* hsync pol/front/pulse/back */
    1, 10, 8, 20);           /* vsync pol/front/pulse/back */

// Affichage ST7701 + RGB (séquence d'init type1 intégrée dans la lib)
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    480, 480, rgbpanel,
    2 /* rotation */, true /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */,
    st7701_type1_init_operations,
    sizeof(st7701_type1_init_operations));

void setup() {
    Serial.begin(115200);

    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);

    if (!gfx->begin()) {
        Serial.println("ERREUR: ecran non initialise");
        while (1) delay(100);
    }

    gfx->fillScreen(BLACK);

    gfx->setTextSize(3);
    gfx->setTextColor(GREEN);
    gfx->setCursor(80, 200);
    gfx->println("SENSECAP OK!");

    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(110, 260);
    gfx->println("Hello World :)");

    Serial.println("Affichage OK");
}

void loop() {
    delay(1000);
}
