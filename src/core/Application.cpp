#include "core/Application.hpp"
#include "core/AssetManager.hpp"
#include "core/AudioManager.hpp"
#include "core/Logger.hpp"
#include "events/GameEvents.hpp"
#include "events/WindowEvents.hpp"

/**
 * @file Application.cpp
 * @brief Implementation of the main Application class.
 *
 * Handles the initialization of the game engine, the main run loop,
 * and high-level event management (e.g., window closing).
 */

Application::Application() {
  Logger::Info("Application Starting...");

  InitAudioDevice();

  // Initialize core systems
  eventBus = std::make_shared<EventBus>();
  window = std::make_unique<Window>(eventBus);
  inputSystem = std::make_unique<InputSystem>(eventBus, *window);
  sceneManager = std::make_unique<SceneManager>(eventBus);
  eventLogger = std::make_unique<EventLogger>(eventBus);
  gameLoop = std::make_unique<GameLoop>();

  // Centralized asset loading
  AssetManager::Get().LoadAllAssets();

  // Initialize Audio
  AudioManager::Get().PlayMusic("bg_music");
  AudioManager::Get().InitUI(eventBus);

  // Start with the main menu
  sceneManager->setScene(SceneType::MainMenu);

  // Subscribe to the WindowCloseEvent to stop the application loop
  closeEventToken = eventBus->subscribe<WindowCloseEvent>([this](const WindowCloseEvent &) {
    Logger::Info("Window Close Event Received - Stopping Loop");
    isRunning = false;
  });

  // Subscribe to Simulation Speed Changes
  eventTokens.push_back(eventBus->subscribe<SimulationSpeedChangedEvent>(
      [this](const SimulationSpeedChangedEvent &e) { gameLoop->setSpeedMultiplier(e.speedMultiplier); }));

  // Subscribe to Scene Changes to reset speed when moving to menus
  eventTokens.push_back(eventBus->subscribe<SceneChangeEvent>([this](const SceneChangeEvent &e) {
    bool isMenu = (e.newScene == SceneType::MainMenu || 
                   e.newScene == SceneType::MapConfig || 
                   e.newScene == SceneType::AdaptiveSignalsConfig);
    if (isMenu) {
      gameLoop->setSpeedMultiplier(1.0);
      eventBus->publish(SimulationSpeedChangedEvent{1.0});
    }
  }));
}

Application::~Application() {
  CloseAudioDevice();
  Logger::Info("Application Stopped Safely");
}

void Application::run() {
  gameLoop->run(
      [this](double dt) {
        AudioManager::Get().Update();
        this->update(dt);
      },
      [this]() { this->render(); }, [this]() { return isRunning; });
}

void Application::update(double dt) { sceneManager->update(dt); }

void Application::render() {
  if (window->shouldClose()) {
    eventBus->publish(WindowCloseEvent{});
  }
  inputSystem->update();

  window->beginDrawing();
  sceneManager->render();

  AudioManager::Get().DrawUI();
  window->endDrawing();
}
