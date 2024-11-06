/*
   ______               _                  _///_ _           _                   _
  /   _  \             (_)                |  ___| |         | |                 (_)
  |  [_|  |__  ___  ___ _  ___  _ __      | |__ | | ___  ___| |_ _ __ ___  _ __  _  ___  _   _  ___
  |   ___/ _ \| __|| __| |/ _ \| '_ \_____|  __|| |/ _ \/  _|  _| '__/   \| '_ \| |/   \| | | |/ _ \
  |  |  | ( ) |__ ||__ | | ( ) | | | |____| |__ | |  __/| (_| |_| | | (_) | | | | | (_) | |_| |  __/
  \__|   \__,_|___||___|_|\___/|_| [_|    \____/|_|\___|\____\__\_|  \___/|_| |_|_|\__  |\__,_|\___|
                                                                                      | |
                                                                                      \_|
  Fichier :       thermometre_hygrometre.ino
  
  Description :   Programme support au projet "Thermomètre d'intérieur Arduino"
                  (avec exemple de mise en oeuvre : capteur DHT22 + écran OLED i2c + mise en boitier impression 3D)

  Auteur :        Jérôme TOMSKI (https://passionelectronique.fr/)
  Créé le :       01.08.2021

  Librairie Adafruit SSD1306 utilisée :           https://github.com/adafruit/Adafruit_SSD1306 (importable depuis le Gestionnaire de bibliothèques de l'IDE Arduino)
  Librairie Adafruit GFX incluse (dépendance) :   https://github.com/adafruit/Adafruit-GFX-Library (à inclure, lors de l'import de la librairie ci-dessus)

  Police de caractère "Tahu" :                    https://www.dafont.com/fr/tahu.font
  Police de caractère "Bebas Neue" :              https://www.dafont.com/fr/bebas-neue.font
  
  Convertisseur TTF vers GFX FONTS utilisé :      https://rop.nl/truetype2gfx/ (pour intégrer des fonts sympa dans votre programme arduino)
  Convertisseur PNG vers GFX PICTURE utilisé :    https://javl.github.io/image2cpp/ (pour intégrer des images à votre code arduino, pour les faire afficher ensuite sur l'écran OLED)

*/

#include <Adafruit_SSD1306.h>
#include "Tahu16pt7b.h"                     // Police de caractère : Tahu, en taille 16 points
#include "BebasNeue11pt7b.h"                // Police de caractère : Bebas Neue, en taille 11 points
#include <DHT.h>

// =======================================================
// Paramètrages du capteur température/hygrométrie (DHT22)
// =======================================================
#define brocheDeBranchementDHT A1   // La ligne de communication du capteur DHT est branchée sur la pin A1 de l'Arduino Nano
#define typeDeDHT DHT22             // Le type de DHT utilisé est un DHT22 (que vous pouvez changer en DHT11, DHT21, ou autre, le cas échéant)
DHT dht(brocheDeBranchementDHT, typeDeDHT);
int valeurHumidite;                         // Variable qui contiendra la valeur du taux d'humidité ambiant (en %)
int valeurTemperatureEnDegreCelsius;        // Variable qui contiendra la valeur de la température ambiante (exprimée en degrés Celsius)
boolean mesureCorrectementEffectuee;        // Variable qui indiquera si oui ou non la lecture des mesures effectuées par le DHT22 s'est bien passée ou non

// =========================================
// Paramètrages écran OLED 128x64 pixels I2C
// =========================================
#define nombreDePixelsEnLargeur 128         // Taille de l'écran OLED, en pixel, au niveau de sa largeur
#define nombreDePixelsEnHauteur 64          // Taille de l'écran OLED, en pixel, au niveau de sa hauteur
#define brocheResetOLED         -1          // Reset de l'OLED partagé avec l'Arduino (d'où la valeur à -1, et non un numéro de pin)
#define adresseI2CecranOLED     0x3C        // Adresse de "mon" écran OLED sur le bus i2c (généralement égal à 0x3C, ou 0x3D)
#define delaiAlternanceAffichageTemperaturePuisHygrometrie   7000    // Tous les 7000 ms (soit 7 secondes), on alternera l'affichage de la t°, et de l'hygrométrie

// =========================================
// Paramètrages batterie
// =========================================
#define brocheBatterie A7   // Déclarer une broche analogique pour lire la tension de la batterie
float tensionBatterie;       // Variable pour stocker la tension lue
int pourcentageBatterie;     // Variable pour stocker le pourcentage de batterie

Adafruit_SSD1306 ecranOLED(nombreDePixelsEnLargeur, nombreDePixelsEnHauteur, &Wire, brocheResetOLED);   // Le "Wire" indique qu'on travaille en I2C, au niveau de l'écran OLED


// ====================================
// Spécifications des images à afficher
// ====================================
#define largeurImageAafficher   27          // Largeur des images à afficher (symbole thermomètre et symbole goutte d'eau), en pixels
#define hauteurImageAafficher   40          // Hauteur des images à afficher (symbole thermomètre et symbole goutte d'eau), en pixels

// Image "goutte-eau-pse_v2", faisant 27x40px
const unsigned char monImageGoutteEauPse [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 
  0x00, 0x13, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x61, 0x80, 0x00, 0x00, 0x40, 0xc0, 0x00, 
  0x00, 0xc0, 0x60, 0x00, 0x01, 0x80, 0x20, 0x00, 0x01, 0x00, 0x30, 0x00, 0x03, 0x00, 0x10, 0x00, 
  0x06, 0x00, 0x10, 0x00, 0x04, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x04, 0x00, 
  0x18, 0x00, 0x06, 0x00, 0x30, 0x00, 0x03, 0x00, 0x20, 0x00, 0x01, 0x00, 0x20, 0x00, 0x01, 0x80, 
  0x20, 0x00, 0x00, 0x80, 0x60, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 
  0x40, 0x00, 0x00, 0xc0, 0x40, 0x00, 0x00, 0x40, 0x48, 0x00, 0x00, 0x40, 0x4c, 0x00, 0x00, 0x40, 
  0x44, 0x00, 0x00, 0xc0, 0x44, 0x00, 0x00, 0x80, 0x66, 0x00, 0x00, 0x80, 0x23, 0x00, 0x00, 0x80, 
  0x31, 0x80, 0x01, 0x80, 0x10, 0xc0, 0x03, 0x00, 0x18, 0x70, 0x06, 0x00, 0x0c, 0x00, 0x0c, 0x00, 
  0x06, 0x00, 0x18, 0x00, 0x03, 0xe3, 0xf0, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// Image "thermometre-mercure-pse-27x40", faisant 27x40px
const unsigned char monImageThermometreMercure [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x06, 0x03, 0x3f, 0x00, 
  0x04, 0x01, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x04, 0x01, 0x38, 0x00, 0x04, 0x01, 0x00, 0x00, 
  0x04, 0x01, 0x00, 0x00, 0x04, 0x01, 0x3f, 0x00, 0x04, 0x01, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 
  0x04, 0x71, 0x38, 0x00, 0x04, 0xf9, 0x00, 0x00, 0x04, 0xf9, 0x00, 0x00, 0x04, 0xf9, 0x3f, 0x00, 
  0x04, 0xf9, 0x00, 0x00, 0x04, 0xf9, 0x00, 0x00, 0x04, 0xf9, 0x38, 0x00, 0x04, 0xf9, 0x00, 0x00, 
  0x04, 0xf9, 0x00, 0x00, 0x04, 0xf9, 0x3e, 0x00, 0x04, 0xf9, 0x00, 0x00, 0x04, 0xf9, 0x00, 0x00, 
  0x0c, 0xf9, 0x80, 0x00, 0x18, 0xf8, 0xc0, 0x00, 0x11, 0xfc, 0x40, 0x00, 0x33, 0xfe, 0x60, 0x00, 
  0x23, 0xfe, 0x20, 0x00, 0x67, 0xff, 0x30, 0x00, 0x47, 0xff, 0x10, 0x00, 0x67, 0xff, 0x30, 0x00, 
  0x23, 0xfe, 0x20, 0x00, 0x31, 0xfc, 0x60, 0x00, 0x10, 0xf8, 0x40, 0x00, 0x18, 0x70, 0xc0, 0x00, 
  0x0e, 0x03, 0x80, 0x00, 0x03, 0x8e, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ========================
// Initialisation programme
// ========================
void setup() {

  // Initialisation du DHT22;
  dht.begin();

  // Initialisation de l'écran OLED
  if(!ecranOLED.begin(SSD1306_SWITCHCAPVCC, adresseI2CecranOLED))
    while(1);                               // Arrêt du programme (boucle infinie) si échec d'initialisation de l'écran OLED
  

  // Affichage du texte de démarrage
  ecranOLED.clearDisplay();                 // Vidange du buffer de l'écran OLED
  ecranOLED.setTextSize(1);                 // Sélection de facteur 1:1 pour le facteur de grossissement du texte
  ecranOLED.setTextColor(WHITE);            // Couleur "blanche" (ou colorée, si votre afficheur monochrome est bleu, jaune, ou bleu/jaune)
  ecranOLED.setCursor(15,35);               // Positionnement du "curseur invisible" de l'écran OLED, en position x=15 (axe horizontal), et y=35 (axe vertical)

  ecranOLED.setFont(&Tahu16pt7b);           // Sélection de la police "Tahu"
  ecranOLED.println("Bonjour !");           // …et affichage du message de bienvenue

  ecranOLED.display();
  delay(3000);                              // On laisse le mot "Bonjour !" affiché 3 secondes à l'écran, avant d'attaquer la boucle principale
    
}


// ======================================
// Boucle principale (boucle perpétuelle)
// ======================================
void loop() {
  // Récupérer les informations de température et d'humidité
  recupInfosTemperatureEtHygrometrie();
  
  // Lire le niveau de la batterie
  lireBatterie();
  
  // Afficher la température
  affichageTemperature();
  delay(1000);
  
  // Afficher le taux d'humidité
  affichageHygrometrie();
  delay(1000);

  // Afficher le pourcentage de batterie
  affichageBatterie();
  delay(2000);  // Petite pause avant de relancer une autre lecture
}

void lireBatterie() {
  // Lire la tension sur la broche (en utilisant un pont diviseur)
  tensionBatterie = analogRead(brocheBatterie) * (5.0 / 1023.0);  // Calculer la tension lue
  
  // Afficher la tension pour déboguer
  Serial.print("Tension lue : ");
  Serial.println(tensionBatterie, 3); // Afficher avec 3 décimales
  
  // Plage de tension de la batterie
  float tensionMax = 9.0;  // Tension maximale de la batterie (9V)
  float tensionMin = 6.0;   // Tension minimale de la batterie (environ 6V)

  // Calculer la tension réelle de la batterie en fonction du pont diviseur
  float tensionReelle = tensionBatterie * 3;  // Multiplier par 3 pour obtenir la tension de la batterie

  // Calcul du pourcentage
  pourcentageBatterie = (int)(((tensionReelle - tensionMin) / (tensionMax - tensionMin)) * 100);

  // Assurer que le pourcentage reste dans les bornes
  if (pourcentageBatterie > 100) {
    pourcentageBatterie = 100;
  } else if (pourcentageBatterie < 0) {
    pourcentageBatterie = 0;
  }

  // Afficher le pourcentage pour déboguer
  Serial.print("Pourcentage batterie : ");
  Serial.println(pourcentageBatterie);
}


void affichageBatterie() {
  // Affichage de l'état de la batterie sur l'écran OLED
  ecranOLED.clearDisplay();
  ecranOLED.setFont(&BebasNeue11pt7b);      // Sélection de la police "Bebas Neue", taille 11pt
  ecranOLED.setTextSize(1);
  ecranOLED.setTextColor(WHITE);
  ecranOLED.setCursor(15,15);               // Positionner le texte dans le coin supérieur gauche de l'écran
  ecranOLED.print("BAT: ");
  
  ecranOLED.setTextSize(1);
  ecranOLED.print(pourcentageBatterie);
  ecranOLED.print("%");
  
  ecranOLED.display();
}


// ========================================================
// Section : récupération des infos température/hygrométrie
// ========================================================
void recupInfosTemperatureEtHygrometrie() {
  // Lecture des données (interrogation du capteur DHT22 par l'Arduino)
  float tauxHumidite = dht.readHumidity();                                  // Lecture du taux d'humidité (en %)
  float temperatureEnDegreCelsius = dht.readTemperature();                  // Lecture de la température, exprimée en degrés Celsius

  // Vérification si données bien reçues
  if (isnan(tauxHumidite) || isnan(temperatureEnDegreCelsius)) {            // Si aucune valeur correcte n'est retournée par le capteur DHT22, on met
    mesureCorrectementEffectuee = false;                                    // à "faux" la variable "mesureCorrectementEffectuee", pour ensuite faire afficher
  }                                                                         // le sigle "--" sur l'écran OLED (indiquant qu'aucune valeur n'a pu être relevée)
  else
  {                                                                         // Sauvegarde de ces valeurs mesurées, pour affichage ultérieur
    valeurHumidite = round(tauxHumidite);                                   // Arrondi à la valeur entière la plus proche
    valeurTemperatureEnDegreCelsius = round(temperatureEnDegreCelsius)-1;   // Arrondi à la valeur entière la plus proche (auquel j'ai enlevé 1°C, pour compenser la mise en boitier)  
    mesureCorrectementEffectuee = true;                                     // … et on mémorise le fait que la mesure s'est bien passée
  }
}


// =====================================
// Section : affichage de la température
// =====================================
void affichageTemperature() {
  // Affichage du symbole "thermomètre" à l'écran
  ecranOLED.clearDisplay();                 // Effaçage de la mémoire tampon de l'écran OLED
  ecranOLED.drawBitmap(
    8,                                      // Alignement de la gauche de l'image à 8 pixels du côté gauche de l'écran
    20,                                     // Alignement du haut de l'image à 20 pixels du haut de l'écran (pour être dans la partie bleue, et non la jaune)
    monImageThermometreMercure,             // Chargement de l'image "thermomètre"
    largeurImageAafficher,
    hauteurImageAafficher,
    1);

  // Titre
  ecranOLED.setFont(&BebasNeue11pt7b);      // Sélection de la police "Bebas Neue", taille 11pt
  ecranOLED.setTextSize(1);
  ecranOLED.setTextColor(WHITE);
  ecranOLED.setCursor(15,15);               // Positionnement du texte en x=15, et y=15 pixels
  ecranOLED.print("TEMPERATURE");           // Et affichage du mot "TEMPERATURE"

  // Valeur
  ecranOLED.setFont();                      // Re-sélection de la police de caractère par défaut, de la librairie Adafruit
  ecranOLED.setTextSize(3);
  ecranOLED.setTextColor(WHITE);
  if(valeurTemperatureEnDegreCelsius<0)
    ecranOLED.setCursor(40,30);             // x, y (48,30) si valeur de t° avec 2 chiffres, et (40,30) pour valeur à 3 chiffres (ou t° négative)
  else
    ecranOLED.setCursor(48,30);
  if(mesureCorrectementEffectuee == false)
    ecranOLED.print("--");                                // Si problème de lecture au niveau du DHT22, affichage du symbole "--"
  else
    ecranOLED.print(valeurTemperatureEnDegreCelsius);     // Si aucun pb au niveau du DHT22, affichage de la valeur de température mesurée
  ecranOLED.setTextSize(2);
  ecranOLED.print((char)247);               // Appelle le symbole ressemblant au sigle degré "°", car ce dernier n'existe pas vraiment dans cette police de caractère
  ecranOLED.setTextSize(3);                 // (d'où le changement de tailles de texte (taille 3 -> taille 2 -> taille 3), pour tricher un peu !
  ecranOLED.print("C");
  
  ecranOLED.display();                      // Transfert de la mémoire tampon à l'écran, pour affichage de tout ça !
}


// ======================================
// Section : affichage du taux d'humidité
// ======================================
void affichageHygrometrie() {
  // Affichage du symbole "goutte d'eau" à l'écran
  ecranOLED.clearDisplay();                 // Effaçage de la mémoire tampon de l'écran OLED
  ecranOLED.drawBitmap(
    16,                                     // Alignement de la gauche de l'image à 8 pixels du côté gauche de l'écran
    20,                                     // Alignement du haut de l'image à 20 pixels du haut de l'écran (pour être dans la partie bleue, et non la jaune)
    monImageGoutteEauPse,                   // Chargement de l'image "goutte d'eau"
    largeurImageAafficher,
    hauteurImageAafficher,
    1);

  // Titre
  ecranOLED.setFont(&BebasNeue11pt7b);      // Sélection de la police "Bebas Neue", taille 11pt
  ecranOLED.setTextSize(1);
  ecranOLED.setTextColor(WHITE);
  ecranOLED.setCursor(18,15);               // Positionnement du texte en x=18, et y=15 pixels
  ecranOLED.print("HYGROMETRIE");           // Et affichage du mot "HYGROMETRIE"

  // Valeur
  ecranOLED.setFont();                      // Re-sélection de la police de caractère par défaut, de la librairie Adafruit
  ecranOLED.setTextSize(3);
  ecranOLED.setTextColor(WHITE);
  if(valeurHumidite==100)
    ecranOLED.setCursor(46,30);             // x, y (56,30) si valeur d'hygrométrie sur 2 chiffres, ou (46,30) si valeur sur 3 chiffres (par ex : "100" %)
  else
    ecranOLED.setCursor(56,30);
  if(mesureCorrectementEffectuee == false)
    ecranOLED.print("--");                  // Si problème de lecture au niveau du DHT22, affichage du symbole "--"
  else
    ecranOLED.print(valeurHumidite);        // Si aucun pb au niveau du DHT22, affichage de la valeur du taux d'humidité mesuré
  ecranOLED.print("%");
  
  ecranOLED.display();                      // Transfert de la mémoire tampon de l'ODED à l'écran, pour affichage
}
