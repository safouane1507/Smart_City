#pragma once
#ifndef intersectioncontroller_h
#define intersectioncontroller_h

#include <iostream>
#include <vector>
#include <string>
#include "trafficlight.h"
#include "sensorr.h"
#include "adaptivelogic.h"
using namespace std;

class TrafficLight;
class Capteur;
class LogiqueAdaptative;

enum class Direction {
    NorthSouth,
    EastWest
};

enum class IntersectionPhase { // les coulores d'intersection dans une phase
    NS_Green,
    NS_Yellow,
    All_Red_Switch_TO_EW,
    // NS=north-south , EW=east-west
    EW_Green,
    EW_Yellow,
    All_Red_Switch_TO_NS
};

class IntersectionController {
public:
    string id_intersection;  //  c'est l'identificatrur d'une intersetion
    vector<TrafficLight*> LightsNS;
    vector<TrafficLight*> LightsEW;

    LogiqueAdaptative* Logique;

    bool adaptiveMode;   // false => mode fixe , true => mode adaptive
    float Timer;   // decompter la duree de la phase actuelle
    IntersectionPhase currentPhase;
    void SetGroupColor(vector<TrafficLight*>& group, LightColors colors);
    float calculateGroupDuration(const vector<TrafficLight*>& group);

public:
    IntersectionController(string id, LogiqueAdaptative* logic);

    void addTrafficLight(TrafficLight* light, Direction direction);

    void update(float deltaTime);
    void switchMode(bool adaptive);

};


#endif