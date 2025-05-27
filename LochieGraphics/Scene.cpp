#include "Scene.h"

#include "SceneObject.h"
#include "Lights.h"
#include "Camera.h"
#include "Ecco.h"
#include "Sync.h"
#include "ResourceManager.h"
#include "Skybox.h"
#include "Collider.h"
#include "Collision.h"

#include "EditorGUI.h"
#include "Serialisation.h"
#include "Paths.h"

#include <fstream>
#include <iostream>

Scene::Scene()
{
	gui.scene = this;
	ecco = new Ecco();
	sync = new Sync();
}

Scene::~Scene()
{
	//TODO: clear sceneobjects.
	//for (int i = 0; i < sceneObjects.size(); i++)
	//{
	//	delete sceneObjects[i];
	//}
}

#define SavePart(container)                                        \
	auto saved##container = toml::array();                         \
	for (auto i = container.begin(); i != container.end(); i++)    \
	{                                                              \
		saved##container.push_back(i->second.Serialise(i->first)); \
	}

#define LoadPart(container, savedName, type)                                                               \
toml::array* loading##type##s = data[##savedName].as_array();                                              \
if(loading##type##s){																				       \
	for (int i = 0; i < loading##type##s->size(); i++)                                                     \
	{                                                                                                      \
		toml::table* loading##type = loading##type##s->at(i).as_table();                                   \
                               /* TODO:  Should be constructing in place */                                \
		container[Serialisation::LoadAsUnsignedLongLong((*loading##type)["guid"])] = type(*loading##type); \
	}																								       \
}

void Scene::DeleteSceneObjectKeepChildren(unsigned long long GUID)
{
	for (std::vector<unsigned long long>::iterator marks = markedForDeletion.begin(); marks != markedForDeletion.end(); marks++)
	{
		if (*marks == GUID) return;
	}
	markedForDeletion.push_back(GUID);
}

void Scene::DeleteSceneObjectAndChildren(unsigned long long GUID)
{
	DeleteSceneObjectKeepChildren(GUID);
	for (auto i : sceneObjects.at(GUID)->transform()->getChildren())
	{
		DeleteSceneObjectAndChildren(i->so->GUID);
	}
}

void Scene::DeleteSceneObjectsMarkedForDelete()
{
	while (!markedForDeletion.empty())
	{
		if (gui.getSelected()) {
			if (gui.getSelected()->GUID == markedForDeletion.front()) {
				gui.setSelected(nullptr);
			}
		}
		delete sceneObjects.at(markedForDeletion.front());
		sceneObjects.erase(markedForDeletion.front());
		markedForDeletion.erase(markedForDeletion.begin());
	}
}

void Scene::DeleteAllSceneObjectsAndParts()
{
	while (!sceneObjects.empty())
	{
		delete sceneObjects.begin()->second;
		sceneObjects.erase(sceneObjects.begin());
	}
	markedForDeletion.clear();

	unsigned int partsChecker = Parts::ALL;

	transforms.clear();

	renderers.clear();
	partsChecker &= ~Parts::modelRenderer;

	animators.clear();
	partsChecker &= ~Parts::animator;

	rigidBodies.clear();
	partsChecker &= ~Parts::rigidBody;

	colliders.clear();
	partsChecker &= ~Parts::collider;

	healths.clear();
	partsChecker &= ~Parts::health;

	enemies.clear();
	partsChecker &= ~Parts::enemy;

	exits.clear();
	partsChecker &= ~Parts::exitElevator;

	spawnManagers.clear();
	partsChecker &= ~Parts::spawnManager;

	plates.clear();
	partsChecker &= ~Parts::plate;

	doors.clear();
	partsChecker &= ~Parts::door;

	bollards.clear();
	partsChecker &= ~Parts::bollard;

	freestyles.clear();
	partsChecker &= ~Parts::freestyle;

	// TODO: Don't like how just setting these flags here but no containers atm
	spotlights.clear();
	partsChecker &= ~Parts::spotlight;

	decals.clear();
	partsChecker &= ~Parts::decal;

	shadowWalls.clear();
	partsChecker &= ~Parts::shadowWall;

	pointLights.clear();
	partsChecker &= ~Parts::pointLight;

	partsChecker &= ~Parts::ecco;
	partsChecker &= ~Parts::sync;
	triggerables.clear();
	partsChecker &= ~Parts::triggerable;
	// TODO: Don't need a whole assert
	assert(partsChecker == 0);
}

SceneObject* Scene::FindSceneObjectOfName(std::string name)
{
	for (auto& i : sceneObjects)
	{
		if (i.second->name == name) {
			return i.second;
		}
	}
	return nullptr;
}

toml::table Scene::SaveSceneObjectsAndParts(bool(*shouldSave)(SceneObject*))
{
	// TODO: Maybe force delete the marked for delete here

	auto savedSceneObjects = toml::array();
	for (auto& i : sceneObjects)
	{
		if (shouldSave) {
			if (!shouldSave(i.second)) {
				continue;
			}
		}
		if (!i.second) continue;
		savedSceneObjects.push_back(i.second->Serialise());
	}
	SavePart(renderers);
	SavePart(transforms);
	auto savedanimators = toml::array(); 
	for (auto i = animators.begin(); i != animators.end(); i++) {
		savedanimators.push_back(i->second->Serialise(i->first));
	};
	SavePart(rigidBodies);
	SavePart(enemies);
	auto savedcolliders = toml::array(); for (auto i = colliders.begin(); i != colliders.end(); i++) {
		savedcolliders.push_back(i->second->Serialise(i->first));
	};
	SavePart(healths);
	SavePart(exits);
	SavePart(spawnManagers);
	SavePart(plates);
	SavePart(doors);
	SavePart(bollards);
	SavePart(freestyles);
	SavePart(triggerables);
	SavePart(pointLights);
	SavePart(spotlights);
	SavePart(decals);
	SavePart(shadowWalls);

	return toml::table{
		{ "SceneObjects", savedSceneObjects },
		{ "Renderers", savedrenderers},
		{ "Transforms", savedtransforms},
		{ "Animators", savedanimators},
		{ "RigidBodies", savedrigidBodies},
		{ "Colliders", savedcolliders},
		{ "Enemies", savedenemies},
		{ "Healths", savedhealths},
		{ "Exits", savedexits},
		{ "SpawnManagers", savedspawnManagers},
		{ "Plates", savedplates},
		{ "Doors", saveddoors},
		{ "Bollards", savedbollards},
		{ "Freestyles", savedfreestyles},
		{ "Sync", sync->Serialise() },
		{ "Ecco", ecco->Serialise() },
		{ "Triggerables", savedtriggerables},
		{ "PointLights", savedpointLights},
		{ "Spotlights", savedspotlights},
		{ "Decals", saveddecals},
		{ "ShadowWalls", savedshadowWalls},
	};

	// TODO: Make sure save all parts, put a checker here
}

void Scene::LoadSceneObjectsAndParts(toml::table& data)
{
	DeleteAllSceneObjectsAndParts();

	toml::array* loadingSceneObjects = data["SceneObjects"].as_array();
	int loadingSceneObjectsSize = (int)loadingSceneObjects->size();
	for (int i = 0; i < loadingSceneObjectsSize; i++)
	{
		auto loadingSceneObject = loadingSceneObjects->at(i).as_table();
		new SceneObject(this, loadingSceneObject);
	}

	LoadPart(renderers, "Renderers", ModelRenderer);
	toml::array* loadingTransforms = data["Transforms"].as_array(); if (loadingTransforms) {
		for (int i = 0; i < loadingTransforms->size(); i++) {
			toml::table* loadingTransform = loadingTransforms->at(i).as_table(); transforms[Serialisation::LoadAsUnsignedLongLong((*loadingTransform)["guid"])].Load(*loadingTransform);
		}
	};

	// Loading transforms doesn't keep the sceneobject pointer, they need to be refreshed
	std::vector<unsigned long long> toDeleteOfTransforms;
	for (auto& i : transforms)
	{
		auto search = sceneObjects.find(i.first);
		if (search == sceneObjects.end()) {
			toDeleteOfTransforms.push_back(i.first);
		}
		else {
			i.second.so = search->second;
		}
	}
	for (size_t i = 0; i < toDeleteOfTransforms.size(); i++)
	{
		transforms.erase(toDeleteOfTransforms.at(i));
	}




	// There was previously a bug that meant the hierarchy linkage could be broken
	// Some transform could be linked incorrectly, remove any that are
	for (auto& i : transforms)
	{
		bool marked = false;
		if (!i.second.isDirectChildOf(i.second.getParent())) {
			marked = true;
		}
		for (auto c : i.second.getChildren())
		{
			if (!i.second.isDirectParentOf(c)) {
				marked = true;
			}
		}
		if (marked) {
			// Delete object
			DeleteSceneObjectKeepChildren(i.first);
			std::cout << "Error: Found a sceneobject with a broken transform hierarchy, removing object\n";
		}
	}

	toml::array* loadingAnimators = data["Animators"].as_array(); 
	if (loadingAnimators) {
		for (int i = 0; i < loadingAnimators->size(); i++) {
			toml::table* loadingAnimator = loadingAnimators->at(i).as_table(); 
			animators[Serialisation::LoadAsUnsignedLongLong((*loadingAnimator)["guid"])] = Animator::Load(*loadingAnimator);
		}
	};

	// Renderers need to know if animators exist
	// TODO: Make it ain't so
	for (auto& i : animators)
	{
		renderers.at(i.first).animator = i.second;
	}


	LoadPart(rigidBodies, "RigidBodies", RigidBody);
	LoadPart(enemies, "Enemies", Enemy);
	LoadPart(spawnManagers, "SpawnManagers", SpawnManager);
	LoadPart(plates, "Plates", PressurePlate);
	LoadPart(doors, "Doors", Door);
	LoadPart(bollards, "Bollards", Bollard);
	LoadPart(freestyles, "Freestyles", Freestyle);
	LoadPart(triggerables, "Triggerables", Triggerable);
	LoadPart(pointLights, "PointLights", PointLight);
	LoadPart(spotlights, "Spotlights", Spotlight);
	LoadPart(decals, "Decals", Decal);
	LoadPart(shadowWalls, "ShadowWalls", ShadowWall);
	// TODO: Fix for colliders

	toml::array* loadingColliders = data["Colliders"].as_array(); 
	for (int i = 0; i < loadingColliders->size(); i++) {
		toml::table* loadingCollider = loadingColliders->at(i).as_table();
		colliders[Serialisation::LoadAsUnsignedLongLong((*loadingCollider)["guid"])] = Collider::Load(*loadingCollider);
	};
	
	LoadPart(healths, "Healths", Health);
	LoadPart(exits, "Exits", ExitElevator);

	*ecco = Ecco(*data["Ecco"].as_table());
	// Sync specifically loads here is it needs to keep some stuff
	sync->Load(*data["Sync"].as_table());

	// Load Hierarchy Data
	for (int i = 0; i < loadingSceneObjects->size(); i++)
	{
		auto loading = loadingSceneObjects->at(i).as_table();
		unsigned long long GUID = Serialisation::LoadAsUnsignedLongLong((*loading)["guid"]);
		SceneObject* sceneObject = sceneObjects[GUID];
		// TODO: Don't think sceneObjects need to store parents
		unsigned long long parentGUID = Serialisation::LoadAsUnsignedLongLong((*loading)["parent"]);
		if (parentGUID) {
			sceneObject->transform()->setParent(sceneObjects.at(parentGUID)->transform());
		}
		auto loadingChildren = (*loading)["children"].as_array();
		for (size_t i = 0; i < loadingChildren->size(); i++)
		{
			auto& temp = loadingChildren->at(i);
			unsigned long long childGUID = Serialisation::LoadAsUnsignedLongLong(temp);
			sceneObject->transform()->AddChild(sceneObjects.at(childGUID)->transform());
		}
	}

	EnsureAllPartsHaveSceneObject();
	for (auto& i : sceneObjects)
	{
		if (!i.second)
		{
			std::cout << "ERROR: Found sceneobject index containing no sceneobject\n";
			DeleteSceneObjectKeepChildren(i.first);
		}	
	}
	EnsurePartsValueMatchesParts();
}

#define EnsurePartSafety(container)                             \
	std::vector<unsigned long long> toDeleteOf##container;      \
	for (auto& i : container)                                   \
	{                                                           \
		if (sceneObjects.find(i.first) == sceneObjects.end()) { \
			toDeleteOf##container.push_back(i.first);           \
		}                                                       \
	}                                                           \
	for (size_t i = 0; i < toDeleteOf##container.size(); i++)   \
	{                                                           \
		container.erase(toDeleteOf##container.at(i));           \
	}

void Scene::EnsureAllPartsHaveSceneObject()
{
	// TODO: Rest of parts
	unsigned int partsChecker = Parts::ALL;

	// Transform do this themselves on load parts
	//EnsurePartSafety(transforms);
	EnsurePartSafety(renderers);
	EnsurePartSafety(rigidBodies);
	EnsurePartSafety(shadowWalls);
	EnsurePartSafety(healths);
}

#define EnsurePartValueMatchesParts(partsType, container)                                                  \
if (container.find(i.first) == container.end()) {                                                          \
	if (i.second->parts & partsType) {                                                                     \
		i.second->parts &= !partsType;                                                                     \
		std::cout << "Error: removed extra (missing actual part) parts marker for " << #partsType << '\n'; \
	}                                                                                                      \
}																										   \
else if (!(i.second->parts & partsType)) {																   \
	i.second->parts |= partsType;                                                                          \
	std::cout << "Error: added extra (had actual part) parts marker for " << #partsType << '\n';           \
}

void Scene::EnsurePartsValueMatchesParts()
{
	// TODO: For all parts
	// TODO: Put a parts checker

	for (auto& i : sceneObjects)
	{
		EnsurePartValueMatchesParts(Parts::rigidBody, rigidBodies);
		EnsurePartValueMatchesParts(Parts::collider, colliders);
		EnsurePartValueMatchesParts(Parts::modelRenderer, renderers);
		EnsurePartValueMatchesParts(Parts::shadowWall, shadowWalls);
		EnsurePartValueMatchesParts(Parts::health, healths);
	}
}

void Scene::InitialiseLayers()
{
	////ecco
	SceneObject* eccoSO = sceneObjects[ecco->GUID];
	eccoSO->rigidbody()->onCollision.push_back([this](Collision collision) { ecco->OnCollision(collision); });
	////sync
	SceneObject* syncSO = sceneObjects[sync->GUID];
	//if (!syncSO->health()) syncSO->setHealth(new Health());
	syncSO->rigidbody()->vel = { 0.0f, 0.0f };

	for (int i = 1; i < (int)CollisionLayers::count; i *= 2)
	{

		physicsSystem.SetCollisionLayerMask((int)CollisionLayers::reflectiveSurface, i, false);
		physicsSystem.SetCollisionLayerMask((int)CollisionLayers::count, i, false);
		physicsSystem.SetCollisionLayerMask(i, i, false);
		switch (i)
		{
		case (int)CollisionLayers::base:
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::softCover, i, false);
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::halfCover, i, false);
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::trigger, i, false);
			break;
		case (int)CollisionLayers::enemy:
			physicsSystem.SetCollisionLayerMask(i, i, true);
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::halfCover, i, false);
			break;
		case (int)CollisionLayers::trigger:
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::softCover, i, false);
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::halfCover, i, false);
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::enemyProjectile, i, false);
			break;
		case (int)CollisionLayers::enemyProjectile:
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::softCover, i, false);
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::halfCover, i, false);
			break;
		case (int)CollisionLayers::softCover:
			physicsSystem.SetCollisionLayerMask((int)CollisionLayers::halfCover, i, false);
			break;
		default:
			break;
		}
	}
}
