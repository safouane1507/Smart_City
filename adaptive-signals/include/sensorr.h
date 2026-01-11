#pragma once
#ifndef SENSOR_H
#define SENSOR_H

#include <string>
#include <vector>


class Vehicule;

//  Classe Capteur : Simule un détecteur de trafic sur une voie
class Capteur {
private:
    std::string identifiant;
    int nombreVehicules;
    int longueurFileActuelle;
    double tempsReinitialisationDerniere; // Moment du dernier reset pour le calcul du flux
    int vehiculesDepuisDerniereReinitialisation; // Compteur de véhicules utilisés uniquement pour le calcul du flux
    float positionX;
    float positionY;
    float rayonDetection;
    std::string directionSurveillee;
    // Seuils de détection pour déterminer le niveau de trafic
    static const int SEUIL_EMBOUTEILLAGE = 15; // > 15 véhicules = embouteillage
    static const int SEUIL_TRAFIC_ELEVE = 10;  // > 10 véhicules = trafic élevé
    static const int SEUIL_TRAFIC_FAIBLE = 3;  // > 3 véhicules = trafic moyen (entre 3 et 10)

public:
    // Constructeur
    Capteur(const std::string& id);
    // Méthodes principales
    void detecterVehicule();          // Un véhicule vient de passer
    void calculerLongueurFile(const std::vector<Vehicule*>& listeVehicules);
    void mettreAJourFile(int longueurFile); // Mettre à jour le nombre de voitures en attente
    double obtenirFluxTrafic() const; // Calculer le flux de véhicules (véh/min)
    int obtenirLongueurFile() const;
    bool estEmbouteillage() const;
    std::string obtenirNiveauTrafic() const; // Renvoie "FAIBLE", "MOYEN", "ELEVE", ou "EMBOUTEILLAGE"
    // Autres utilitaires
    int obtenirNombreTotalVehicules() const;
    void reinitialiser(); // Reset le compteur de flux et le temps (pour un nouveau cycle de calcul)
    std::string obtenirIdentifiant() const;
    void afficherEtat() const;

};

#endif 
