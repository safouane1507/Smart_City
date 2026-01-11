#pragma once
#include "raylib.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include "trafficlight.h"
#include "intersectioncontroller.h"

// ✅ Identity behaviors (lane-correct, light-correct, safe)
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

void DrawTrafficLightRealistic(float x, float y, Color activeColor, bool vertical);
void DrawCrosswalk(float x, float y, float width, float height, bool horizontal);
