#pragma once

#include "UiElement.h"

#include "Input.h"

class Transform;
class RigidBody;
class Health;
struct Collision;
namespace toml {
	inline namespace v3 {
		class table;
	}
}

class Ecco
{
public:
	unsigned long long GUID = 0;

	Ecco() {};
	Ecco(toml::table table);
	
	bool Update(
		Input::InputDevice& inputDevice,
		Transform& transform,
		RigidBody& rigidBody,
		Health& health,
		float delta,
		float cameraRotationDelta
	);

	void OnCollision(Collision collision);

	void GUI();
	toml::table Serialise();

	glm::vec2 wheelDirection = { 0.0f, 0.0f };	

	int currHealth = 10;
	int maxHealth = 200;

	float windowOfTimeForHealPressed = 0.2f;

	UiElement healthUI;
	UiElement boostUI;

	float getSpeedBoostCooldownPercent() const;
private:
	float carMoveSpeed = 0.25f;
	float carReverseMoveSpeed = 0.125f;
	float maxCarMoveSpeed = 6.0f;
	float maxReverseSpeed = 100.0f;
	float deadZone = 0.6f;
	float turningCircleScalar = 6.0f;
	float turningCircleScalarBoosting = 6.0f;
	float speedWheelTurnInfluence = 50.0f;
	float boostWheelTurnInfluence = 50.0f;
	float maxWheelAngle = 60.0f;
	float wheelTurnSpeed = 4.0f;
	float sidewaysFrictionCoef = 0.65f;
	float portionOfSidewaysSpeedKept = 100.0f;
	float stoppingFrictionCoef = 0.8f;
	float exceedingSlowIntensity = 2.5f;
	bool controlState = false;
	float speedBoost = 1.0f;
	float speedBoostCooldown = 0.1f;
	float speedBoostDuration = 0.0f;
	float timeSinceSpeedBoost = 0.0f;
	float timeInSpeedBoost = 0.0f;
	bool boosting = false;
	int speedBoostHPCost = 0;
	bool speedBoostUnactuated = true;
	float lastSpeedBoostPressed = 0.0f;
	float minSpeedDamageThreshold = 0.0f;
	int speedDamage = 0;
	int healingFromDamage = 0;
	float speedReductionAfterDamaging = 0.0f;
	float timeSinceHealButtonPressed = FLT_MAX;
	int boostDamage = 10;
	float speedKnockback = 10.0f;
	float boostKnockback = 10.0f;
	float healCooldown = 1.0f;

	float timeInDrive = FLT_MAX;
	float timeSinceLastHeal = FLT_MAX;
	glm::vec2 lastFrameVelocity = { 0.0f, 0.0f };
	float lerpForwardAni = 0.0f;
	//TESTING VALUES
	bool speedBoostInDirectionOfBody = true;
	glm::vec3 headSpot;
};