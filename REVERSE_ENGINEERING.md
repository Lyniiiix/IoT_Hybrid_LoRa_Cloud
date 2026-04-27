# 🌟 Guide du Projet IoT

Ce projet permet de surveiller la température et l'humidité à distance, sans que les capteurs aient besoin d'Internet !

## 🎯 Le But du Projet
Imaginez que vous voulez connaître la météo au fond de votre jardin ou dans un champ, mais le WiFi ne va pas jusque-là.
Ce projet résout ce problème en deux étapes :
1. **Les Capteurs (Émetteurs)** mesurent la température et l'envoient par ondes radio longue portée (**LoRa**).
2. **Les Récepteurs (Passerelles)** reçoivent ces ondes et, comme ils sont connectés au WiFi, ils envoient les mesures sur Internet (**Adafruit IO**).

---

## 🚀 Comment ça marche ? (Le Flux de Données)

1. **Je mesure** 🌡️
   - Des petites cartes électroniques (Arduino/Heltec) lisent la température et l'humidité.
2. **J'envoie loin** 📡
   - Elles transforment ces mesures en un message radio **LoRa** (comme un talkie-walkie très puissant mais qui parle lentement).
   - *Exemple de message : "Il fait 22°C et 50% d'humidité"*
3. **Je reçois et je traduis** 📟 ➡️ ☁️
   - Une autre carte (le Récepteur) entend le message radio.
   - Elle se connecte à votre WiFi et envoie ces infos sur un site web appelé **Adafruit IO** via un protocole de messagerie appelé **MQTT**.
4. **Je visualise** 💻
   - Vous ouvrez votre navigateur sur Adafruit IO et vous voyez les courbes de température !

---

## 📦 Qui fait quoi ? (Le matériel)

### Les Équipes

#### 🔵 L'Équipe "Terrain" (Les Émetteurs)
*Ils sont dehors, sur batterie, sans WiFi.*
1. **Sender 1 (Heltec V3)** :
   - Capteur : **DHT11** (un petit capteur bleu classique).
   - Rôle : Lit la température et l'envoie par radio.
2. **Sender 2 (Arduino MKR WAN)** :
   - Capteur : **BME680** (plus précis, mesure aussi la pression).
   - Rôle : Lit la température, l'humidité et la pression, puis l'envoie par radio.

#### 🟢 L'Équipe "Maison" (Les Récepteurs)
*Ils sont à l'intérieur, connectés au WiFi.*
1. **Receiver 1 (Heltec V3)** & **Receiver 2 (M5Core)** :
   - Rôle : Ils écoutent la fréquence radio 868 MHz. Dès qu'ils reçoivent un message, ils le transmettent sur Internet.

---

## 📚 Petit Glossaire pour Comprendre
- **LoRa (Long Range)** : Une technologie radio qui va très loin (plusieurs km !) mais qui ne peut envoyer que de tout petits messages (pas de vidéo, juste du texte). C'est idéal pour des capteurs.
- **MQTT** : Le langage que les objets connectés utilisent pour parler sur Internet. C'est comme des SMS pour les machines.
- **Passerelle (Gateway)** : Un appareil qui fait le pont entre deux mondes (ici, entre le monde Radio LoRa et le monde Internet WiFi).
- **Payload** : C'est le contenu utile du message (ex: "Temp=24.5").

---

## 🛠️ Détails Techniques (Pour aller plus loin)

> *Cette section contient les détails techniques bruts si vous avez besoin de modifier le code plus tard.*

### Configuration Radio
- **Fréquence** : 868.0 MHz (Standard en Europe).
- **Format** : Les messages ressemblent à ça : `Temp=20.5,Hum=45.0`.

### Les Programmes
- **LoRa_Sender_HeltecV3** : Code pour l'émetteur avec DHT11.
- **LoRa_Sender_MKR-WAN-1310** : Code pour l'émetteur Arduino.
- **LoRa_Receiver_...** : Codes pour les récepteurs qui font le lien avec Adafruit.

### Connexion Internet
- Les récepteurs envoient les données sur le site `io.adafruit.com`.
- Il faudra mettre votre **Nom WiFi** et votre **Mot de passe** dans le code des fichiers "Receiver".
