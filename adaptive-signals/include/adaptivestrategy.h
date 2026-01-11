#pragma once
#include "trafficstrategy.h"

class AdaptiveStrategy : public TrafficStrategy {
public:
    void update(TrafficLight& light) override;
};