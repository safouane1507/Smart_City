#pragma once
#include "scenes/IScene.hpp"
#include "events/GameEvents.hpp"
#include "core/EventBus.hpp"
#include "ui/UIManager.hpp"

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
    explicit AdaptiveSignalsScene(std::shared_ptr<EventBus> bus, const AdaptiveSignalsConfig& config);
    virtual ~AdaptiveSignalsScene();

    void load() override;
    void unload() override;
    void update(double dt) override;
    void draw() override;

private:
    std::shared_ptr<EventBus> eventBus;
    UIManager uiManager;
    AdaptiveSignalsConfig adaptiveConfig;
    
    // adaptive-signals state
    Coordinator* coordinator = nullptr;
    LogiqueAdaptative* logiqueGlobal = nullptr;
    
    bool isInitialized = false;
    float currentSpeed = 1.0f;
    std::vector<Subscription> eventTokens;
};
