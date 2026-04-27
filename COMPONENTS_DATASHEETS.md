# 📑 Fiches Techniques Simplifiées (Datasheets)

Voici les cartes d'identité des composants utilisés dans ce projet.

## 1. Heltec WiFi LoRa 32 (V3)
**C'est quoi ?** : Une carte tout-en-un puissante avec écran, WiFi et LoRa.
**Rôle** : Cerveau des émetteurs et récepteurs.

| Caractéristique | Valeur / Détail |
| :--- | :--- |
| **Processeur** | ESP32-S3 (Rapide, double cœur) |
| **Puce LoRa** | SX1262 (Nouvelle génération, très performante) |
| **Connectivité** | WiFi, Bluetooth 5, LoRa (868 MHz) |
| **Écran** | OLED 0.96" intégré (Noir & Blanc) |
| **Alimentation** | USB-C ou Batterie LiPo (connecteur JST) |
| **Voltage Logique** | 3.3V |

**Pourquoi on l'aime ?** : Tout est intégré (écran, LoRa, Antenne), pas besoin de souder ou de mettre des fils partout. C'est le couteau suisse du projet.

---

## 2. Arduino MKR WAN 1310
**C'est quoi ?** : Une carte officielle Arduino spécialisée pour le LoRa basse consommation.
**Rôle** : Émetteur longue durée (sur le terrain).

| Caractéristique | Valeur / Détail |
| :--- | :--- |
| **Processeur** | SAMD21 Cortex-M0+ |
| **Module LoRa** | Murata CMWX1ZZABZ |
| **Voltage Logique** | **3.3V** (Attention ! Ne supporte pas le 5V) |
| **Connecteur** | Micro-USB |
| **Alimentation** | Port pour batterie LiPo/LiIon intégré |

**Pourquoi on l'aime ?** : Elle consomme très peu d'énergie. C'est la carte idéale pour rester des mois dehors sur batterie.

---

## 3. M5Stack Core (Basic/Gray)
**C'est quoi ?** : Un boîtier carré robuste avec écran couleur, boutons et batterie intégrée.
**Rôle** : Récepteur (Gateway) confortable à utiliser.

| Caractéristique | Valeur / Détail |
| :--- | :--- |
| **Processeur** | ESP32 (240MHz, Dual Core) |
| **Écran** | LCD Couleur 2 pouces (320x240 px) |
| **Interface** | 3 Gros boutons en façade, Haut-parleur |
| **Extension LoRa** | Nécessite un module LoRa externe (M5 LoRa Module) |
| **Connectivité** | WiFi, Bluetooth |

**Pourquoi on l'aime ?** : C'est "propre". Ça fait produit fini industriel, solide, et l'écran couleur permet d'afficher de belles interfaces.

---

## 4. Capteur DHT11 (Le Bleu)
**C'est quoi ?** : Le capteur de température le plus basique et le moins cher du marché.
**Mesures** :
- **Température** : 0 à 50°C (Précision ±2°C)
- **Humidité** : 20 à 90% (Précision ±5%)

**Verdict** : Un peu lent et pas très précis, mais suffisant pour débuter et apprendre.

---

## 5. Capteur BME680 (Sur MKR IoT Carrier)
**C'est quoi ?** : Un capteur environnemental haut de gamme "4-en-1".
**Mesures** :
- **Température** : -40 à +85°C (Précision ±1.0°C)
- **Humidité** : 0 à 100% (Précision ±3%)
- **Pression** : 300 à 1100 hPa (Précision ±0.6 hPa)
- **Gaz** : Qualité de l'air (Composés Organiques Volatils)

**Verdict** : Le top du top. Beaucoup plus fiable, rapide et complet que le DHT11. Il est ici intégré directement sur la platine "MKR IoT Carrier".
