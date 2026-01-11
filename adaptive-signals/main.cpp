#include "raylib.h"

#include "coordinator.h"
#include "intersectioncontroller.h"
#include "trafficlight.h"
#include "sensorr.h"
#include "adaptivelogic.h"
#include <vector>
#include <string>
#include <iostream>
#include <cmath>


// --- Paramètres de l'écran ---
int currentScreenWidth = 1000;
int currentScreenHeight = 900;
Music bgMusic;
bool musicLoaded = false;
bool musicEnabled = true;   // controlled by menu button


enum GameState { MENU = 0, GAME = 1 };

// ✅ Reality-based behaviors (lane-correct, light-correct, safe)
enum Behavior {
    // Straight
    STRAIGHT_H_RIGHT,   // eastbound (left -> right) on lane y+45
    STRAIGHT_H_LEFT,    // westbound (right -> left) on lane y+15
    STRAIGHT_V_DOWN,    // southbound (top -> bottom) on lane x+15
    STRAIGHT_V_UP,      // northbound (bottom -> top) on lane x+45

    // Turning (always at the nearest intersection ahead)
    TURN_EAST_TO_SOUTH, // YELLOW: eastbound then turn DOWN
    TURN_WEST_TO_NORTH  // ORANGE: westbound then turn UP
};

// --- CLASSE VOITURE (SAFE + REALISTIC) ---
struct Car {
    Vector2 pos;
    float speed;
    float maxSpeed;
    Color color;

    // axis tells which light to use (EW vs NS)
    Direction axis;     // Direction::NorthSouth or Direction::EastWest
    Behavior behavior;

    bool hasTurned = false;
    float blinkTimer = 0;

    Rectangle rec;

    Car(Vector2 p, Direction a, Color c, Behavior b)
        : pos(p), axis(a), color(c), behavior(b)
    {
        maxSpeed = (float)GetRandomValue(150, 220);
        speed = maxSpeed;
        updateRect();
    }

    void updateRect() {
        if (axis == Direction::NorthSouth) rec = { pos.x, pos.y, 20, 35 };
        else                              rec = { pos.x, pos.y, 35, 20 };
    }

    bool isTurningCar() const {
        return (behavior == TURN_EAST_TO_SOUTH || behavior == TURN_WEST_TO_NORTH) && !hasTurned;
    }



    // --- REALISTIC UPDATE ---
    void draw(float interX, float roadWidth)
    {
        // PIXEL SHADOW
        DrawRectangle(rec.x + 2, rec.y + 2, rec.width, rec.height, DARKGRAY);

        // BODY
        DrawRectangleRec(rec, color);

        // ROOF BLOCK
        Color roof = Color{
            (unsigned char)std::min(255, color.r + 40),
            (unsigned char)std::min(255, color.g + 40),
            (unsigned char)std::min(255, color.b + 40),
            255
        };

        DrawRectangle(
            rec.x + rec.width * 0.25f,
            rec.y + rec.height * 0.25f,
            rec.width * 0.5f,
            rec.height * 0.5f,
            roof
        );

        // CENTER STRIPE (your signature)
        DrawRectangle(
            rec.x + rec.width / 2 - 2,
            rec.y + 3,
            4,
            rec.height - 6,
            Color{ 200, 220, 255, 255 }
        );

        // WHEELS (PIXEL STYLE)
        DrawRectangle(rec.x - 2, rec.y + 4, 2, 6, BLACK);
        DrawRectangle(rec.x - 2, rec.y + rec.height - 10, 2, 6, BLACK);
        DrawRectangle(rec.x + rec.width, rec.y + 4, 2, 6, BLACK);
        DrawRectangle(rec.x + rec.width, rec.y + rec.height - 10, 2, 6, BLACK);

        // LIGHTS
        if (axis == Direction::EastWest) {
            DrawRectangle(rec.x + rec.width - 2, rec.y + 3, 2, 4, YELLOW);
            DrawRectangle(rec.x + rec.width - 2, rec.y + rec.height - 7, 2, 4, YELLOW);
            DrawRectangle(rec.x, rec.y + 3, 2, 4, RED);
            DrawRectangle(rec.x, rec.y + rec.height - 7, 2, 4, RED);
        }
    }





    void update(float dt,
        const std::vector<Car>& allCars,
        Color lightColorForThisCar,
        float stopDistToLine,
        float interX,
        float roadWidth)
    {
        blinkTimer += dt;

        // 1) Same-lane following (same axis + same direction + same lane)
        const float followDist = 45.0f;
        bool mustStopForCar = false;

        for (const auto& other : allCars) {
            if (&other == this) continue;
            if (axis != other.axis) continue;

            // same direction check (so opposite lane doesn't cause braking)
            bool sameDir = false;

            if (behavior == STRAIGHT_H_RIGHT || behavior == TURN_EAST_TO_SOUTH) {
                sameDir = (other.behavior == STRAIGHT_H_RIGHT || other.behavior == TURN_EAST_TO_SOUTH);
            }
            else if (behavior == STRAIGHT_H_LEFT || behavior == TURN_WEST_TO_NORTH) {
                sameDir = (other.behavior == STRAIGHT_H_LEFT || other.behavior == TURN_WEST_TO_NORTH);
            }
            else if (behavior == STRAIGHT_V_DOWN) {
                sameDir = (other.behavior == STRAIGHT_V_DOWN);
            }
            else if (behavior == STRAIGHT_V_UP) {
                sameDir = (other.behavior == STRAIGHT_V_UP);
            }

            if (!sameDir) continue;

            // lane tolerance
            const float laneTol = 10.0f;
            float sideDist = (axis == Direction::EastWest)
                ? std::abs(other.pos.y - pos.y)
                : std::abs(other.pos.x - pos.x);
            if (sideDist > laneTol) continue;

            // forward distance in travel direction
            float forward = 999999.0f;
            if (behavior == STRAIGHT_H_RIGHT || behavior == TURN_EAST_TO_SOUTH) forward = other.pos.x - pos.x;
            else if (behavior == STRAIGHT_H_LEFT || behavior == TURN_WEST_TO_NORTH) forward = pos.x - other.pos.x;
            else if (behavior == STRAIGHT_V_DOWN) forward = other.pos.y - pos.y;
            else if (behavior == STRAIGHT_V_UP) forward = pos.y - other.pos.y;

            if (forward > 0 && forward < followDist) {
                mustStopForCar = true;
                break;
            }
        }

        // 2) Traffic light stopping (only at stop line when red)
        bool isRed = (lightColorForThisCar.r > 200 && lightColorForThisCar.g < 100);

        // snap distance to stop line
        const float stopSnap = 10.0f;
        bool mustStopForLight = (isRed && stopDistToLine >= 0 && stopDistToLine < stopSnap);

        // 3) Speed decision
        if (mustStopForCar || mustStopForLight) speed = 0;
        else speed = maxSpeed;

        // 4) Turning (always at nearest intersection ahead)
        if (isTurningCar()) {
            float triggerX = interX + roadWidth * 0.50f;

            if (behavior == TURN_EAST_TO_SOUTH) {
                // moving right: turn when front reaches trigger
                float triggerX = interX + roadWidth * 0.50f;

                if (pos.x + rec.width >= triggerX) {
                    axis = Direction::NorthSouth;
                    pos.x = interX + 15.0f;   // align with V_DOWN lane
                    behavior = STRAIGHT_V_DOWN;
                    hasTurned = true;
                    updateRect();
                }
            }


            if (behavior == TURN_WEST_TO_NORTH) {
                // moving left: turn ONLY after fully clearing the crosswalks
                float turnTriggerX = interX + roadWidth + 20.0f;

                if (pos.x <= turnTriggerX) {
                    axis = Direction::NorthSouth;
                    pos.x = interX + 45.0f;   // align with upward lane
                    behavior = STRAIGHT_V_UP;
                    hasTurned = true;
                    updateRect();
                }
            }



        }

        // 5) Move
        if (behavior == STRAIGHT_V_UP) pos.y -= speed * dt;
        else if (axis == Direction::NorthSouth) pos.y += speed * dt;
        else if (behavior == STRAIGHT_H_LEFT || behavior == TURN_WEST_TO_NORTH) pos.x -= speed * dt;
        else pos.x += speed * dt;

        updateRect();
    }


};

// --- Prototypes ---
void runMenu(int& rows, int& cols, GameState& state, int screenW, int screenH);
void runGame(int rows, int cols, GameState& state, int screenW, int screenH);
void DrawTrafficLightRealistic(float x, float y, Color activeColor, bool vertical);
void DrawCrosswalk(float x, float y, float width, float height, bool horizontal);

int main() {
    InitWindow(currentScreenWidth, currentScreenHeight, "Gestion de Trafic Intelligent");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);


    InitAudioDevice();

    bgMusic = LoadMusicStream("nan.mp3");
    bgMusic.looping = true;
    SetMusicVolume(bgMusic, 0.4f);
    PlayMusicStream(bgMusic);
    musicLoaded = true;


    int currentRows = 1;
    int currentColls = 1;
    GameState currentState = MENU;


    while (!WindowShouldClose()) {

        if (musicEnabled && musicLoaded) {
            UpdateMusicStream(bgMusic);
        }


        currentScreenWidth = GetScreenWidth();
        currentScreenHeight = GetScreenHeight();

        if (currentState == MENU) {
            runMenu(currentRows, currentColls, currentState, currentScreenWidth, currentScreenHeight);
        }
        else {
            runGame(currentRows, currentColls, currentState, currentScreenWidth, currentScreenHeight);
        }
    }

    UnloadMusicStream(bgMusic);


    CloseAudioDevice();
    CloseWindow();
    return 0;
}

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

void runMenu(int& rows, int& cols, GameState& state, int screenW, int screenH)
{
    static int inputRows = 2;
    static int inputCols = 3;

    Vector2 mouse = GetMousePosition();
    bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    bool valid = (inputRows >= 1 && inputCols >= 1);

    // === COLORS ===
    Color bg = WHITE;
    Color text = Color{ 90, 90, 90, 255 };
    Color btn = Color{ 180, 180, 180, 255 };
    Color btnHover = Color{ 210, 210, 210, 255 };
    Color startBg = valid ? GREEN : RED;

    // === CENTER ===
    float cx = screenW * 0.5f;
    float cy = screenH * 0.5f;

    // === ROWS LINE ===
    float lineY1 = cy - 40;
    Rectangle rowMinus = { cx + 20, lineY1 - 12, 24, 24 };
    Rectangle rowPlus = { cx + 90, lineY1 - 12, 24, 24 };

    // === COLS LINE ===
    float lineY2 = cy;
    Rectangle colMinus = { cx + 20, lineY2 - 12, 24, 24 };
    Rectangle colPlus = { cx + 90, lineY2 - 12, 24, 24 };

    // === START ===
    Rectangle startRect = { cx - 100, cy + 60, 200, 36 };


    Rectangle musicBtn = {
    startRect.x + startRect.width / 2 - 20,
    startRect.y + startRect.height + 15,
    40,
    40
    };
    // === INPUT ===
    if (click) {
        if (CheckCollisionPointRec(mouse, rowPlus))  inputRows++;
        if (CheckCollisionPointRec(mouse, rowMinus) && inputRows > 0) inputRows--;

        if (CheckCollisionPointRec(mouse, colPlus))  inputCols++;
        if (CheckCollisionPointRec(mouse, colMinus) && inputCols > 0) inputCols--;

        if (valid && CheckCollisionPointRec(mouse, startRect)) {
            rows = inputRows;
            cols = inputCols;
            state = GAME;
        }
    }
    if (CheckCollisionPointRec(mouse, musicBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        musicEnabled = !musicEnabled;

        if (!musicEnabled)
            PauseMusicStream(bgMusic);
        else
            ResumeMusicStream(bgMusic);
    }
    // === DRAW ===
    BeginDrawing();
    ClearBackground(bg);

    // TITLE
    const char* title = "ULTIMATE CITY TRAFFIC";
    int titleSize = 32;
    DrawText(title, cx - MeasureText(title, titleSize) / 2, cy - 140, titleSize, text);

    // ROWS
    DrawText("ROWS:", cx - 120, lineY1 - 12, 20, text);
    DrawText(TextFormat("%d", inputRows), cx + 60, lineY1 - 12, 20, text);

    DrawRectangleRec(rowMinus, CheckCollisionPointRec(mouse, rowMinus) ? btnHover : btn);
    DrawRectangleRec(rowPlus, CheckCollisionPointRec(mouse, rowPlus) ? btnHover : btn);
    DrawText("-", rowMinus.x + 7, rowMinus.y + 2, 20, BLACK);
    DrawText("+", rowPlus.x + 7, rowPlus.y + 2, 20, BLACK);

    // COLS
    DrawText("COLS:", cx - 120, lineY2 - 12, 20, text);
    DrawText(TextFormat("%d", inputCols), cx + 60, lineY2 - 12, 20, text);

    DrawRectangleRec(colMinus, CheckCollisionPointRec(mouse, colMinus) ? btnHover : btn);
    DrawRectangleRec(colPlus, CheckCollisionPointRec(mouse, colPlus) ? btnHover : btn);
    DrawText("-", colMinus.x + 7, colMinus.y + 2, 20, BLACK);
    DrawText("+", colPlus.x + 7, colPlus.y + 2, 20, BLACK);

    // START GAME
    DrawRectangleRec(startRect, startBg);
    DrawText(
        "START GAME",
        cx - MeasureText("START GAME", 20) / 2,
        startRect.y + 8,
        20,
        BLACK
    );

    // --- MUSIC BUTTON BACKGROUND ---
    DrawRectangleRec(musicBtn, Color{ 220, 220, 220, 255 });
    DrawRectangleLines(
        (int)musicBtn.x,
        (int)musicBtn.y,
        (int)musicBtn.width,
        (int)musicBtn.height,
        GRAY
    );

    // --- ICON CENTER (RENAMED VARIABLES) ---
    float iconCx = musicBtn.x + musicBtn.width * 0.5f;
    float iconCy = musicBtn.y + musicBtn.height * 0.5f;

    if (musicEnabled) {
        // 🔊 SPEAKER ON
        DrawTriangle(
            Vector2{ iconCx - 10, iconCy - 8 },
            Vector2{ iconCx - 10, iconCy + 8 },
            Vector2{ iconCx + 2,  iconCy },
            DARKGREEN
        );
        DrawCircle(iconCx + 8, iconCy, 4, DARKGREEN);
    }
    else {
        // 🔇 SPEAKER OFF
        DrawTriangle(
            Vector2{ iconCx - 10, iconCy - 8 },
            Vector2{ iconCx - 10, iconCy + 8 },
            Vector2{ iconCx + 2,  iconCy },
            RED
        );
        DrawLine(
            iconCx - 12, iconCy - 12,
            iconCx + 12, iconCy + 12,
            RED
        );
    }
    EndDrawing();

}




void runGame(int rows, int cols, GameState& state, int screenW, int screenH) {
    static Coordinator* coordinator = nullptr;
    static LogiqueAdaptative* logiqueGlobal = nullptr;
    static std::vector<Car> cars;
    static float spawnTimer = 0;
    static bool isInitialized = false;
    static bool isNightMode = false;
    static Music bgMusic;
    static bool musicLoaded = false; // Ajouté pour la sécurité

    float roadWidth = 80.0f;
    float tile = 140.0f;
    float cellSpacing = roadWidth + tile;

    // --- INITIALISATION ---
    if (!isInitialized) {
        coordinator = new Coordinator();
        logiqueGlobal = new LogiqueAdaptative();
        for (int i = 0; i < (rows * cols); i++) {
            std::string id = "Inter_" + std::to_string(i);
            IntersectionController* inter = new IntersectionController(id, logiqueGlobal);
            inter->addTrafficLight(new TrafficLight(LightColors::Red, ModeFeux::Fixe, new Capteur("S_NS" + id)), Direction::NorthSouth);
            inter->addTrafficLight(new TrafficLight(LightColors::Green, ModeFeux::Fixe, new Capteur("S_EW" + id)), Direction::EastWest);
            coordinator->addIntersection(inter);
        }
        isInitialized = true;
    }

    // --- CONTRÔLES (INPUTS) ---
    if (IsKeyPressed(KEY_N)) isNightMode = !isNightMode;

    if (IsKeyPressed(KEY_A)) {
        for (auto inter : coordinator->intersections) {
            ModeFeux nouveauMode = (inter->LightsNS[0]->getMode() == ModeFeux::Fixe) ? ModeFeux::Adaptatif : ModeFeux::Fixe;
            inter->LightsNS[0]->setMode(nouveauMode);
            inter->LightsEW[0]->setMode(nouveauMode);
        }
    }

    if (IsKeyPressed(KEY_M)) {
        isInitialized = false; cars.clear();
        if (musicLoaded) { StopMusicStream(bgMusic); UnloadMusicStream(bgMusic); }
        delete coordinator; coordinator = nullptr;
        delete logiqueGlobal; logiqueGlobal = nullptr;
        state = MENU; return;
    }

    float totalWidth = (cols * roadWidth) + ((cols - 1) * tile);
    float totalHeight = (rows * roadWidth) + ((rows - 1) * tile);
    float startX = (screenW - totalWidth) / 2.0f;
    float startY = (screenH - totalHeight) / 2.0f;

    // --- LOGIQUE DES CAPTEURS ---
    for (int idx = 0; idx < (int)coordinator->intersections.size(); idx++) {
        int r = idx / cols; int c = idx % cols;
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

    coordinator->updateAll(GetFrameTime());

    // --- SPAWN VOITURES ---
    spawnTimer += GetFrameTime();
    float spawnRate = isNightMode ? 3.0f : 1.0f; // Ajusté pour pas que ce soit trop vide la nuit
    if (spawnTimer > spawnRate) {
        int r = GetRandomValue(0, rows - 1);
        int c = GetRandomValue(0, cols - 1);
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

    Color currentGrass = isNightMode ? Color{ 15, 15, 30, 255 } : Color{ 100, 130, 70, 255 };
    Color currentRoad = isNightMode ? Color{ 35, 35, 40, 255 } : DARKGRAY;
    Color currentSide = isNightMode ? Color{ 60, 60, 75, 255 } : Color{ 190, 190, 190, 255 };
    Color currentMark = isNightMode ? Color{ 180, 180, 200, 200 } : WHITE;
    Color treeD = isNightMode ? Color{ 10, 30, 10, 255 } : Color{ 30, 110, 30, 255 };
    Color treeL = isNightMode ? Color{ 20, 50, 20, 255 } : Color{ 50, 150, 50, 255 };


    BeginDrawing();
    ClearBackground(currentGrass);

    // --- COUCHE 1 : ROUTES ---
    for (int i = 0; i < rows; i++) DrawRectangle(0, (int)(startY + (i * cellSpacing)), screenW, (int)roadWidth, currentRoad);
    for (int j = 0; j < cols; j++) DrawRectangle((int)(startX + (j * cellSpacing)), 0, (int)roadWidth, screenH, currentRoad);

    // --- COUCHE 1b : DÉCOR (GRASS + SIDEWALKS) ---
    for (int i = 0; i <= rows; i++) {
        for (int j = 0; j <= cols; j++) {
            float xTile = (j == 0) ? 0 : startX + (j - 1) * cellSpacing + roadWidth;
            float yTile = (i == 0) ? 0 : startY + (i - 1) * cellSpacing + roadWidth;
            float wTile = (j == 0) ? startX : (j == cols) ? (screenW - xTile) : tile;
            float hTile = (i == 0) ? startY : (i == rows) ? (screenH - yTile) : tile;

            // Grass Base (Dynamique)
            DrawRectangle((int)xTile, (int)yTile, (int)wTile, (int)hTile, currentGrass);

            // Trees (Dynamique)
            if (wTile > 80 && hTile > 80) {
                int tx1 = (int)(xTile + wTile * 0.3f); int ty1 = (int)(yTile + hTile * 0.3f);
                DrawCircle(tx1, ty1, 6, treeD); DrawCircle(tx1 - 2, ty1 - 2, 4, treeL);
                int tx2 = (int)(xTile + wTile * 0.7f); int ty2 = (int)(yTile + hTile * 0.6f);
                DrawCircle(tx2, ty2, 6, treeD); DrawCircle(tx2 - 2, ty2 - 2, 4, treeL);
            }

            // Sidewalk (Dynamique)
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

    for (int i = 0; i < rows; i++) {
        float yLine = startY + i * cellSpacing + roadWidth * 0.5f - 2;
        // Bord gauche -> 1ère inter
        DrawRectangle(0, (int)yLine, (int)(startX - GAP_BEFORE), (int)LINE_THICK, currentMark);
        // Entre inters
        for (int j = 0; j < cols - 1; j++) {
            float xS = startX + j * cellSpacing + roadWidth + GAP_AFTER;
            float xE = startX + (j + 1) * cellSpacing - GAP_BEFORE;
            DrawRectangle((int)xS, (int)yLine, (int)(xE - xS), (int)LINE_THICK, currentMark);
        }
        // Dernière inter -> Bord droit
        float xLast = startX + (cols - 1) * cellSpacing + roadWidth + GAP_AFTER;
        DrawRectangle((int)xLast, (int)yLine, (int)(screenW - xLast), (int)LINE_THICK, currentMark);
    }

    for (int j = 0; j < cols; j++) {
        float xLine = startX + j * cellSpacing + roadWidth * 0.5f - 2;
        // Bord haut -> 1ère inter
        DrawRectangle((int)xLine, 0, (int)LINE_THICK, (int)(startY - GAP_BEFORE), currentMark);
        // Entre inters
        for (int i = 0; i < rows - 1; i++) {
            float yS = startY + i * cellSpacing + roadWidth + GAP_AFTER;
            float yE = startY + (i + 1) * cellSpacing - GAP_BEFORE;
            DrawRectangle((int)xLine, (int)yS, (int)LINE_THICK, (int)(yE - yS), currentMark);
        }
        // Dernière inter -> Bord bas
        float yLast = startY + (rows - 1) * cellSpacing + roadWidth + GAP_AFTER;
        DrawRectangle((int)xLine, (int)yLast, (int)LINE_THICK, (int)(screenH - yLast), currentMark);
    }

    // Passages Piétons
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float xb = startX + j * cellSpacing; float yb = startY + i * cellSpacing;
            DrawCrosswalk(xb, yb - 15, roadWidth, 10, true);
            DrawCrosswalk(xb, yb + roadWidth + 5, roadWidth, 10, true);
            DrawCrosswalk(xb - 15, yb, 10, roadWidth, false);
            DrawCrosswalk(xb + roadWidth + 5, yb, 10, roadWidth, false);
        }
    }



    // --- COUCHE 3 : VOITURES ---
    for (size_t i = 0; i < cars.size(); i++) {
        int bestIdx = -1; float bestStopDist = 1e9f; float bestInterX = 0.0f;
        const float STOP_GAP = 10.0f;

        for (int idx = 0; idx < (int)coordinator->intersections.size(); idx++) {
            int r = idx / cols; int c = idx % cols;
            float ix = startX + c * cellSpacing; float iy = startY + r * cellSpacing;
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

        cars[i].update(GetFrameTime(), cars, currentLight, (bestIdx == -1 ? 1e9f : bestStopDist), (bestIdx == -1 ? cars[i].pos.x : bestInterX), roadWidth);
        cars[i].draw(bestInterX, roadWidth);

        if (isNightMode) DrawCircleGradient(cars[i].pos.x + 10, cars[i].pos.y + 10, 35, Fade(WHITE, 0.3f), BLANK);

        if (cars[i].pos.x < -200 || cars[i].pos.x > screenW + 200 || cars[i].pos.y < -200 || cars[i].pos.y > screenH + 200) {
            cars.erase(cars.begin() + i); i--;
        }
    }

    // --- COUCHE 4 : FEUX + PANNEAUX ---
    for (int i = 0; i < (int)coordinator->intersections.size(); i++) {
        float xB = startX + (i % cols) * cellSpacing;
        float yB = startY + (i / cols) * cellSpacing;

        auto inter = coordinator->intersections[i];

        Color cNS = inter->LightsNS[0]->getRaylibColor();
        Color cEW = inter->LightsEW[0]->getRaylibColor();

        // Feux
        DrawTrafficLightRealistic(xB + roadWidth + 10, yB + roadWidth + 10, cNS, true);
        DrawTrafficLightRealistic(xB - 25, yB - 45, cNS, true);
        DrawTrafficLightRealistic(xB - 25, yB + roadWidth + 10, cEW, true);
        DrawTrafficLightRealistic(xB + roadWidth + 10, yB - 45, cEW, true);


        // --- COUCHE 4 : FEUX + TIMERS ---
        for (int i = 0; i < (int)coordinator->intersections.size(); i++) {

            float xB = startX + (i % cols) * cellSpacing;
            float yB = startY + (i / cols) * cellSpacing;

            auto inter = coordinator->intersections[i];

            // ===== DRAW LIGHTS =====
            Color cNS = inter->LightsNS[0]->getRaylibColor();
            Color cEW = inter->LightsEW[0]->getRaylibColor();

            DrawTrafficLightRealistic(xB + roadWidth + 10, yB + roadWidth + 10, cNS, true);
            DrawTrafficLightRealistic(xB - 25, yB - 45, cNS, true);
            DrawTrafficLightRealistic(xB - 25, yB + roadWidth + 10, cEW, true);
            DrawTrafficLightRealistic(xB + roadWidth + 10, yB - 45, cEW, true);


        }



        // Halo des feux la nuit
        if (isNightMode) {
            DrawCircleGradient(xB + roadWidth + 15, yB + roadWidth + 15, 20, Fade(cNS, 0.3f), BLANK);
            DrawCircleGradient(xB - 20, yB + roadWidth + 15, 20, Fade(cEW, 0.3f), BLANK);
        }

        // Indicateurs de capteurs (présence de véhicules)
        if (inter->LightsNS[0]->getCapteur()->obtenirLongueurFile() > 0)
            DrawCircle(xB + roadWidth / 2, yB - 15, 6, LIME);

        if (inter->LightsEW[0]->getCapteur()->obtenirLongueurFile() > 0)
            DrawCircle(xB - 15, yB + roadWidth / 2, 6, LIME);

        // Panneau d'information trafic
        float pX = xB + roadWidth + 20;
        float pY = yB - 60;

        DrawRectangle(pX, pY, 110, 50, Fade(BLACK, 0.4f));
        DrawText(
            TextFormat("NS: %d | %s",
                inter->LightsNS[0]->getCapteur()->obtenirLongueurFile(),
                inter->LightsNS[0]->getCapteur()->obtenirNiveauTrafic().c_str()),
            pX + 5, pY + 10, 10, WHITE
        );
        DrawText(
            TextFormat("EW: %d | %s",
                inter->LightsEW[0]->getCapteur()->obtenirLongueurFile(),
                inter->LightsEW[0]->getCapteur()->obtenirNiveauTrafic().c_str()),
            pX + 5, pY + 30, 10, WHITE
        );
    }





    // --- UI GLOBALE ---
    DrawRectangle(10, 10, 180, 35, Fade(BLACK, 0.6f));
    const char* modeT = (coordinator->intersections[0]->LightsNS[0]->getMode() == ModeFeux::Adaptatif) ? "MODE: ADAPTATIF" : "MODE: FIXE";
    DrawText(modeT, 20, 20, 16, (coordinator->intersections[0]->LightsNS[0]->getMode() == ModeFeux::Adaptatif) ? LIME : GOLD);

    DrawRectangle(10, 50, 180, 35, Fade(BLACK, 0.6f));
    DrawText(isNightMode ? "CYCLE: NUIT (N)" : "CYCLE: JOUR (N)", 20, 60, 16, isNightMode ? SKYBLUE : ORANGE);

    EndDrawing();
}