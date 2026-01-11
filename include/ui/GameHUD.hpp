#pragma once

/**
 * @file GameHUD.hpp
 * @brief Main Heads-Up Display manager.
 */
#include "core/EventBus.hpp"
#include "events/GameEvents.hpp"
#include "ui/UIManager.hpp"
#include <memory>
#include <vector>

class EntityManager;

/**
 * @class GameHUD
 * @brief Manages top-level UI elements (Buttons, Overlay).
 *
 * Handles the composition of the user interface.
 */
class GameHUD {
public:
  GameHUD(std::shared_ptr<EventBus> bus, EntityManager *entityManager, const AdaptiveSignalsConfig& config);
  ~GameHUD();

  void update(double dt);
  void draw();

private:
  std::shared_ptr<EventBus> eventBus;
  UIManager uiManager;
  std::vector<Subscription> eventTokens;

  bool isPaused = false;
  double currentSpeed = 1.0;
  AdaptiveSignalsConfig adaptiveConfig;
};
