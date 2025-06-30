#pragma once
#include <unordered_map>

class Transform;
class ModelRenderer;
class Ecco;
class Sync;
class Health;
class Animator;
class RigidBody;
class ExitElevator;
class SpawnManager;
class PressurePlate;
class Door;
class Bollard;
class Triggerable;
class PointLight;
class Spotlight;
class Decal;
struct Enemy;
struct ShadowWall;
class Freestyle;

class FreestyleSystem
{
public:
	void Start(
		std::unordered_map<unsigned long long, Transform>& transforms,
		std::unordered_map<unsigned long long, ModelRenderer>& renderers,
		Ecco& ecco,
		Sync& sync,
		std::unordered_map<unsigned long long, Health>& healths,
		std::unordered_map<unsigned long long, Animator*>& animators,
		std::unordered_map<unsigned long long, RigidBody>& rigidbodies,
		std::unordered_map<unsigned long long, ExitElevator>& exitElevators,
		std::unordered_map<unsigned long long, SpawnManager>& spawnManagers,
		std::unordered_map<unsigned long long, PressurePlate>& pressurePlates,
		std::unordered_map<unsigned long long, Door>& doors,
		std::unordered_map<unsigned long long, Bollard>& bollards,
		std::unordered_map<unsigned long long, Triggerable>& triggerable,
		std::unordered_map<unsigned long long, PointLight>& pointLights,
		std::unordered_map<unsigned long long, Spotlight>& spotlights,
		std::unordered_map<unsigned long long, Decal>& decals,
		std::unordered_map<unsigned long long, Enemy>& enemies,
		std::unordered_map<unsigned long long, ShadowWall>& shadowWalls,
		std::unordered_map<unsigned long long, Freestyle>& freestyles
	);

	void Update(
		std::unordered_map<unsigned long long, Transform>& transforms,
		std::unordered_map<unsigned long long, ModelRenderer>& renderers,
		Ecco& ecco,
		Sync& sync,
		std::unordered_map<unsigned long long, Health>& healths,
		std::unordered_map<unsigned long long, Animator*>& animators,
		std::unordered_map<unsigned long long, RigidBody>& rigidbodies,
		std::unordered_map<unsigned long long, ExitElevator>& exitElevators,
		std::unordered_map<unsigned long long, SpawnManager>& spawnManagers,
		std::unordered_map<unsigned long long, PressurePlate>& pressurePlates,
		std::unordered_map<unsigned long long, Door>& doors,
		std::unordered_map<unsigned long long, Bollard>& bollards,
		std::unordered_map<unsigned long long, Triggerable>& triggerable,
		std::unordered_map<unsigned long long, PointLight>& pointLights,
		std::unordered_map<unsigned long long, Spotlight>& spotlights,
		std::unordered_map<unsigned long long, Decal>& decals,
		std::unordered_map<unsigned long long, Enemy>& enemies,
		std::unordered_map<unsigned long long, ShadowWall>& shadowWalls,
		std::unordered_map<unsigned long long, Freestyle>& freestyles,
		float delta
	);
};

