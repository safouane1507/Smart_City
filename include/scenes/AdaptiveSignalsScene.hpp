#pragma once
#include "scenes/IScene.hpp"
#include "core/EventBus.hpp"
#include "raylib.h"
#include <memory>
#include <vector>

// Forward declarations for adaptive-signals classes
class Coordinator;
class LogiqueAdaptative;
struct Car;

/**
 * @class AdaptiveSignalsScene
 * @brief Integrated scene from the adaptive-signals project.
 */
class AdaptiveSignalsScene : public IScene {
public:
    explicit AdaptiveSignalsScene(std::shared_ptr<EventBus> bus);
    virtual ~AdaptiveSignalsScene();

    void load() override;
    void unload() override;
    void update(double dt) override;
    void draw() override;

private:
    std::shared_ptr<EventBus> eventBus;
    
    // adaptive-signals state
    Coordinator* coordinator = nullptr;
    LogiqueAdaptative* logiqueGlobal = nullptr;
    // We'll use a local struct/class for Car if needed, or include the header
    // For now, let's assume we can include the headers from adaptive-signals
    
    // We need to store the cars here as they were static in the original code
    // But since this is a class, member variables are better
    // Note: I'll need to make sure the Car struct is accessible.
    // In the original main.cpp, it was defined there.
    // I should probably move it to a header in adaptive-signals/include.
};
