# Guide : Premier flash du SenseCAP Indicator via Arduino IDE

## Prérequis

- SenseCAP Indicator D1
- Câble USB-C
- Mac (macOS 12+)
- Arduino IDE installé (2.x recommandé)

---

## Étape 1 — Identifier les ports USB

Le SenseCAP a **deux MCU** = **deux ports série** quand tu le branches.

Branche-le en USB-C, puis ouvre un **Terminal** et tape :

```bash
ls /dev/tty.*
```

Tu verras apparaître deux ports :

| Port | MCU | Usage |
|------|-----|-------|
| `/dev/tty.wchusbserial*` ou `/dev/tty.usbserial*` | ESP32-S3 | **C'est celui-ci qu'on utilise** (écran + WiFi) |
| `/dev/tty.usbmodem*` | RP2040 | Pour les capteurs (on y touche pas pour l'instant) |

> **Si tu ne vois aucun port wchusbserial/usbserial** : installe le driver CH340 pour Mac.
> Téléchargement : https://www.wch-ic.com/downloads/CH341SER_MAC_ZIP.html
> (Sur macOS 12+, le driver est parfois déjà inclus nativement — essaye d'abord sans.)

---

## Étape 2 — Configurer Arduino IDE

### 2.1 Ajouter le support ESP32

1. Ouvre Arduino IDE → **Fichier > Préférences**
2. Dans "URL de gestionnaire de cartes supplémentaires", ajoute :
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Clique **OK**

### 2.2 Installer le package ESP32

1. Va dans **Outils > Type de carte > Gestionnaire de cartes**
2. Cherche **"esp32"** par Espressif Systems
3. Installe-le (prends la dernière version stable, ex: 2.0.17 ou 3.x)

### 2.3 Sélectionner la bonne carte

1. **Outils > Type de carte** → Choisis **"ESP32S3 Dev Module"**
2. **Outils > Port** → Choisis le port **CH340** (ex: COM4)

### 2.4 Paramètres CRITIQUES à changer

Dans le menu **Outils**, configure ces options :

| Paramètre | Valeur |
|-----------|--------|
| **Board** | ESP32S3 Dev Module |
| **USB CDC On Boot** | Disabled |
| **Flash Size** | 8MB (64Mb) |
| **PSRAM** | OPI PSRAM |
| **Partition Scheme** | 8M with spiffs (3MB APP/1.5MB SPIFFS) |
| **Upload Speed** | 921600 |

> **PSRAM = OPI PSRAM** est **obligatoire** pour que l'écran fonctionne ensuite. Sans ça, ça plante.

---

## Étape 3 — Premier test : Hello World Serial

Copie-colle ce code dans Arduino IDE :

```cpp
// Test SenseCAP Indicator - Hello World via Serial
// Carte : ESP32S3 Dev Module
// Port : USB-SERIAL CH340

void setup() {
  Serial.begin(115200);
  delay(2000); // Laisse le temps au port série de s'initialiser

  Serial.println("=================================");
  Serial.println("  SenseCAP Indicator - TEST OK");
  Serial.println("  ESP32-S3 fonctionne !");
  Serial.println("=================================");
  Serial.println();
  Serial.println("Si tu vois ce message, le flash a marché.");
  Serial.println("Prochaine étape : allumer l'écran !");
}

void loop() {
  Serial.print("Uptime : ");
  Serial.print(millis() / 1000);
  Serial.println(" secondes");
  delay(2000);
}
```

### Pour flasher :

1. Clique sur **Téléverser** (flèche →)
2. Attends la compilation puis l'upload
3. Ouvre **Outils > Moniteur série** à **115200 baud**
4. Tu dois voir les messages "SenseCAP Indicator - TEST OK"

> **Si l'upload échoue** : essaye de maintenir le **bouton interne** (derrière le SenseCAP)
> enfoncé pendant que tu branches le câble USB, puis relâche. Ça force le mode boot.

---

## Étape 4 — Allumer le rétro-éclairage (Bonus)

Si l'étape 3 marche, teste ce code qui allume le rétro-éclairage de l'écran :

```cpp
// Test SenseCAP Indicator - Rétro-éclairage
// Le backlight est sur GPIO45

#define BACKLIGHT_PIN 45

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Allumer le rétro-éclairage
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  Serial.println("Backlight ON !");
  Serial.println("Si l'écran s'éclaire (même blanc/vide), c'est bon !");
}

void loop() {
  // Rien pour l'instant
  delay(1000);
}
```

Si l'écran s'allume (même s'il est blanc ou avec des artefacts), **c'est gagné** — le hardware est OK et tu peux flasher du code dessus.

---

## Dépannage

| Problème | Solution |
|----------|----------|
| Pas de port wchusbserial visible | `ls /dev/tty.*` pour vérifier, installer driver CH340 Mac si besoin |
| Upload timeout | Maintenir bouton interne en branchant le câble |
| Erreur PSRAM | Vérifier que PSRAM = OPI PSRAM dans les options |
| Écran ne s'allume pas (après Étape 4) | Vérifier GPIO45, essayer `analogWrite(45, 128)` |
| "Permission denied" sur le port | Aller dans Préférences Système > Sécurité, autoriser le driver |

---

## Pour la suite

Une fois que ces tests passent, on pourra :
1. Initialiser l'écran LCD (librairies ST7701 + PCA9535)
2. Connecter au WiFi
3. S'abonner aux feeds MQTT Adafruit IO
4. Afficher un dashboard temps réel sur l'écran tactile

---

## Ressources

- Wiki officiel Arduino : https://wiki.seeedstudio.com/SenseCAP_Indicator_ESP32_Arduino/
- SDK ESP32 SenseCAP : https://github.com/Seeed-Solution/SenseCAP_Indicator_ESP32
- SDK RP2040 SenseCAP : https://github.com/Seeed-Solution/SenseCAP_Indicator_RP2040
- Driver CH340 Mac : https://www.wch-ic.com/downloads/CH341SER_MAC_ZIP.html