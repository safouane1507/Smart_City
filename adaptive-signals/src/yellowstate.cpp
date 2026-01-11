#include "yellowstate.h"
#include "trafficlight.h"
#include "redstate.h"

void YellowState::next(TrafficLight& light) {
    light.setState(new RedState());
}
