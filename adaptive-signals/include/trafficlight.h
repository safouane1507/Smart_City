#pragma once
#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "raylib.h"
#include <string>

// ✅ Forward declarations (ONLY names)
class TrafficLightState;
class TrafficStrategy;
class Capteur;

enum class LightColors { Red, Green, Yellow };
enum class ModeFeux { Fixe, Adaptatif };

struct Timer {
    float lifetime;
};

class TrafficLight {
private:
    LightColors currentColor;
    ModeFeux mode;
    Capteur* capteur;
    Timer timer;

    // ✅ Design Patterns members
    TrafficLightState* state;
    TrafficStrategy* strategy;

public:
    TrafficLight(LightColors initialColor, ModeFeux m, Capteur* capteurAssocie = nullptr);

    void update();
    void setColor(LightColors c);
    LightColors getColor() const;

    void setMode(ModeFeux m);
    ModeFeux getMode() const;

    void setCapteur(Capteur* c);
    Capteur* getCapteur() const;

    float getRemainingTime() const;
    Color getRaylibColor() const;

    // 🔥 State / Strategy API
    void setState(TrafficLightState* s);
    void switchState();
    void extendGreen(float seconds);
};

#endif
