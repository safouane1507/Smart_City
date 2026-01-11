#include "adaptivelogic.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// Constructeur : initialise les modes spéciaux
LogiqueAdaptative::LogiqueAdaptative()
    : modeUrgence(false), directionUrgence("")
    , modeNuit(false), seuilModeNuit(-1.0f), minuterModeNuit(0.0f) {
}

// Calcule la durée optimale du feu vert pour une voie
int LogiqueAdaptative::calculerDureeVerte(const Capteur& capteur) const {
    // Règle 1 : Modes spéciaux (Urgence ou Nuit)
    if (modeNuit) return 0; // 0 = feu orange clignotant
    if (modeUrgence) {
        bool estPrioritaire = capteur.obtenirIdentifiant().find(directionUrgence) != std::string::npos;
        return estPrioritaire ? DUREE_VERTE_MAXIMALE : DUREE_VERTE_MINIMALE;
    }

    // Règle 2 : Calcul basé sur le NIVEAU de trafic (grossier)
    std::string niveauTrafic = capteur.obtenirNiveauTrafic();
    int duree = DUREE_VERTE_PAR_DEFAUT;

    if (niveauTrafic == "FAIBLE") {
        duree = static_cast<int>(DUREE_VERTE_PAR_DEFAUT * FACTEUR_TRAFIC_FAIBLE);
    }
    else if (niveauTrafic == "ELEVE") {
        duree = static_cast<int>(DUREE_VERTE_PAR_DEFAUT * FACTEUR_TRAFIC_ELEVE);
    }
    else if (niveauTrafic == "EMBOUTEILLAGE") {
        duree = static_cast<int>(DUREE_VERTE_PAR_DEFAUT * FACTEUR_EMBOUTEILLAGE);
    }

    // Règle 3 : Ajustement fin basé sur la longueur EXACTE de la file
    int longueurFile = capteur.obtenirLongueurFile();
    duree += longueurFile / 3;  // On ajoute 1 seconde tous les 3 véhicules en file.

    // Règle 4 : Respecter les limites Min/Max
    duree = std::max(DUREE_VERTE_MINIMALE, std::min(duree, DUREE_VERTE_MAXIMALE));

    return duree;
}

// Calcule un score pour évaluer l'urgence d'une voie.
int LogiqueAdaptative::calculerScorePriorite(const Capteur& capteur) const {
    int score = 0;

    // 1. Longueur de la file (le plus important)
    score += capteur.obtenirLongueurFile() * 10; // Chaque voiture en file donne 10 points

    // 2. Bonus si embouteillage
    if (capteur.estEmbouteillage()) {
        score += 50;
    }

    // 3. Flux de trafic (combien de voitures arrivent)
    double flux = capteur.obtenirFluxTrafic();
    score += static_cast<int>(flux / 2);  // +1 point tous les 2 véh/min

    return score;
}

// Détermine quelle voie doit passer au vert.
int LogiqueAdaptative::determinerPriorite(const std::vector<Capteur>& capteurs) const {
    if (capteurs.empty()) {
        return -1;
    }

    // Priorité absolue en mode urgence
    if (modeUrgence) {
        // Recherche du capteur correspondant à la direction d'urgence
        for (size_t i = 0; i < capteurs.size(); i++) {
            if (capteurs[i].obtenirIdentifiant().find(directionUrgence) != std::string::npos) {
                return static_cast<int>(i); // Retourne l'index de la voie prioritaire
            }
        }
    }

    // Trouver le capteur avec le meilleur score de priorité
    int scoreMax = -1;
    int indexPriorite = 0;

    for (size_t i = 0; i < capteurs.size(); i++) {
        int score = calculerScorePriorite(capteurs[i]);
        if (score > scoreMax) {
            scoreMax = score;
            indexPriorite = static_cast<int>(i);
        }
    }

    return indexPriorite;
}


// Évalue si le trafic est trop faible pour justifier le mode nuit
bool LogiqueAdaptative::doitActiverModeNuit(const std::vector<Capteur>& capteurs) const {
    if (capteurs.empty()) return false;

    int totalVehicules = 0;
    for (const auto& capteur : capteurs) {
        totalVehicules += capteur.obtenirLongueurFile();
    }

    // Calcul de la moyenne des véhicules en attente par voie.
    float moyenneVehiculesParVoie = static_cast<float>(totalVehicules) / capteurs.size();

    // Si la moyenne est en dessous du seuil (ex: < 2 véhicules), on peut passer en mode nuit
    return false;
}

// Gère le passage en mode nuit ou le retour au mode normal.
void LogiqueAdaptative::mettreAJourModeNuit(const std::vector<Capteur>& capteurs, float tempsEcoule) {
    if (modeUrgence) return; // Ne rien faire si une urgence est en cours

    minuterModeNuit += tempsEcoule;

    // Vérifie toutes les 5 secondes (pour éviter les changements constants)
    if (minuterModeNuit >= 5.0f) {
        minuterModeNuit = 0.0f;

        bool doitEtreNuit = doitActiverModeNuit(capteurs);

        if (doitEtreNuit && !modeNuit) {
            activerModeNuit();
        }
        else if (!doitEtreNuit && modeNuit) {
            desactiverModeNuit();
        }
    }
}

// --- Fonctions d'activation/désactivation ---
void LogiqueAdaptative::activerModeUrgence(const std::string& direction) {
    modeUrgence = true;
    directionUrgence = direction;
    std::cout << "🚨 MODE URGENCE ACTIVÉ - Direction prioritaire: " << direction << std::endl;
}

void LogiqueAdaptative::desactiverModeUrgence() {
    modeUrgence = false;
    directionUrgence = "";
    std::cout << "✅ Mode urgence désactivé" << std::endl;
}

void LogiqueAdaptative::activerModeNuit() {
    modeNuit = true;
    std::cout << "🌙 MODE NUIT ACTIVÉ - Feux en orange clignotant" << std::endl;
}

void LogiqueAdaptative::desactiverModeNuit() {
    modeNuit = false;
    std::cout << "☀️  Mode nuit désactivé - Retour au mode normal" << std::endl;
}

// --- Autres accesseurs et affichage ---
bool LogiqueAdaptative::estModeUrgence() const { return modeUrgence; }
bool LogiqueAdaptative::estModeNuit() const { return modeNuit; }
int LogiqueAdaptative::obtenirDureeOrange() { return DUREE_ORANGE; }

void LogiqueAdaptative::afficherRecommandations(const std::vector<Capteur>& capteurs) const {
    // ... (affichage de l'état et des durées recommandées)
}