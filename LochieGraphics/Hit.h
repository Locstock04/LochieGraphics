#pragma once
#include "Maths.h"

#include <vector>

class RigidBody;
class SceneObject;
struct Collider;

struct Hit
{
	glm::vec2 normal;
	float distance;
	glm::vec2 position;
	Collider* collider;
	SceneObject* sceneObject;
};