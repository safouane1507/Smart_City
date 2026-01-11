#pragma once
#ifndef ADAPTIVE_LOGIC_H
#define ADAPTIVE_LOGIC_H

#include "sensorr.h"
#include <vector>
#include <string>

//  Classe LogiqueAdaptative : Contient les règles pour ajuster les feux
class LogiqueAdaptative {
private:
    static constexpr int DUREE_VERTE_MINIMALE = 10;
    static constexpr int DUREE_VERTE_MAXIMALE = 90;
    static constexpr int DUREE_VERTE_PAR_DEFAUT = 30;
    static constexpr int DUREE_ORANGE = 3;
    // Facteurs d'ajustement basés sur le niveau de trafic
    static constexpr double FACTEUR_TRAFIC_FAIBLE = 0.7; // 30% de réduction
    static constexpr double FACTEUR_TRAFIC_ELEVE = 1.5;  // 50% d'augmentation
    static constexpr double FACTEUR_EMBOUTEILLAGE = 2.0; // 100% d'augmentation

    // Mode d'urgence (pour ambulance/police)
    bool modeUrgence;
    std::string directionUrgence;

    // Mode nuit (feux clignotants quand il n'y a personne)
    bool modeNuit;
    float seuilModeNuit;   // Seuil de véhicules pour activer le mode nuit (ex: < 2 véhicules en moyenne)
    float minuterModeNuit; // Minuteur pour éviter que le mode Nuit ne s'active/désactive trop souvent

public:
    LogiqueAdaptative();

    // Fonctions de calcul des durées
    int calculerDureeVerte(const Capteur& capteur) const; // Calcule la durée pour une seule voie.
    int calculerDureeVerteEquilibree(const Capteur& capteur1, const Capteur& capteur2) const; // Durée pour deux voies opposées.
    int calculerScorePriorite(const Capteur& capteur) const; // Donne un score à une voie (longueur file + flux).
    int determinerPriorite(const std::vector<Capteur>& capteurs) const; // Trouve la voie avec le meilleur score.

    // Fonctions de gestion des modes spéciaux
    void activerModeUrgence(const std::string& direction);
    void desactiverModeUrgence();
    bool estModeUrgence() const;
    void activerModeNuit();
    void desactiverModeNuit();
    bool estModeNuit() const;
    bool doitActiverModeNuit(const std::vector<Capteur>& capteurs) const;
    void mettreAJourModeNuit(const std::vector<Capteur>& capteurs, float tempsEcoule);

    // Utilitaires
    std::string obtenirDirectionUrgence() const;
    static int obtenirDureeOrange();
    void afficherRecommandations(const std::vector<Capteur>& capteurs) const;
};


#endif 