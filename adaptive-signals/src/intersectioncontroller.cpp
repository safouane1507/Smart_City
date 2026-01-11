#include<iostream>
#include"intersectioncontroller.h"
#include "trafficlight.h"
#include "sensorr.h"
#include "adaptivelogic.h"
#include <vector>
#include <string>
#include<vector>
#include<string>
#include<math.h>
using namespace std;

//constructeur
IntersectionController::IntersectionController(string id, LogiqueAdaptative* logic) {
    this->id_intersection = id;
    this->Logique = logic;
    this->adaptiveMode = false;    // initialisation par le mode fix
    this->currentPhase = IntersectionPhase::All_Red_Switch_TO_NS;
    this->Timer = 2.0f;
}

void IntersectionController::addTrafficLight(TrafficLight* light, Direction direction) {
    if (direction == Direction::NorthSouth) {
        this->LightsNS.push_back(light);
    }
    else {
        this->LightsEW.push_back(light);
    }
}

void IntersectionController::SetGroupColor(vector<TrafficLight*>& group, LightColors colors) {
    for (auto light : group) {
        light->setColor(colors);
    }
}

float IntersectionController::calculateGroupDuration(const vector<TrafficLight*>& group) {
    if (!adaptiveMode || group.empty()) {
        return 10.0f; //duree fix de 10 sec
    }
    int maxDuration = 10;
    for (auto light : group) {
        Capteur* capteur = light->getCapteur();

        if (capteur != nullptr) {
            int duree = Logique->calculerDureeVerte(*capteur);
            if (duree > maxDuration) {
                maxDuration = duree;
            }
        }

    }
    return(float)maxDuration;
}


void IntersectionController::switchMode(bool adaptive) {
    this->adaptiveMode = adaptive;
    for (auto light : LightsNS) light->setMode(adaptive ? ModeFeux::Adaptatif : ModeFeux::Fixe);
    for (auto light : LightsEW) light->setMode(adaptive ? ModeFeux::Adaptatif : ModeFeux::Fixe);

    cout << "Intersection " << id_intersection << " Mode: "
        << (adaptiveMode ? "Adaptatif" : "Fixe") << endl;
}

void IntersectionController::update(float deltaTime) {
    vector<Capteur> allCapteurs;
    for (auto light : LightsNS) {
        if (light->getCapteur() != nullptr) allCapteurs.push_back(*light->getCapteur());
    }
    for (auto light : LightsEW) {
        if (light->getCapteur() != nullptr) allCapteurs.push_back(*light->getCapteur());
    }
    Logique->mettreAJourModeNuit(allCapteurs, deltaTime);
    //gestion de mode nuit
    if (Logique->estModeNuit()) {
        SetGroupColor(LightsNS, LightColors::Yellow);
        SetGroupColor(LightsEW, LightColors::Yellow);
        return; // stop le cycle normal de phases
    }
    this->Timer -= deltaTime;
    if (this->Timer <= 0.0f) {
        switch (currentPhase) {
        case IntersectionPhase::All_Red_Switch_TO_NS:
            currentPhase = IntersectionPhase::NS_Green;
            SetGroupColor(LightsNS, LightColors::Green);
            SetGroupColor(LightsEW, LightColors::Red);

            this->Timer = calculateGroupDuration(LightsNS);
            cout << "Intersection " << id_intersection << " -> NS_Green. Duree: " << this->Timer << "s" << endl;
            break;
        case IntersectionPhase::NS_Green:
            currentPhase = IntersectionPhase::NS_Yellow;
            SetGroupColor(LightsNS, LightColors::Yellow);
            this->Timer = (float)LogiqueAdaptative::obtenirDureeOrange();
            cout << "Intersection " << id_intersection << " ->NS_Yellow. Duree: " << this->Timer << "s" << endl;
            break;
        case IntersectionPhase::NS_Yellow:
            // Transition vers Tout Rouge (avant EW)
            currentPhase = IntersectionPhase::All_Red_Switch_TO_EW;
            SetGroupColor(LightsNS, LightColors::Red);
            // Les feux EW restent Rouges

            // Dur�e du Tout Rouge (par exemple 2 secondes, comme la valeur initiale)
            this->Timer = 2.0f;
            cout << "Intersection " << id_intersection << " -> All_Red_Switch_TO_EW. Duree: " << this->Timer << "s" << endl;
            break;

        case IntersectionPhase::All_Red_Switch_TO_EW:
            // Transition vers le Vert EW
            currentPhase = IntersectionPhase::EW_Green;
            SetGroupColor(LightsEW, LightColors::Green);
            // Les feux NS restent Rouges

            // Calcul de la duree Verte EW (adaptative ou fixe)
            this->Timer = calculateGroupDuration(LightsEW);
            cout << "Intersection " << id_intersection << " -> EW_Green. Duree: " << this->Timer << "s" << endl;
            break;

        case IntersectionPhase::EW_Green:
            // Transition vers l'Orange EW
            currentPhase = IntersectionPhase::EW_Yellow;
            SetGroupColor(LightsEW, LightColors::Yellow);
            // Les feux NS restent Rouges

            // Dur�e de l'Orange (fixe, depuis LogiqueAdaptative)
            this->Timer = (float)LogiqueAdaptative::obtenirDureeOrange();
            cout << "Intersection " << id_intersection << " -> EW_Yellow. Duree: " << this->Timer << "s" << endl;
            break;

        case IntersectionPhase::EW_Yellow:
            // Transition vers Tout Rouge (avant NS)
            currentPhase = IntersectionPhase::All_Red_Switch_TO_NS;
            SetGroupColor(LightsEW, LightColors::Red);
            // Les feux NS restent Rouges

            // Duree du Tout Rouge (par exemple 2 secondes)
            this->Timer = 2.0f;
            cout << "Intersection " << id_intersection << " -> All_Red_Switch_TO_NS. Duree: " << this->Timer << "s" << endl;
            break;

        }

    }
}