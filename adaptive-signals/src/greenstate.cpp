#include "greenstate.h"
#include "trafficlight.h"
#include "yellowstate.h"

void GreenState::next(TrafficLight& light) {
    light.setState(new YellowState());
}
