# Lit Connecté — Réveil intelligent par détection de présence

Système de réveil anti-triche : si je ne suis pas levé 5 minutes après mon alarme, une alarme secondaire se déclenche et continue tant que je suis encore dans le lit. Impossible de tricher — 4 cellules de force sous les pieds du lit mesurent mon poids en continu.

## Comment ça marche

1. L'alarme iPhone sonne → un Raccourci Apple envoie automatiquement une requête HTTP à l'ESP8266
2. L'ESP8266 actionne un relais soudé sur la télécommande du rideau → le rideau se lève
3. Un décompte de 5 minutes démarre en tâche de fond
4. Si je suis encore dans le lit à 5 min → le DFPlayer Mini lance la musique depuis la carte SD
5. La musique s'arrête dès que je me lève

## Matériel

- ESP8266 (serveur web embarqué)
- 4 cellules de force 35 kg + amplificateur HX711
- DFPlayer Mini + carte SD
- Relais soudé sur la télécommande du rideau
- PCB fabriquée et soudée à la main
- Supports imprimés en 3D pour les cellules de force

## Contenu du dépôt

- `docs/` → documentation complète du projet (PDF)
- `code/` → firmware C++ pour ESP8266
- `images/` → photos du montage et de l'installation

## Résultats

- Utilisé quotidiennement, zéro faux positif depuis la mise en service
- Coût total < 35 €
- Intégration iOS native via Raccourci Apple, aucune application tierce

## Licence

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) — Utilisation libre et modification autorisées, usage commercial interdit.
