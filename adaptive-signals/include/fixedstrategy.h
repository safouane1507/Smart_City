#pragma once
#include "trafficstrategy.h"

class FixedStrategy : public TrafficStrategy {
public:
    void update(TrafficLight& light) override;
};
