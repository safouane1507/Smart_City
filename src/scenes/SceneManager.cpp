#include "scenes/SceneManager.hpp"
#include "core/Logger.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/MapConfigScene.hpp"
#include "scenes/AdaptiveSignalsScene.hpp"

SceneManager::SceneManager(std::shared_ptr<EventBus> bus) : eventBus(bus) {
  // Subscribe to SceneChangeEvent to handle scene transitions requested by other components
  sceneChangeToken = eventBus->subscribe<SceneChangeEvent>([this](const SceneChangeEvent &e) {
    changeQueued = true;
    nextScene = e.newScene;
    nextConfig = e.config;
    Logger::Info("Scene Change Requested via EventBus");
  });
}

void SceneManager::update(double dt) {
  if (changeQueued) {
    setScene(nextScene);
    changeQueued = false;
  }
  if (currentScene)
    currentScene->update(dt);
}

void SceneManager::render() {
  if (currentScene)
    currentScene->draw();
}

void SceneManager::setScene(SceneType type) {
  if (currentScene) {
    currentScene->unload();
    Logger::Info("Scene Unloaded");
  }
  switch (type) {
  case SceneType::MainMenu:
    currentScene = std::make_unique<MainMenuScene>(eventBus);
    break;
  case SceneType::MapConfig:
    currentScene = std::make_unique<MapConfigScene>(eventBus);
    break;
    case SceneType::Game:
    currentScene = std::make_unique<GameScene>(eventBus, nextConfig);
    break;
  case SceneType::AdaptiveSignals:
    currentScene = std::make_unique<AdaptiveSignalsScene>(eventBus);
    break;
  }
  if (currentScene) {
    currentScene->load();
    Logger::Info("Scene Loaded");
  }
}
