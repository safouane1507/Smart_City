#pragma once
#ifndef coordinator2_h
#define coordinator2_h

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "intersectioncontroller.h"



class Coordinator {

public:
    std::vector<IntersectionController*> intersections;


    Coordinator() {}
    void addIntersection(IntersectionController* intersection); //addition d'une intersection
    void updateAll(float deltaTime);                            //mettre a jour tout les intersection
    void switchModeAll(bool adaptive);     //changer le mode pour toutes les intersections





};


#endif
