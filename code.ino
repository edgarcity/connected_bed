#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>
#include "HX711.h"
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// WiFi
const char* ssid = "Reseau YC";
const char* password = "classcroute01";

// Broches
#define RELAIS 5
#define DF_RX  13  // RX du DFPlayer (vers TX ESP8266)
#define DF_TX  2   // TX du DFPlayer (vers RX ESP8266)
#define DOUT 14    // HX711 Data (D5 sur NodeMCU)
#define SCK  12    // HX711 Clock (D6 sur NodeMCU)

// Seuils de poids
#define SEUIL_HAUT 50.0  
#define SEUIL_BAS 30.0   

ESP8266WebServer server(80);
HX711 balance;
SoftwareSerial mp3Serial(DF_RX, DF_TX);  
DFRobotDFPlayerMini dfPlayer;

float facteur_calibration = 11370.0;
float OFFSET_FIXE = 37.98;

unsigned long reveilTimestamp = 0;
bool attente15min = false;
bool musiqueActive = false;
bool surveillanceActive = false;
bool dejaLeve = false;

void handleRequest() {
    if (server.arg("action") == "rotate") {
        server.send(200, "text/plain", "Action effectuée !");
        yield();
        Serial.println("Activation du relais...");

        digitalWrite(RELAIS, HIGH);
        delay(500);
        digitalWrite(RELAIS, LOW);

        reveilTimestamp = millis();
        attente15min = true;  
        dejaLeve = false;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(RELAIS, OUTPUT);

    WiFi.begin(ssid, password);
    unsigned long wifiTimeout = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiTimeout < 30000) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnecté. IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n⚠️ Échec de connexion Wi-Fi !");
    }

    server.on("/", handleRequest);
    server.begin();
    Serial.println("Serveur Web démarré.");

    balance.begin(DOUT, SCK);
    balance.set_scale(facteur_calibration);

    // Init DFPlayer Mini sur les nouvelles broches
    mp3Serial.begin(9600);
    if (!dfPlayer.begin(mp3Serial)) {
        Serial.println("⚠️ Erreur DFPlayer Mini !");
        while (true);
    }
    dfPlayer.volume(20);
}

void loop() {
    server.handleClient();
    
    float poids = (balance.get_units(10) - OFFSET_FIXE);
    Serial.print("Poids mesuré : ");
    Serial.println(poids);

    unsigned long elapsedTime = millis() - reveilTimestamp;

    // Déclenchement après 15 min
    if (attente15min && elapsedTime >= 900000) {  
        Serial.println("⏳ Temps écoulé : Vérification du lit.");
        attente15min = false;

        if (poids > SEUIL_HAUT) {
            musiqueActive = true;
            dfPlayer.loop(1);  // 🔥 Joue le fichier 1 en boucle
            Serial.println("🎵 Tu es encore au lit ! Musique activée.");
        }
    }

    // Arrêt de la musique quand l'utilisateur se lève
    if (musiqueActive && poids < SEUIL_BAS) {
        musiqueActive = false;
        dfPlayer.stop();
        dejaLeve = true;
        reveilTimestamp = millis();
        Serial.println("✅ Tu es levé, musique arrêtée.");
    }

    // Surveillance pendant 1h après le réveil
    if (dejaLeve && (millis() - reveilTimestamp <= 3600000)) {
        if (poids > SEUIL_HAUT && !musiqueActive) {
            musiqueActive = true;
            dfPlayer.loop(1);  // 🔥 Joue en boucle
            Serial.println("🎵 Tu es retourné au lit ! Musique réactivée.");
        }
    }
}
