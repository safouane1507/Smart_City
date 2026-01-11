#pragma once

class TrafficLight;

class TrafficLightState {
public:
    virtual ~TrafficLightState() = default;
    virtual void next(TrafficLight& light) = 0;
    virtual float duration() const = 0;
};
