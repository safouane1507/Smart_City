#include"trafficlight_fixe.h"
#include"raylib.h"


traffic_core::traffic_core(std::string identite, lights c) {

	curruentlight = c;
	id = identite;
	fixduration_Red = 10.0f;
	fixeduration_Green = 10.0f;
	fixeduration_Yelow = 3.0f;
	timer = 0;
};

void traffic_core:: updatTimer() {

	timer += GetFrameTime();
};

void traffic_core::NextState() {

	if (timer >= 3.0 && curruentlight == lights::Yellow) {

		switchColor();
		timer = 0;
	}

	else if (timer >= 10.0 && curruentlight == lights::Green) {

		switchColor();
		timer = 0;
	}

	else if (timer >= 10.0 && curruentlight == lights::Red) {

		switchColor();
		timer = 0;
	}
}


	void traffic_core::switchColor() {
		if (curruentlight == lights::Red)
			curruentlight = lights::Green;
		else if (curruentlight == lights::Green)
			curruentlight = lights::Yellow;
		else if (curruentlight == lights::Yellow)
			curruentlight = lights::Red;
	}


lights traffic_core::getcolor() {

	return curruentlight;

}

std::string traffic_core::getId() {

	return id;

}