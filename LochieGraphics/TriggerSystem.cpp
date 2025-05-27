#include "TriggerSystem.h"
#include "SceneManager.h"
#include "Scene.h"
#include "SceneObject.h"
#include "SpawnManager.h"
#include "RigidBody.h"
#include "Collider.h"
#include "Collision.h"
#include "Transform.h"
#include "Triggerable.h"
#include "Spotlight.h"
#include "PressurePlate.h"
#include "Freestyle.h"
#include <iostream>

std::multimap<std::string, unsigned long long> TriggerSystem::triggerables = {};

void TriggerSystem::TriggerTag(std::string tag, bool toggle)
{
	if (tag == "") return;
	auto range = triggerables.equal_range(tag);
	for (auto& it = range.first; it != range.second; it++)
	{
		SceneManager::scene->sceneObjects[(*it).second]->TriggerCall(tag, toggle);
	}
}

void TriggerSystem::Start(
	std::unordered_map<unsigned long long, RigidBody>& rigidbodies,
	std::unordered_map<unsigned long long, PressurePlate>& plates,
	std::unordered_map<unsigned long long, SpawnManager>& spawnManagers,
	std::unordered_map<unsigned long long, Door>& doors,
	std::unordered_map<unsigned long long, Bollard>& bollards,
	std::unordered_map<unsigned long long, Triggerable>& triggerables,
	std::unordered_map<unsigned long long, Spotlight>& spotlights,
	std::unordered_map<unsigned long long, Freestyle>& freestyles
)
{
	TriggerSystem::triggerables.clear();
	for (auto& spawnPair : spawnManagers)
	{
		TriggerSystem::triggerables.insert({ spawnPair.second.triggerTag, spawnPair.first });
	}
	for (auto& platePair : plates)
	{
		if (!(SceneManager::scene->sceneObjects[platePair.first]->parts & Parts::rigidBody))
		{
			SceneManager::scene->sceneObjects[platePair.first]->setRigidBody(new RigidBody());
		}
		rigidbodies[platePair.first].onTrigger.push_back(
			[platePair](Collision collision) { SceneManager::scene->plates[platePair.first].OnTrigger(collision.collisionMask); });
	}
	for (auto& pair : doors)
	{
		TriggerSystem::triggerables.insert({ pair.second.triggerTag, pair.first });
	}
	for (auto& pair : bollards)
	{
		TriggerSystem::triggerables.insert({ pair.second.triggerTag, pair.first });
	}

	for (auto& pair : freestyles)
	{
		TriggerSystem::triggerables.insert({ pair.second.triggerTag, pair.first });
	}

	for (auto& pair : triggerables)
	{
		SceneObject* so = SceneManager::scene->sceneObjects[pair.first];
		if (!so) continue;
		if (!(so->parts & Parts::rigidBody))
		{
			so->setRigidBody(new RigidBody());
		}
		rigidbodies[pair.first].onTrigger.push_back(
			[pair](Collision collision) { SceneManager::scene->triggerables[pair.first].OnTrigger(collision.collisionMask); });
	}
	for (auto& pair : spotlights)
	{
		TriggerSystem::triggerables.insert({ pair.second.triggerTag, pair.first });
	}
}

void TriggerSystem::Update(
	std::unordered_map<unsigned long long, PressurePlate>& plates,
	std::unordered_map<unsigned long long, Triggerable>& triggerables,
	std::unordered_map<unsigned long long, Transform>& transforms,
	float delta
)
{
	for (auto& platePair : plates)
	{
		glm::vec2 pos = transforms[platePair.first].get2DPosition();
		float actuation = Utilities::Lerp(platePair.second.atRestHeight, -platePair.second.actuationAmount, platePair.second.timeInActuation / timeToActuate );
		transforms[platePair.first].setPosition({ pos.x, actuation, pos.y });

		//Actuation
		if (platePair.second.triggeredThisFrame)
			platePair.second.timeInActuation += delta;
		else
			platePair.second.timeInActuation -= delta;

		platePair.second.timeInActuation = glm::clamp(platePair.second.timeInActuation, 0.0f, timeToActuate);

		if (!platePair.second.eccoToggled)
		{
			if (platePair.second.triggeredLastFrame && !platePair.second.triggeredThisFrame)
			{
				TriggerSystem::TriggerTag(platePair.second.triggerTag, false);
			}
			platePair.second.triggeredLastFrame = platePair.second.triggeredThisFrame;
			platePair.second.triggeredThisFrame = false;

		}
		if (!platePair.second.triggeredLastFrame && platePair.second.triggeredThisFrame) {
			// TODO: Sound not playing correctly, plays every frame, fix
			//SceneManager::scene->audio.PlaySound(Audio::pressurePlateActivate);
		}
	}

	for (auto& pair : triggerables)
	{
		if (pair.second.bothPlayersNeeded && pair.second.eccoThisFrame && pair.second.syncThisFrame)
			TriggerTag(pair.second.triggerTag, !pair.second.falseIsTrue);
		else if (!pair.second.bothPlayersNeeded && (pair.second.eccoThisFrame || pair.second.syncThisFrame))
			TriggerTag(pair.second.triggerTag, !pair.second.falseIsTrue);
		else if(!pair.second.doesntSendFalseEveryFrame)
			TriggerTag(pair.second.triggerTag, pair.second.falseIsTrue);
		pair.second.eccoThisFrame = pair.second.syncThisFrame = false;
	}
}

void TriggerSystem::Clear()
{
	triggerables.clear();
}
