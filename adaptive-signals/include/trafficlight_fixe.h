#pragma once
#ifndef  TRAFFIC_LIGHT_FIXE_H
#define TRAFFIC_LIGHT_FIXE_H

#include<string>
  
enum class lights {

    Red,
    Green,
    Yellow

};

class traffic_core {

private:

    std::string id;
    lights curruentlight;
    float timer;
    float fixduration_Red;
    float fixeduration_Yelow;
    float fixeduration_Green;

public:

    traffic_core(std::string identite, lights c);

    void updatTimer();

    void NextState();

    void switchColor();

    lights getcolor();

    std::string getId();

};


#endif