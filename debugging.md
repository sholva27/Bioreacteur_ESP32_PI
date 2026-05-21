# Guide de Débogage Granulaire (Sonde par Sonde)

Ce guide détaille la procédure pour valider le matériel étape par étape en utilisant les drapeaux d'isolation du firmware.

## Étape 1 : Validation de l'Alimentation et de l'I2C
Avant de brancher les sondes, vérifiez que le bus de communication est sain.

1.  Dans `config.h`, réglez :
    ```cpp
    #define USE_ADS1115 1
    #define USE_RTC 1
    #define USE_PH_PROBE 0
    #define USE_OD_SENSOR 0
    #define USE_ACTUATORS 0
    ```
2.  Téléversez le code. Ouvrez le moniteur série (115200 baud).
3.  **Succès** : Le message "WiFi connected" s'affiche sans erreur "Failed to initialize ADS1115".
4.  **Échec** : Vérifiez les tensions 3.3V et 5V, et les résistances de pull-up sur GPIO 8/9.

## Étape 2 : Test de la Sonde pH
1.  Activez la sonde dans `config.h` :
    ```cpp
    #define USE_PH_PROBE 1
    #define VERBOSE_ADC 1
    ```
2.  Observez `PH_V` dans le moniteur série.
3.  **Test à vide** : Court-circuitez le centre du connecteur BNC avec l'extérieur. La tension doit se stabiliser vers ~2.5V (soit pH 7.0 théorique).
4.  **Test avec sonde** : Plongez la sonde dans une solution tampon pH 7.0. Notez la tension. Répétez pour pH 4.0.

## Étape 3 : Test de l'Optical Density (OD)
1.  Activez l'OD dans `config.h` :
    ```cpp
    #define USE_OD_SENSOR 1
    ```
2.  Vérifiez que le Laser/LED s'allume brièvement toutes les 2 secondes.
3.  **Test d'obscurité** : Couvrez la photodiode. `OD_V` doit être proche de 0V.
4.  **Test de pleine lumière** : Pointez le laser sur la photodiode (vaisseau vide). `OD_V` doit être à son maximum (proche de 3.3V ou 5V selon votre montage).

## Étape 4 : Test des Actionneurs (Pompes et Chauffage)
**Attention :** Assurez-vous que les pompes sont amorcées avec de l'eau ou que les tubes ne sont pas à sec pour éviter l'usure.

1.  Activez les actionneurs :
    ```cpp
    #define USE_ACTUATORS 1
    ```
2.  Utilisez le bouton tactile (GPIO 17) pour tester manuellement la pompe à nutriments.
3.  Utilisez le Dashboard Web pour activer manuellement chaque pompe et vérifier le sens de rotation.

## Étape 5 : Test de Fluorescence (NADH)
1.  Activez la LED de fluorescence (GPIO 18).
2.  Vérifiez visuellement la lueur UV-A (attention aux yeux, utilisez des lunettes de protection).
3.  Vérifiez la réponse du capteur AS7341 ou de l'ADC via `VERBOSE_ADC`.

## Résolution des Problèmes Communs

| Problème | Cause Possible | Solution |
| :--- | :--- | :--- |
| **"ADS1115 Failed"** | Conflit d'adresse I2C ou câblage | Vérifiez que ADDR est à GND (0x48). Scannez le bus I2C. |
| **Lecture pH instable** | Boucle de masse (Ground Loop) | Utilisez un isolateur I2C ou testez sur batterie. |
| **OD ne change pas** | Laser mal aligné | Ajustez le support 3D pour centrer le faisceau sur la photodiode. |
| **ESP32 redémarre au lancement d'une pompe** | Chute de tension | Ajoutez un condensateur de 1000µF sur le rail 12V. |
