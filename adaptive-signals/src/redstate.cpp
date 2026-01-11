#include "redstate.h"
#include "trafficlight.h"
#include "greenstate.h"

void RedState::next(TrafficLight& light) {
    light.setState(new GreenState());
}
