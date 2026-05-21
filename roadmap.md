# Roadmap des Améliorations Futures

Ce document définit les axes de développement pour transformer ce biofermenteur en un outil de grade industriel ou de recherche avancée.

## 🟢 Niveau 1 : Fiabilité & Précision (Hardware)
*Objectif : Éliminer le bruit et sécuriser le fonctionnement à long terme.*

*   **Isolation Galvanique :** Intégration d'un circuit **ISO1540** ou **ADuM1250** sur le bus I2C pour isoler électriquement les capteurs sensibles (pH/OD) du bruit des moteurs (pompes/agitateur).
*   **Conception d'un Shield PCB :** Transition du câblage "breadboard" vers un PCB personnalisé (KiCad) avec des plans de masse séparés pour l'analogique et le numérique.
*   **Alimentation Stabilisée :** Utilisation d'un régulateur LDO haute performance pour les capteurs et de condensateurs de filtrage (1000µF+) sur les lignes de puissance des pompes.

## 🔵 Niveau 2 : Intelligence & Connectivité (Software)
*Objectif : Automatiser la surveillance et optimiser les rendements.*

*   **Alertes Mobiles :** Intégration d'un **Bot Telegram** via l'API `UniversalTelegramBot` pour recevoir des alertes en temps réel (ex: "Alerte pH bas !", "Niveau de nutriment critique").
*   **Edge AI (TinyML) :** Utilisation de la bibliothèque **EloquentTinyML** sur l'ESP32-S3 pour entraîner un modèle capable de prédire le moment optimal de récolte ou de détecter une contamination.
*   **Journalisation Cloud :** Exportation automatique des données vers **InfluxDB** ou **ThingsBoard** pour un archivage historique illimité et des analyses multi-batch.

## 🔴 Niveau 3 : Ingénierie des Procédés (Bio-Process)
*Objectif : Contrôle total sur l'environnement métabolique.*

*   **Contrôle de l'Oxygène Dissous (DO) :** Ajout d'une sonde DO (polarographique ou optique) et asservissement d'une pompe à air ou d'un concentrateur d'O2.
*   **Analyse des Gaz de Sortie (Off-gas) :** Montage d'un capteur de CO2 (type NDIR) et O2 sur l'évent du vaisseau pour calculer le **Quotient Respiratoire (RQ)**.
*   **Stérilisation In-Situ (SIP) :** Ajout de résistances de chauffe haute puissance pour permettre une stérilisation à la vapeur ou thermique du vaisseau vide avant inoculation.

## 🟡 Niveau 4 : Expérience Utilisateur (UX)
*   **Auto-Calibration des Pompes :** Assistant logiciel permettant de calculer automatiquement le volume par tour/seconde (mL/s) pour chaque pompe peristaltique.
*   **Interface Locale :** Ajout d'un écran tactile **ILI9488** pour monitorer le système sans smartphone ni PC.
