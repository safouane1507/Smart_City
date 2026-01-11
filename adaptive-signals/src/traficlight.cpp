#include "trafficlight.h"

#include "trafficlightstate.h"
#include "trafficstrategy.h"

#include "redstate.h"
#include "greenstate.h"
#include "yellowstate.h"

#include "fixedstrategy.h"
#include "adaptivestrategy.h"

#include "sensorr.h"

// ================= CONSTRUCTOR =================

TrafficLight::TrafficLight(LightColors initialColor, ModeFeux m, Capteur* c)
    : currentColor(initialColor), capteur(c)
{
    // Initial state
    state = new RedState();

    // Strategy selection
    strategy = (m == ModeFeux::Fixe)
        ? static_cast<TrafficStrategy*>(new FixedStrategy())
        : static_cast<TrafficStrategy*>(new AdaptiveStrategy());

    timer.lifetime = state->duration();
}

// ================= UPDATE =================

void TrafficLight::update() {
    timer.lifetime -= GetFrameTime();

    if (timer.lifetime <= 0) {
        strategy->update(*this);
        timer.lifetime = state->duration();
    }
}

// ================= COLOR API =================

void TrafficLight::setColor(LightColors c) {
    currentColor = c;
}

LightColors TrafficLight::getColor() const {
    return currentColor;
}

Color TrafficLight::getRaylibColor() const {
    if (currentColor == LightColors::Red) return RED;
    if (currentColor == LightColors::Green) return GREEN;
    return YELLOW;
}

// ================= MODE / SENSOR =================

void TrafficLight::setMode(ModeFeux m) {
    mode = m;
}

ModeFeux TrafficLight::getMode() const {
    return mode;
}

void TrafficLight::setCapteur(Capteur* c) {
    capteur = c;
}

Capteur* TrafficLight::getCapteur() const {
    return capteur;
}

// ================= TIMER =================

float TrafficLight::getRemainingTime() const {
    return (timer.lifetime < 0) ? 0 : timer.lifetime;
}

// ================= STATE PATTERN =================

void TrafficLight::setState(TrafficLightState* s) {
    delete state;
    state = s;
}

void TrafficLight::switchState() {
    state->next(*this);
}

// ================= STRATEGY HELPER =================

void TrafficLight::extendGreen(float seconds) {
    timer.lifetime += seconds;
}

// ================= LEGACY SUPPORT (OPTIONAL) =================
// You can KEEP this if other code still uses it


