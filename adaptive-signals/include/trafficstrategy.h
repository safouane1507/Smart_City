#pragma once

class TrafficLight;

class TrafficStrategy {
public:
    virtual ~TrafficStrategy() = default;
    virtual void update(TrafficLight& light) = 0;
};