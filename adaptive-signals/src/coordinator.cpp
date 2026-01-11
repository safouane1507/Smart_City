#include"coordinator.h"
#include"intersectioncontroller.h"
#include"trafficlight.h"
#include"sensorr.h"
#include<iostream>
#include<vector>
#include<string>
using namespace std;

void Coordinator::addIntersection(IntersectionController* intersection) {
    this->intersections.push_back(intersection);
}
void Coordinator::updateAll(float deltaTime) {
    for (auto& inter : intersections) {
        inter->update(deltaTime);
    }
}

void Coordinator::switchModeAll(bool adaptive) {
    for (auto& inter : intersections) {
        inter->switchMode(adaptive);
    }
}