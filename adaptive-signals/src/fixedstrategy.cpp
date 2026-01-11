#include "fixedstrategy.h"
#include "trafficlight.h"

void FixedStrategy::update(TrafficLight& light) {
    light.switchState();
}