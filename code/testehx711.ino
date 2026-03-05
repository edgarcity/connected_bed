#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "HX711.h"

// WiFi
const char* ssid = "Reseau YC";
const char* password = "classcroute01";

// Broches
#define RELAIS 5
#define BUZZER 4
#define DOUT 14  // GPIO14 (D5 sur NodeMCU)
#define SCK  12  // GPIO12 (D6 sur NodeMCU)

// Seuils de poids
#define SEUIL_HAUT 50.0  // Active le buzzer si poids > 50kg
#define SEUIL_BAS 30.0   // Désactive le buzzer si poids < 30kg

ESP8266WebServer server(80);
HX711 balance;
float facteur_calibration = 11370.0;
float OFFSET_FIXE = 24.11;
unsigned long reveilTimestamp = 0;
bool buzzerActif = false;
bool surveillanceActive = false;

// ✅ Fonction pour gérer la requête sans provoquer de déconnexion
void handleRequest() {
    if (server.arg("action") == "rotate") {
        server.send(200, "text/plain", "Action effectuée !"); // Répond immédiatement

        yield(); // ⚠️ Évite que le watchdog reset l'ESP8266
        Serial.println("Activation du relais...");
        
        digitalWrite(RELAIS, HIGH);
        delay(500);  // ⏳ Réduit le temps d’activation du relais pour éviter les coupures Wi-Fi
        digitalWrite(RELAIS, LOW);

        reveilTimestamp = millis();  // Enregistre l’heure du réveil
        surveillanceActive = true;

        delay(100); // 🛠️ Pause pour stabiliser la connexion après la réponse
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(RELAIS, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);

    // ✅ WiFi avec timeout de 30s pour éviter une boucle infinie
    WiFi.begin(ssid, password);
    Serial.print("Connexion au WiFi...");
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

    // Serveur Web
    server.on("/", handleRequest);
    server.begin();
    Serial.println("Serveur Web démarré.");

    // Initialisation du capteur HX711
    balance.begin(DOUT, SCK);
    balance.set_scale(facteur_calibration);
}

void loop() {
    server.handleClient();

    if (surveillanceActive && reveilTimestamp > 0) {
        unsigned long elapsedTime = millis() - reveilTimestamp;

        // Vérification après 15 minutes
        if (elapsedTime >= 30000) {
            float poids = (balance.get_units(10) - OFFSET_FIXE);
            Serial.print("Poids mesuré : ");
            Serial.println(poids);

            if (!buzzerActif && poids > SEUIL_HAUT) {
                buzzerActif = true;
                digitalWrite(BUZZER, HIGH);
                Serial.println("Tu es toujours au lit ! Buzzer activé.");
            } else if (buzzerActif && poids < SEUIL_BAS) {
                buzzerActif = false;
                digitalWrite(BUZZER, LOW);
                Serial.println("Tu es levé ! Buzzer désactivé.");
                reveilTimestamp = millis();  // Redémarre la surveillance pour 1 heure
            }
        }

        // Surveillance d'une heure après le lever
        if (!buzzerActif && (millis() - reveilTimestamp <= 3600000)) {
            float poids = (balance.get_units(10) - OFFSET_FIXE);
            Serial.print("Poids surveillé après lever : ");
            Serial.println(poids);

            if (poids > SEUIL_HAUT) {
                buzzerActif = true;
                digitalWrite(BUZZER, HIGH);
                Serial.println("Tu es retourné au lit ! Buzzer réactivé.");
            }
        }
    }
}



