#pragma once

#include <string>
#include "Maths.h"

namespace toml {
	inline namespace v3 {
		class table;
	}
}

class Freestyle
{
public:

	Freestyle() {};
	Freestyle(toml::table table);

	void GUI();

	void TriggerCall(std::string tag, bool toggle);
	std::string triggerTag;

	bool boolean;

	glm::vec2 vector2;
	glm::vec3 vector3;
	glm::vec4 colour;

	int i = 0;
	float f = 0.0f;
	std::string string = "";


	toml::table Serialise(unsigned long long guid);
};