#include "scenes/AdaptiveSignalsConfigScene.hpp"
#include "core/AssetManager.hpp"
#include "config.hpp"
#include "ui/UIButton.hpp"
#include <string>

AdaptiveSignalsConfigScene::AdaptiveSignalsConfigScene(std::shared_ptr<EventBus> bus, const MapConfig& prevConfig) 
    : eventBus(bus), mapConfig(prevConfig) {}

void AdaptiveSignalsConfigScene::load() {
  float cx = Config::LOGICAL_WIDTH / 2.0f;
  float cy = Config::LOGICAL_HEIGHT / 2.0f;
  float rowHeight = 50.0f;
  float spacing = 20.0f;
  float startY = cy - 100.0f;

  auto createCounter = [&](int &value, const std::string &label, float y) {
    float labelWidth = 250.0f;
    float btnSize = 40.0f;
    float gap = 10.0f;

    float totalW = labelWidth + 2 * (btnSize + gap);
    float startX = cx - totalW / 2.0f;

    auto decBtn = std::make_shared<UIButton>(Vector2{startX, y}, Vector2{btnSize, rowHeight}, "<", eventBus);
    auto dispBtn = std::make_shared<UIButton>(Vector2{startX + btnSize + gap, y}, Vector2{labelWidth, rowHeight},
                                              label + ": " + std::to_string(value), eventBus);
    auto incBtn = std::make_shared<UIButton>(Vector2{startX + btnSize + gap + labelWidth + gap, y},
                                             Vector2{btnSize, rowHeight}, ">", eventBus);

    decBtn->setOnClick([&value, dispBtn, label]() {
      if (value > 1)
        value--;
      dispBtn->setText(label + ": " + std::to_string(value));
    });

    incBtn->setOnClick([&value, dispBtn, label]() {
      if (value < 5)
        value++;
      dispBtn->setText(label + ": " + std::to_string(value));
    });

    ui.add(decBtn);
    ui.add(dispBtn);
    ui.add(incBtn);
  };

  createCounter(adaptiveConfig.rows, "Traffic Rows", startY);
  createCounter(adaptiveConfig.cols, "Traffic Cols", startY + (rowHeight + spacing));

  // Start Button
  float startBtnWidth = 200.0f;
  auto startBtn = std::make_shared<UIButton>(Vector2{cx - startBtnWidth / 2, startY + 2 * (rowHeight + spacing) + 40},
                                            Vector2{startBtnWidth, rowHeight}, "START", eventBus);
  startBtn->setOnClick([this]() { 
    eventBus->publish(SceneChangeEvent{SceneType::Game, mapConfig, adaptiveConfig}); 
  });

  ui.add(startBtn);
}

void AdaptiveSignalsConfigScene::unload() {}

void AdaptiveSignalsConfigScene::update(double dt) { ui.update(dt); }

void AdaptiveSignalsConfigScene::draw() {
  Texture2D bg = AssetManager::Get().GetTexture("config_bg");
  DrawTexturePro(bg, 
      { 0, 0, (float)bg.width, (float)bg.height }, 
      { 0, 0, (float)Config::LOGICAL_WIDTH, (float)Config::LOGICAL_HEIGHT }, 
      { 0, 0 }, 0.0f, WHITE);
  
  const char* title = "TRAFFIC SIMULATION CONFIG";
  int fontSize = 30;
  DrawText(title, Config::LOGICAL_WIDTH / 2 - MeasureText(title, fontSize) / 2, 50, fontSize, WHITE);
  
  ui.draw();
}
