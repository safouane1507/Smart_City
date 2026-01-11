#pragma once
#include "core/EventBus.hpp"
#include "events/GameEvents.hpp"
#include "scenes/IScene.hpp"
#include <memory>
#include <set>
#include <vector>

class TrackingSystem;
class GameScene : public IScene {
public:
  explicit GameScene(std::shared_ptr<EventBus> bus, MapConfig config, AdaptiveSignalsConfig adaptiveConfig);
  ~GameScene() override;

  void load() override;
  void unload() override;
  void update(double dt) override;
  void draw() override;

private:
  void handleInput();
  std::unique_ptr<TrackingSystem> trackingSystem;
  std::shared_ptr<EventBus> eventBus;
  std::vector<Subscription> eventTokens;

  std::unique_ptr<class EntityManager> entityManager;
  std::unique_ptr<class TrafficSystem> trafficSystem;
  std::unique_ptr<class GameHUD> gameHUD;

  std::unique_ptr<class CameraSystem> cameraSystem;
  bool isPaused = false;
  MapConfig config;
  AdaptiveSignalsConfig adaptiveConfig;
  std::set<int> keysDown;
};
