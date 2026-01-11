#include "scenes/SceneManager.hpp"
#include "core/Logger.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/MapConfigScene.hpp"
#include "scenes/AdaptiveSignalsScene.hpp"
#include "scenes/AdaptiveSignalsConfigScene.hpp"

SceneManager::SceneManager(std::shared_ptr<EventBus> bus) : eventBus(bus) {
  // Subscribe to SceneChangeEvent to handle scene transitions requested by other components
  sceneChangeToken = eventBus->subscribe<SceneChangeEvent>([this](const SceneChangeEvent &e) {
    changeQueued = true;
    nextScene = e.newScene;
    nextConfig = e.config;
    nextAdaptiveConfig = e.adaptiveConfig;
    Logger::Info("Scene Change Requested via EventBus: {}", (int)nextScene);
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
  // Determine if the current scene should be kept in persistence
  bool isPersistent = (type == SceneType::Game || type == SceneType::AdaptiveSignals);

  // If we are moving to a configuration/menu scene, we should clear persistence
  if (type == SceneType::MainMenu || type == SceneType::MapConfig || type == SceneType::AdaptiveSignalsConfig) {
    for (auto& pair : persistentScenes) {
        pair.second->unload();
    }
    persistentScenes.clear();
    if (nonPersistentScene) {
        nonPersistentScene->unload();
        nonPersistentScene.reset();
    }
    currentScene = nullptr;
  }

  if (isPersistent) {
    // If it's a persistent scene, check if it already exists
    if (persistentScenes.find(type) == persistentScenes.end()) {
      if (type == SceneType::Game) {
        persistentScenes[type] = std::make_unique<GameScene>(eventBus, nextConfig, nextAdaptiveConfig);
      } else if (type == SceneType::AdaptiveSignals) {
        persistentScenes[type] = std::make_unique<AdaptiveSignalsScene>(eventBus, nextAdaptiveConfig);
      }
      persistentScenes[type]->load();
      Logger::Info("Persistent Scene Created and Loaded: {}", (int)type);
    }
    currentScene = persistentScenes[type].get();
    // No need to call load() again if it was already loaded
  } else {
    // Clean up old non-persistent scene
    if (nonPersistentScene) {
        nonPersistentScene->unload();
        nonPersistentScene.reset();
    }

    switch (type) {
    case SceneType::MainMenu:
      nonPersistentScene = std::make_unique<MainMenuScene>(eventBus);
      break;
    case SceneType::MapConfig:
      nonPersistentScene = std::make_unique<MapConfigScene>(eventBus);
      break;
    case SceneType::AdaptiveSignalsConfig:
      nonPersistentScene = std::make_unique<AdaptiveSignalsConfigScene>(eventBus, nextConfig);
      break;
    default:
      break;
    }

    if (nonPersistentScene) {
      nonPersistentScene->load();
      currentScene = nonPersistentScene.get();
      Logger::Info("Non-Persistent Scene Loaded: {}", (int)type);
    }
  }
}
