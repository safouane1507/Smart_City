#pragma once
#include "events/GameEvents.hpp"
#include "scenes/IScene.hpp"
#include "ui/UIManager.hpp"

class AdaptiveSignalsConfigScene : public IScene {
public:
  explicit AdaptiveSignalsConfigScene(std::shared_ptr<EventBus> bus, const MapConfig& prevConfig);

  void load() override;
  void unload() override;
  void update(double dt) override;
  void draw() override;

private:
  std::shared_ptr<EventBus> eventBus;
  UIManager ui;
  MapConfig mapConfig;
  AdaptiveSignalsConfig adaptiveConfig;
};
