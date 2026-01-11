#include "sensorr.h"
#include <iostream>
#include <iomanip>
#include "raylib.h"
#include <vector>

// =====================
// Classe Vehicule (mock)
// =====================
class Vehicule {
public:
    double obtenirPosition() const { return 0.0; }
    bool estArrete() const { return true; }
};

// =====================
// Définition des seuils
// =====================
constexpr int SEUIL_TRAFIC_FAIBLE = 3;
constexpr int SEUIL_TRAFIC_ELEVE = 8;
constexpr int SEUIL_EMBOUTEILLAGE = 12;

// =====================
// Constructeur
// =====================
Capteur::Capteur(const std::string& id)
    : identifiant(id),
    nombreVehicules(0),
    longueurFileActuelle(0),
    vehiculesDepuisDerniereReinitialisation(0)
{
    tempsReinitialisationDerniere = GetTime();

    positionX = 0.0f;
    positionY = 0.0f;
    rayonDetection = 0.0f;
    directionSurveillee = "";
}

// =====================
// Détection véhicule
// =====================
void Capteur::detecterVehicule() {
    nombreVehicules++;
    vehiculesDepuisDerniereReinitialisation++;
}

// =====================
// Flux de trafic
// =====================
double Capteur::obtenirFluxTrafic() const {
    double maintenant = GetTime();
    double tempsEcoule = maintenant - tempsReinitialisationDerniere;

    if (tempsEcoule < 0.0001) {
        return 0.0;
    }

    return (vehiculesDepuisDerniereReinitialisation / tempsEcoule) * 60.0;
}

// =====================
// Réinitialisation flux
// =====================
void Capteur::reinitialiser() {
    tempsReinitialisationDerniere = GetTime();
    vehiculesDepuisDerniereReinitialisation = 0;
}

// =====================
// Calcul file d’attente
// =====================
void Capteur::calculerLongueurFile(const std::vector<Vehicule*>& listeVehicules) {
    int file = 0;

    const double POSITION_FEU = 5.0;
    const double LONGUEUR_ZONE_FILE = 80.0;
    const double POSITION_DEBUT_FILE = POSITION_FEU - LONGUEUR_ZONE_FILE;

    for (const auto& vehicule : listeVehicules) {
        double position = vehicule->obtenirPosition();

        if (position >= POSITION_DEBUT_FILE && position < POSITION_FEU &&
            vehicule->estArrete()) {
            file++;
        }
    }

    mettreAJourFile(file);
}

// =====================
// Mise à jour file
// =====================
void Capteur::mettreAJourFile(int longueurFile) {
    if (longueurFile >= 0) {
        longueurFileActuelle = longueurFile;
    }
}

// =====================
// Getters
// =====================
int Capteur::obtenirLongueurFile() const {
    return longueurFileActuelle;
}

int Capteur::obtenirNombreTotalVehicules() const {
    return nombreVehicules;
}

std::string Capteur::obtenirIdentifiant() const {
    return identifiant;
}

// =====================
// Analyse trafic
// =====================
bool Capteur::estEmbouteillage() const {
    return longueurFileActuelle >= SEUIL_EMBOUTEILLAGE;
}

std::string Capteur::obtenirNiveauTrafic() const {
    if (estEmbouteillage()) {
        return "EMBOUTEILLAGE";
    }
    if (longueurFileActuelle >= SEUIL_TRAFIC_ELEVE) {
        return "ELEVE";
    }
    if (longueurFileActuelle >= SEUIL_TRAFIC_FAIBLE) {
        return "MOYEN";
    }
    return "FAIBLE";
}

// =====================
// Affichage console
// =====================
void Capteur::afficherEtat() const {
    std::cout << "=== Capteur: " << identifiant << " ===\n";
    std::cout << "  Véhicules total: " << nombreVehicules << "\n";
    std::cout << "  File d'attente: " << longueurFileActuelle << " véhicules\n";
    std::cout << "  Flux: " << std::fixed << std::setprecision(1)
        << obtenirFluxTrafic() << " véh/min\n";
    std::cout << "  Niveau: " << obtenirNiveauTrafic() << "\n\n";
}
