#include "scenes/MapConfigScene.hpp"
#include "core/AssetManager.hpp"
#include "config.hpp"
#include "ui/UIButton.hpp"
#include <string>

MapConfigScene::MapConfigScene(std::shared_ptr<EventBus> bus) : eventBus(bus) {}

void MapConfigScene::load() {
  float cx = Config::LOGICAL_WIDTH / 2.0f;
  float cy = Config::LOGICAL_HEIGHT / 2.0f;
  float rowHeight = 50.0f;
  float spacing = 10.0f;
  float startY = cy - 150.0f;

  auto createCounter = [&](int &value, const std::string &label, float y) {
    float labelWidth = 250.0f;
    float btnSize = 40.0f;
    float gap = 10.0f;

    // Label Button (Display)
    // Centered X
    float totalW = labelWidth + 2 * (btnSize + gap);
    float startX = cx - totalW / 2.0f;

    // Decrease Button
    auto decBtn = std::make_shared<UIButton>(Vector2{startX, y}, Vector2{btnSize, rowHeight}, "<", eventBus);

    // Display Button (Label + Value)
    auto dispBtn = std::make_shared<UIButton>(Vector2{startX + btnSize + gap, y}, Vector2{labelWidth, rowHeight},
                                              label + ": " + std::to_string(value), eventBus);

    // Increase Button
    auto incBtn = std::make_shared<UIButton>(Vector2{startX + btnSize + gap + labelWidth + gap, y},
                                             Vector2{btnSize, rowHeight}, ">", eventBus);

    // Logic
    decBtn->setOnClick([&value, dispBtn, label]() {
      if (value > 0)
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

  createCounter(config.smallParkingCount, "Small Parking", startY);
  createCounter(config.largeParkingCount, "Large Parking", startY + (rowHeight + spacing));
  createCounter(config.smallChargingCount, "Small Charging", startY + 2 * (rowHeight + spacing));
  createCounter(config.largeChargingCount, "Large Charging", startY + 3 * (rowHeight + spacing));

  // Play Button
  float playBtnWidth = 200.0f;
  auto playBtn = std::make_shared<UIButton>(Vector2{cx - playBtnWidth / 2, startY + 4 * (rowHeight + spacing) + 20},
                                            Vector2{playBtnWidth, rowHeight}, "PLAY", eventBus);
  playBtn->setOnClick([this]() { eventBus->publish(SceneChangeEvent{SceneType::AdaptiveSignalsConfig, config}); });

  ui.add(playBtn);
}

void MapConfigScene::unload() {}

void MapConfigScene::update(double dt) { ui.update(dt); }

void MapConfigScene::draw() {
  Texture2D bg = AssetManager::Get().GetTexture("config_bg");
    DrawTexturePro(bg, 
        { 0, 0, (float)bg.width, (float)bg.height }, 
        { 0, 0, (float)Config::LOGICAL_WIDTH, (float)Config::LOGICAL_HEIGHT }, 
        { 0, 0 }, 0.0f, WHITE);
  ui.draw();
}
