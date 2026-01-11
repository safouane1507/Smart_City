#include <gtest/gtest.h>
#include "scenes/GameScene.hpp"
#include "core/Window.hpp"
#include "events/GameEvents.hpp"
#include <memory>

class GameSceneTests : public ::testing::Test {
protected:
    std::shared_ptr<EventBus> bus;
    std::unique_ptr<Window> window;
    
    // We need a window context for GameScene because it (or its children) might load assets or use Raylib functions.
    void SetUp() override {
        bus = std::make_shared<EventBus>();
        // Window constructor InitWindow().
        // Note: multiple tests initializing window might flicker or be slow, but it's safe if sequential.
        window = std::make_unique<Window>(bus);
    }

    void TearDown() override {
        window.reset(); // CloseWindow()
    }
};

TEST_F(GameSceneTests, CanInstantiateAndLoad) {
    MapConfig config;
    // Setup minimal valid config if needed
    config.smallParkingCount = 1;


    AdaptiveSignalsConfig adaptiveConfig;
    auto scene = std::make_unique<GameScene>(bus, config, adaptiveConfig);
    
    EXPECT_NO_THROW(scene->load());
    
    // Update once to see if it crashes
    EXPECT_NO_THROW(scene->update(0.1));
    
    // Draw once
    // Note: this requires active OpenGL context, which Window provides.
    EXPECT_NO_THROW(scene->draw());
    
    EXPECT_NO_THROW(scene->unload());
}
