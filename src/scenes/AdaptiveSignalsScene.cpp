#include "scenes/AdaptiveSignalsScene.hpp"
#include "coordinator.h"
#include "intersectioncontroller.h"
#include "trafficlight.h"
#include "sensorr.h"
#include "adaptivelogic.h"
#include "car_utils.h"
#include "events/GameEvents.hpp"
#include "core/Logger.hpp"
#include "config.hpp"
#include "ui/UIButton.hpp"
#include <string>

void DrawTrafficLightRealistic(float x, float y, Color activeColor, bool vertical) {
    DrawRectangle((int)x, (int)y, 14, 34, BLACK);
    DrawRectangleLines((int)x, (int)y, 14, 34, GRAY);
    DrawCircle((int)x + 7, (int)y + 7, 4, (activeColor.r > 200 && activeColor.g < 100) ? RED : DARKGRAY);
    DrawCircle((int)x + 7, (int)y + 17, 4, (activeColor.r > 200 && activeColor.g > 150) ? GOLD : DARKGRAY);
    DrawCircle((int)x + 7, (int)y + 27, 4, (activeColor.g > 200) ? GREEN : DARKGRAY);
}

void DrawCrosswalk(float x, float y, float width, float height, bool horizontal) {
    int numStripes = 6;
    if (horizontal) {
        float stripeWidth = width / (numStripes * 2 - 1);
        for (int i = 0; i < numStripes; i++) {
            DrawRectangle((int)(x + (i * 2 * stripeWidth)), (int)y, (int)stripeWidth, (int)height, WHITE);
        }
    }
    else {
        float stripeHeight = height / (numStripes * 2 - 1);
        for (int i = 0; i < numStripes; i++) {
            DrawRectangle((int)x, (int)(y + (i * 2 * stripeHeight)), (int)width, (int)stripeHeight, WHITE);
        }
    }
}

static std::vector<Car> cars;
static float spawnTimer = 0;
static bool isNightMode = false;

AdaptiveSignalsScene::AdaptiveSignalsScene(std::shared_ptr<EventBus> bus, const AdaptiveSignalsConfig& config) 
    : eventBus(bus), adaptiveConfig(config) {}

AdaptiveSignalsScene::~AdaptiveSignalsScene() {
    unload();
}

void AdaptiveSignalsScene::load() {
    if (isInitialized) return;
    
    Logger::Info("Loading AdaptiveSignalsScene with {}x{} grid", adaptiveConfig.rows, adaptiveConfig.cols);
    coordinator = new Coordinator();
    logiqueGlobal = new LogiqueAdaptative();
    
    int totalIntersections = adaptiveConfig.rows * adaptiveConfig.cols;
    for (int i = 0; i < totalIntersections; i++) {
        std::string id = "Inter_" + std::to_string(i);
        IntersectionController* inter = new IntersectionController(id, logiqueGlobal);
        inter->addTrafficLight(new TrafficLight(LightColors::Red, ModeFeux::Fixe, new Capteur("S_NS" + id)), Direction::NorthSouth);
        inter->addTrafficLight(new TrafficLight(LightColors::Green, ModeFeux::Fixe, new Capteur("S_EW" + id)), Direction::EastWest);
        coordinator->addIntersection(inter);
    }
    
    cars.clear();
    spawnTimer = 0;
    isNightMode = false;

    // Back button
    auto backBtn = std::make_shared<UIButton>(Vector2{10, 100}, Vector2{150, 40}, "Back to Game", eventBus);
    backBtn->setOnClick([this]() { eventBus->publish(SceneChangeEvent{SceneType::Game, {}}); });
    uiManager.add(backBtn);

    isInitialized = true;
}

void AdaptiveSignalsScene::unload() {
    Logger::Info("Force unloading AdaptiveSignalsScene");
    if (coordinator) {
        delete coordinator;
        coordinator = nullptr;
    }
    if (logiqueGlobal) {
        delete logiqueGlobal;
        logiqueGlobal = nullptr;
    }
    cars.clear();
    isInitialized = false;
}

void AdaptiveSignalsScene::update(double dt) {
    uiManager.update(dt);

    if (IsKeyPressed(KEY_N)) isNightMode = !isNightMode;

    if (IsKeyPressed(KEY_A)) {
        for (auto inter : coordinator->intersections) {
            ModeFeux nouveauMode = (inter->LightsNS[0]->getMode() == ModeFeux::Fixe) ? ModeFeux::Adaptatif : ModeFeux::Fixe;
            inter->LightsNS[0]->setMode(nouveauMode);
            inter->LightsEW[0]->setMode(nouveauMode);
        }
    }

    if (IsKeyPressed(KEY_M)) {
        eventBus->publish(SceneChangeEvent{SceneType::MainMenu, {}});
        return;
    }

    float roadWidth = 80.0f;
    float tile = 140.0f;
    float cellSpacing = roadWidth + tile;
    
    float screenW = Config::LOGICAL_WIDTH;
    float screenH = Config::LOGICAL_HEIGHT;

    float totalWidth = (adaptiveConfig.cols * roadWidth) + ((adaptiveConfig.cols - 1) * tile);
    float totalHeight = (adaptiveConfig.rows * roadWidth) + ((adaptiveConfig.rows - 1) * tile);
    float startX = (screenW - totalWidth) / 2.0f;
    float startY = (screenH - totalHeight) / 2.0f;

    // --- LOGIQUE DES CAPTEURS ---
    for (int idx = 0; idx < (int)coordinator->intersections.size(); idx++) {
        int r = idx / adaptiveConfig.cols; int c = idx % adaptiveConfig.cols;
        float ix = startX + c * cellSpacing; float iy = startY + r * cellSpacing;
        const float detectionRange = 120.0f;
        int countNS = 0; int countEW = 0;

        for (auto& car : cars) {
            if (car.axis == Direction::NorthSouth) {
                float distY = (car.behavior == STRAIGHT_V_DOWN) ? (iy - car.pos.y) : (car.pos.y - (iy + roadWidth));
                if (distY > 0 && distY < detectionRange) countNS++;
            }
            else {
                float distX = (car.behavior == STRAIGHT_H_RIGHT || car.behavior == TURN_EAST_TO_SOUTH)
                    ? (ix - car.pos.x) : (car.pos.x - (ix + roadWidth));
                if (distX > 0 && distX < detectionRange) countEW++;
            }
        }
        coordinator->intersections[idx]->LightsNS[0]->getCapteur()->mettreAJourFile(countNS);
        coordinator->intersections[idx]->LightsEW[0]->getCapteur()->mettreAJourFile(countEW);
    }

    coordinator->updateAll((float)dt);

    // --- SPAWN VOITURES ---
    spawnTimer += (float)dt;
    float spawnRate = isNightMode ? 3.0f : 1.0f;
    if (spawnTimer > spawnRate) {
        int r = GetRandomValue(0, adaptiveConfig.rows - 1);
        int c = GetRandomValue(0, adaptiveConfig.cols - 1);
        int choice = GetRandomValue(0, 5);
        switch (choice) {
        case 0: cars.emplace_back(Vector2{ startX + c * cellSpacing + 15, -50 }, Direction::NorthSouth, BLUE, STRAIGHT_V_DOWN); break;
        case 1: cars.emplace_back(Vector2{ startX + c * cellSpacing + 45, (float)screenH + 50 }, Direction::NorthSouth, RED, STRAIGHT_V_UP); break;
        case 2: cars.emplace_back(Vector2{ (float)screenW + 50, startY + r * cellSpacing + 15 }, Direction::EastWest, GREEN, STRAIGHT_H_LEFT); break;
        case 3: cars.emplace_back(Vector2{ -50, startY + r * cellSpacing + 45 }, Direction::EastWest, PURPLE, STRAIGHT_H_RIGHT); break;
        case 4: cars.emplace_back(Vector2{ -50, startY + r * cellSpacing + 45 }, Direction::EastWest, YELLOW, TURN_EAST_TO_SOUTH); break;
        case 5: cars.emplace_back(Vector2{ (float)screenW + 50, startY + r * cellSpacing + 15 }, Direction::EastWest, ORANGE, TURN_WEST_TO_NORTH); break;
        }
        spawnTimer = 0;
    }

    // Update cars
    for (size_t i = 0; i < cars.size(); i++) {
        int bestIdx = -1; float bestStopDist = 1e9f; float bestInterX = 0.0f;
        const float STOP_GAP = 10.0f;

        for (int idx = 0; idx < (int)coordinator->intersections.size(); idx++) {
            int cur_r = idx / adaptiveConfig.cols; int cur_c = idx % adaptiveConfig.cols;
            float ix = startX + cur_c * cellSpacing; float iy = startY + cur_r * cellSpacing;
            float stopDist = -1.0f;

            if (cars[i].axis == Direction::EastWest) {
                if (cars[i].behavior == STRAIGHT_H_RIGHT || cars[i].behavior == TURN_EAST_TO_SOUTH) {
                    if (std::abs(cars[i].pos.y - (iy + 45)) < 25 && cars[i].pos.x < ix) stopDist = (ix - STOP_GAP - cars[i].rec.width) - cars[i].pos.x;
                }
                else {
                    if (std::abs(cars[i].pos.y - (iy + 15)) < 25 && cars[i].pos.x > ix + roadWidth) stopDist = cars[i].pos.x - (ix + roadWidth + STOP_GAP);
                }
            }
            else {
                if (cars[i].behavior == STRAIGHT_V_DOWN) {
                    if (std::abs(cars[i].pos.x - (ix + 15)) < 25 && cars[i].pos.y < iy) stopDist = (iy - STOP_GAP - cars[i].rec.height) - cars[i].pos.y;
                }
                else {
                    if (std::abs(cars[i].pos.x - (ix + 45)) < 25 && cars[i].pos.y > iy + roadWidth) stopDist = cars[i].pos.y - (iy + roadWidth + STOP_GAP);
                }
            }
            if (stopDist >= 0 && stopDist < bestStopDist) { bestStopDist = stopDist; bestIdx = idx; bestInterX = ix; }
        }

        Color currentLight = GREEN;
        if (bestIdx != -1 && bestStopDist < 140.0f) {
            currentLight = (cars[i].axis == Direction::EastWest) ?
                coordinator->intersections[bestIdx]->LightsEW[0]->getRaylibColor() :
                coordinator->intersections[bestIdx]->LightsNS[0]->getRaylibColor();
        }

        cars[i].update((float)dt, cars, currentLight, (bestIdx == -1 ? 1e9f : bestStopDist), (bestIdx == -1 ? cars[i].pos.x : bestInterX), roadWidth);
        
        if (cars[i].pos.x < -200 || cars[i].pos.x > screenW + 200 || cars[i].pos.y < -200 || cars[i].pos.y > screenH + 200) {
            cars.erase(cars.begin() + i); i--;
        }
    }
}

void AdaptiveSignalsScene::draw() {
    float roadWidth = 80.0f;
    float tile = 140.0f;
    float cellSpacing = roadWidth + tile;
    float screenW = Config::LOGICAL_WIDTH;
    float screenH = Config::LOGICAL_HEIGHT;

    float totalWidth = (adaptiveConfig.cols * roadWidth) + ((adaptiveConfig.cols - 1) * tile);
    float totalHeight = (adaptiveConfig.rows * roadWidth) + ((adaptiveConfig.rows - 1) * tile);
    float startX = (screenW - totalWidth) / 2.0f;
    float startY = (screenH - totalHeight) / 2.0f;

    Color currentGrass = isNightMode ? Color{ 15, 15, 30, 255 } : Color{ 100, 130, 70, 255 };
    Color currentRoad = isNightMode ? Color{ 35, 35, 40, 255 } : DARKGRAY;
    Color currentSide = isNightMode ? Color{ 60, 60, 75, 255 } : Color{ 190, 190, 190, 255 };
    Color currentMark = isNightMode ? Color{ 180, 180, 200, 200 } : WHITE;
    Color treeD = isNightMode ? Color{ 10, 30, 10, 255 } : Color{ 30, 110, 30, 255 };
    Color treeL = isNightMode ? Color{ 20, 50, 20, 255 } : Color{ 50, 150, 50, 255 };

    ClearBackground(currentGrass);

    // --- COUCHE 1 : ROUTES ---
    for (int i = 0; i < adaptiveConfig.rows; i++) DrawRectangle(0, (int)(startY + (i * cellSpacing)), (int)screenW, (int)roadWidth, currentRoad);
    for (int j = 0; j < adaptiveConfig.cols; j++) DrawRectangle((int)(startX + (j * cellSpacing)), 0, (int)roadWidth, (int)screenH, currentRoad);

    // --- COUCHE 1b : DÉCOR (GRASS + SIDEWALKS) ---
    for (int i = 0; i <= adaptiveConfig.rows; i++) {
        for (int j = 0; j <= adaptiveConfig.cols; j++) {
            float xTile = (j == 0) ? 0 : startX + (j - 1) * cellSpacing + roadWidth;
            float yTile = (i == 0) ? 0 : startY + (i - 1) * cellSpacing + roadWidth;
            float wTile = (j == 0) ? startX : (j == adaptiveConfig.cols) ? (screenW - xTile) : tile;
            float hTile = (i == 0) ? startY : (i == adaptiveConfig.rows) ? (screenH - yTile) : tile;

            DrawRectangle((int)xTile, (int)yTile, (int)wTile, (int)hTile, currentGrass);

            if (wTile > 80 && hTile > 80) {
                int tx1 = (int)(xTile + wTile * 0.3f); int ty1 = (int)(yTile + hTile * 0.3f);
                DrawCircle(tx1, ty1, 6, treeD); DrawCircle(tx1 - 2, ty1 - 2, 4, treeL);
                int tx2 = (int)(xTile + wTile * 0.7f); int ty2 = (int)(yTile + hTile * 0.6f);
                DrawCircle(tx2, ty2, 6, treeD); DrawCircle(tx2 - 2, ty2 - 2, 4, treeL);
            }

            int sw = 10;
            DrawRectangle((int)xTile, (int)yTile, (int)wTile, sw, currentSide);
            DrawRectangle((int)xTile, (int)(yTile + hTile - sw), (int)wTile, sw, currentSide);
            DrawRectangle((int)xTile, (int)yTile, sw, (int)hTile, currentSide);
            DrawRectangle((int)(xTile + wTile - sw), (int)yTile, sw, (int)hTile, currentSide);
        }
    }

    // --- COUCHE 2 : MARQUAGES AU SOL ---
    const float LINE_THICK = 4.0f;
    const float GAP_BEFORE = 15.0f;
    const float GAP_AFTER = 25.0f;

    for (int i = 0; i < adaptiveConfig.rows; i++) {
        float yLine = startY + i * cellSpacing + roadWidth * 0.5f - 2;
        DrawRectangle(0, (int)yLine, (int)(startX - GAP_BEFORE), (int)LINE_THICK, currentMark);
        for (int j = 0; j < adaptiveConfig.cols - 1; j++) {
            float xS = startX + j * cellSpacing + roadWidth + GAP_AFTER;
            float xE = startX + (j + 1) * cellSpacing - GAP_BEFORE;
            DrawRectangle((int)xS, (int)yLine, (int)(xE - xS), (int)LINE_THICK, currentMark);
        }
        float xLast = startX + (adaptiveConfig.cols - 1) * cellSpacing + roadWidth + GAP_AFTER;
        DrawRectangle((int)xLast, (int)yLine, (int)(screenW - xLast), (int)LINE_THICK, currentMark);
    }

    for (int j = 0; j < adaptiveConfig.cols; j++) {
        float xLine = startX + j * cellSpacing + roadWidth * 0.5f - 2;
        DrawRectangle((int)xLine, 0, (int)LINE_THICK, (int)(startY - GAP_BEFORE), currentMark);
        for (int i = 0; i < adaptiveConfig.rows - 1; i++) {
            float yS = startY + i * cellSpacing + roadWidth + GAP_AFTER;
            float yE = startY + (i + 1) * cellSpacing - GAP_BEFORE;
            DrawRectangle((int)xLine, (int)yS, (int)LINE_THICK, (int)(yE - yS), currentMark);
        }
        float yLast = startY + (adaptiveConfig.rows - 1) * cellSpacing + roadWidth + GAP_AFTER;
        DrawRectangle((int)xLine, (int)yLast, (int)LINE_THICK, (int)(screenH - yLast), currentMark);
    }

    for (int i = 0; i < adaptiveConfig.rows; i++) {
        for (int j = 0; j < adaptiveConfig.cols; j++) {
            float xb = startX + j * cellSpacing; float yb = startY + i * cellSpacing;
            DrawCrosswalk(xb, yb - 15, roadWidth, 10, true);
            DrawCrosswalk(xb, yb + roadWidth + 5, roadWidth, 10, true);
            DrawCrosswalk(xb - 15, yb, 10, roadWidth, false);
            DrawCrosswalk(xb + roadWidth + 5, yb, 10, roadWidth, false);
        }
    }

    // --- COUCHE 3 : VOITURES ---
    for (auto& car : cars) {
        float dummyInterX = 0; // Simplified for integrated draw
        car.draw(dummyInterX, roadWidth);
        if (isNightMode) DrawCircleGradient((int)car.pos.x + 10, (int)car.pos.y + 10, 35, Fade(WHITE, 0.3f), BLANK);
    }

    // --- COUCHE 4 : FEUX + PANNEAUX ---
    for (int i = 0; i < (int)coordinator->intersections.size(); i++) {
        float xB = startX + (i % adaptiveConfig.cols) * cellSpacing;
        float yB = startY + (i / adaptiveConfig.cols) * cellSpacing;

        auto inter = coordinator->intersections[i];
        Color cNS = inter->LightsNS[0]->getRaylibColor();
        Color cEW = inter->LightsEW[0]->getRaylibColor();

        DrawTrafficLightRealistic(xB + roadWidth + 10, yB + roadWidth + 10, cNS, true);
        DrawTrafficLightRealistic(xB - 25, yB - 45, cNS, true);
        DrawTrafficLightRealistic(xB - 25, yB + roadWidth + 10, cEW, true);
        DrawTrafficLightRealistic(xB + roadWidth + 10, yB - 45, cEW, true);

        if (isNightMode) {
            DrawCircleGradient((int)xB + roadWidth + 15, (int)yB + roadWidth + 15, 20, Fade(cNS, 0.3f), BLANK);
            DrawCircleGradient((int)xB - 20, (int)yB + roadWidth + 15, 20, Fade(cEW, 0.3f), BLANK);
        }

        if (inter->LightsNS[0]->getCapteur()->obtenirLongueurFile() > 0)
            DrawCircle((int)(xB + roadWidth / 2), (int)(yB - 15), 6, LIME);

        if (inter->LightsEW[0]->getCapteur()->obtenirLongueurFile() > 0)
            DrawCircle((int)(xB - 15), (int)(yB + roadWidth / 2), 6, LIME);

        float pX = xB + roadWidth + 20;
        float pY = yB - 60;

        DrawRectangle((int)pX, (int)pY, 110, 50, Fade(BLACK, 0.4f));
        DrawText(TextFormat("NS: %d | %s",
                inter->LightsNS[0]->getCapteur()->obtenirLongueurFile(),
                inter->LightsNS[0]->getCapteur()->obtenirNiveauTrafic().c_str()),
            (int)pX + 5, (int)pY + 10, 10, WHITE);
        DrawText(TextFormat("EW: %d | %s",
                inter->LightsEW[0]->getCapteur()->obtenirLongueurFile(),
                inter->LightsEW[0]->getCapteur()->obtenirNiveauTrafic().c_str()),
            (int)pX + 5, (int)pY + 30, 10, WHITE);
    }

    // --- UI GLOBALE ---
    DrawRectangle(10, 10, 180, 35, Fade(BLACK, 0.6f));
    const char* modeT = (coordinator->intersections[0]->LightsNS[0]->getMode() == ModeFeux::Adaptatif) ? "MODE: ADAPTATIF" : "MODE: FIXE";
    DrawText(modeT, 20, 20, 16, (coordinator->intersections[0]->LightsNS[0]->getMode() == ModeFeux::Adaptatif) ? LIME : GOLD);

    DrawRectangle(10, 50, 180, 35, Fade(BLACK, 0.6f));
    DrawText(isNightMode ? "CYCLE: NUIT (N)" : "CYCLE: JOUR (N)", 20, 60, 16, isNightMode ? SKYBLUE : ORANGE);
    
    DrawText("ESC: Menu | N: Night | A: Adapt", 10, (int)screenH - 30, 20, LIGHTGRAY);

    uiManager.draw();
}
