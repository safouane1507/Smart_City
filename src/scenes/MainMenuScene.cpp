#include "scenes/MainMenuScene.hpp"
#include "core/AssetManager.hpp"
#include "config.hpp"
#include "events/GameEvents.hpp"
#include "events/WindowEvents.hpp"
#include "ui/UIButton.hpp"

MainMenuScene::MainMenuScene(std::shared_ptr<EventBus> bus) : eventBus(bus) {}

void MainMenuScene::load() {
  float cx = Config::LOGICAL_WIDTH / 2.0f;
  float cy = Config::LOGICAL_HEIGHT / 2.0f;
  float btnWidth = 200.0f;
  float btnHeight = 50.0f;
  float spacing = 20.0f;

  // Create "START" button
  auto start =
      std::make_shared<UIButton>(Vector2{cx - btnWidth / 2, cy - btnHeight - spacing}, Vector2{btnWidth, btnHeight}, "START", eventBus);
  start->setOnClick([this]() { eventBus->publish(SceneChangeEvent{SceneType::MapConfig, {}}); });

  // Create "TRAFFIC" button
  auto traffic = std::make_shared<UIButton>(Vector2{cx - btnWidth / 2, cy},
                                         Vector2{btnWidth, btnHeight}, "TRAFFIC", eventBus);
  traffic->setOnClick([this]() { eventBus->publish(SceneChangeEvent{SceneType::AdaptiveSignals, {}}); });

  // Create "EXIT" button
  auto exit = std::make_shared<UIButton>(Vector2{cx - btnWidth / 2, cy + btnHeight + spacing},
                                         Vector2{btnWidth, btnHeight}, "EXIT", eventBus);
  exit->setOnClick([this]() { eventBus->publish(WindowCloseEvent{}); });

  ui.add(start);
  ui.add(traffic);
  ui.add(exit);
}
void MainMenuScene::unload() {}
void MainMenuScene::update(double dt) { ui.update(dt); }
void MainMenuScene::draw() {
  Texture2D bg = AssetManager::Get().GetTexture("menu_bg");
    DrawTexturePro(bg, 
        { 0, 0, (float)bg.width, (float)bg.height }, 
        { 0, 0, (float)Config::LOGICAL_WIDTH, (float)Config::LOGICAL_HEIGHT }, 
        { 0, 0 }, 0.0f, WHITE);
  ui.draw();
}
