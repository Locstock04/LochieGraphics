#pragma once

// Is only included for the dummy rigidbody
#include "RigidBody.h"

#include <unordered_map>

struct Hit;
class Transform;
struct CollisionPacket;

class PhysicsSystem
{
private:
	float collisionBuffer = 10.0f;

public:
	PhysicsSystem() {};
	PhysicsSystem(toml::table table);

	void UpdateRigidBodies(
		std::unordered_map<unsigned long long, Transform>& transforms,
		std::unordered_map<unsigned long long, RigidBody>& rigidBodies,
		float deltaTime
	);

	void DepenertrationStep(
		std::unordered_map<unsigned long long, Transform>& transforms,
		std::unordered_map<unsigned long long, RigidBody>& rigidBodies
	);


	void CollisionCheckPhase
	(
		std::unordered_map<unsigned long long, Transform>& transforms,
		std::unordered_map<unsigned long long, RigidBody>& rigidBodies,
		std::unordered_map<unsigned long long, Collider*>& colliders
	);

	void GetCollisions(
		RigidBody* a, RigidBody* b,
		Transform* transformA, Transform* transformB,
		std::vector<CollisionPacket>& collisions
	);
	 
	void GetCollisions(
		RigidBody* a, Collider* b,
		Transform* transformA, Transform* transformB,
		std::vector<CollisionPacket>& collisions
	);

	void CollisisonResolution(CollisionPacket collision);

	/// <summary>
	/// Set the collision layer mask to true or false for two collision layers
	/// </summary>
	void SetCollisionLayerMask(int layerA, int layerB, bool state);
	void SetCollisionLayerMaskIndexed(int layerA, int layerB, bool state);

	/// <summary>
	/// Sets the entire bit mask for a layer in physics collisions
	/// </summary>
	void SetCollisionLayerMask(int layer, unsigned int bitMask);

	bool GetCollisionLayerBool(int layerA, int layerB);
	bool GetCollisionLayerIndexed(int layerA, int layerB);
	toml::table Serialise() const;

	bool displayAllColliders = false;
	unsigned int layerMasks[32] = {
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
		UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
	};

	int collisionItterations = 3;

	float minCollisonDistance = 500.0f;
private: 
	RigidBody dummyRigidBody = RigidBody(0.0f,0.0f);
public:
	static bool RayCastRadiusExpansion(glm::vec2 pos, glm::vec2 direction, std::vector<Hit>& hits,float radiusExpansion, float length = FLT_MAX, int layerMask = INT32_MAX, bool ignoreTriggers = true);
	static bool RayCast(glm::vec2 pos, glm::vec2 direction, std::vector<Hit>& hits, float length = FLT_MAX, int layerMask = INT32_MAX, bool ignoreTriggers = true);
	static std::vector<Hit> CircleCast(glm::vec2 pos, float radius, int layerMask = INT32_MAX, bool ignoreTriggers = true);

private:
	static std::unordered_map<unsigned long long, Transform>* transformsInScene;
	static std::unordered_map<unsigned long long, RigidBody>* rigidBodiesInScene;
	static std::unordered_map<unsigned long long, Collider*>* collidersInScene;

	static CollisionPacket RayCastAgainstCollider(
		glm::vec2 pos, glm::vec2 direction,
		Transform& transform, Collider* collider,
		float radiusExpansion
		);

};