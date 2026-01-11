#pragma once
#include "trafficlightstate.h"

class YellowState : public TrafficLightState {
public:
    void next(TrafficLight& light) override;
    float duration() const override { return 2.0f; }
};
