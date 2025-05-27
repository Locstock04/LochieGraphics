#include "SceneObject.h"

#include "SceneManager.h"
#include "ResourceManager.h"
#include "Animator.h"
#include "Collider.h"
#include "ExitElevator.h"
#include "Triggerable.h"
#include "Sync.h"
#include "Ecco.h"
#include "Scene.h"
#include "Paths.h"
#include "PrefabManager.h"
#include "BlendedAnimator.h"
#include "Directional2dAnimator.h"

#include "Utilities.h"

#include "EditorGUI.h"
#include "LineRenderer.h"
#include "Serialisation.h"

#include <fstream>
#include <array>

#define SavePart(saveName, partsName, container)                              \
	if (Parts::##partsName & parts) {                                         \
		table.emplace(saveName, scene->##container.at(GUID).Serialise(GUID)); \
		safetyCheck &= ~Parts::##partsName;                                   \
	}

#define LoadPart(saveName, partsName, setter, type)         \
	if (intendedParts & Parts::##partsName) {               \
		/* TODO: likely leaking here, fix */                \
		setter(new type(*table[saveName].as_table()));      \
	}

#define AddPartGUI(getter, setter, constructor, label) \
if (getter() == nullptr) {                             \
if (ImGui::MenuItem(label)) {                          \
		setter(new constructor);                       \
	}                                                  \
}

#define RemovePartGUI(partsType, setter, label) \
if (parts & Parts::partsType) {                 \
	if (ImGui::MenuItem(label)) {               \
		setter(nullptr);                        \
	}                                           \
}

#define setPart(part, container, enumValue)                     \
	if (part) {                                                 \
		parts |= enumValue;                                     \
 /*TODO: Ensure that this isn't leaking memory and is alright*/ \
		scene->container[GUID] = *part;                         \
	}                                                           \
	else {                                                      \
		parts &= ~enumValue;                                    \
		scene->container.erase(GUID);                           \
	}

#define getPart(container, enumValue)        \
	if (parts & enumValue) {                 \
		return &(scene->container.at(GUID)); \
	}                                        \
	return nullptr

#define SetAndGetForPart(Type, container, enumValue, nameInSet, nameInGet) \
void SceneObject::set##nameInSet(Type* part)                               \
{                                                                          \
	setPart(part, container, enumValue);                                   \
}                                                                          \
Type * SceneObject::##nameInGet()                                          \
{                                                                          \
	getPart(container, enumValue);                                         \
}

SceneObject::SceneObject(Scene* _scene, std::string _name) :
	scene(_scene),
	name(_name)
{
	GUID = ResourceManager::GetNewGuid();
	scene->transforms[GUID].so = this;
	scene->sceneObjects[GUID] = this;
	// TODO: Put a safetly check for if a guid that gets made is already on a sceneobject
}

SceneObject::~SceneObject()
{
	ClearParts();
	scene->transforms.erase(GUID);
}

void SceneObject::GUI()
{
	std::string tag = Utilities::PointerToString(this);
	ImGui::InputText(("Name##" + tag).c_str(), &name);

	if (prefabStatus == PrefabStatus::origin) {
		if (ImGui::Button(("Save Prefab##" + tag).c_str())) {
			SaveAsPrefab();
		}
		ImGui::SameLine();
		if (ImGui::Button(("Refresh Prefab Instances##" + tag).c_str())) {
			PrefabManager::RefreshPrefabInstancesOf(GUID);
		}
	}
	if (prefabStatus == PrefabStatus::instance) {
		if (ImGui::Button(("Refresh This Instance##" + tag).c_str())) {
			LoadFromPrefab(PrefabManager::loadedPrefabOriginals.at(prefabBase));
		}
		// TODO: Button to 'overwrite' the prefab, like apply changes to prefab
	}

	scene->transforms.at(GUID).GUI();

	if (parts & Parts::modelRenderer) { scene->renderers.at(GUID).GUI(); }
	if (parts & Parts::rigidBody) { scene->rigidBodies.at(GUID).GUI(); }

	if (parts & Parts::collider) { 
		// Collapsing Header is not apart of the collider GUI as it can be apart of the rigidbody too
		if (ImGui::CollapsingHeader(("Collider##" + tag).c_str())) {
			scene->colliders.at(GUID)->GUI();
		}
	}
	
	if (parts & Parts::enemy) { scene->enemies[GUID].GUI(); }
	if (parts & Parts::health) { scene->healths[GUID].GUI(); }
	if (parts & Parts::spawnManager) { scene->spawnManagers[GUID].GUI(); }
	if (parts & Parts::plate) { scene->plates[GUID].GUI(); }
	if (parts & Parts::door) { scene->doors[GUID].GUI(); }
	if (parts & Parts::bollard) { scene->bollards[GUID].GUI(); }
	if (parts & Parts::freestyle) { scene->freestyles[GUID].GUI(); }
	if (parts & Parts::triggerable) { scene->triggerables[GUID].GUI(); }
	if (parts & Parts::pointLight) { scene->pointLights[GUID].GUI(); }
	if (parts & Parts::spotlight) { scene->spotlights[GUID].GUI(); }
	if (parts & Parts::decal) { scene->decals[GUID].GUI(); }
	if (parts & Parts::shadowWall) { scene->shadowWalls[GUID].GUI(); }

	if (parts & Parts::ecco)
	{
		if(scene->ecco)
			scene->ecco->GUI();
	}

	if (parts & Parts::sync)
	{
		if (scene->sync)
			scene->sync->GUI();
	}

	if (parts & Parts::exitElevator)
	{
		scene->exits.at(GUID).GUI(this);
	}
	// TODO: Add animator parts;
	if ((parts & Parts::animator)) {
		scene->animators.at(GUID)->GUI();
	}

	std::string addPopup = "SceneObject Add Part" + tag;
	std::string removePopup = "SceneObject Remove Part" + tag;

	if (ImGui::Button(("Add Part##" + tag).c_str())) {
		ImGui::OpenPopup(addPopup.c_str());
	}
	ImGui::SameLine();
	if (ImGui::Button(("Remove Part##" + tag).c_str())) {
		ImGui::OpenPopup(removePopup.c_str());
	}

	if (ImGui::BeginPopup(addPopup.c_str())) {

		AddPartGUI(animator, setAnimator, Animator(), ("Animator##Add part" + tag).c_str());
		AddPartGUI(animator, setAnimator, BlendedAnimator(), ("Blended Animator##Add part" + tag).c_str());
		AddPartGUI(animator, setAnimator, Directional2dAnimator(), ("Directional 2D Animator##Add part" + tag).c_str());
		AddPartGUI(bollard, setBollard, Bollard, ("Bollard ##Add part" + tag).c_str());

		AddPartGUI(decal, setDecal, Decal, ("Decal##Add part" + tag).c_str());

		AddPartGUI(door, setDoor, Door, ("Door ##Add part" + tag).c_str());
		if (ecco() == nullptr && scene->ecco->GUID == 0) {
			if (ImGui::MenuItem(("Ecco##Add part" + tag).c_str())) {
				setEcco();
			}
		}
		AddPartGUI(enemy, setEnemy, Enemy, ("Enemy##Add part" + tag).c_str());
		AddPartGUI(exitElevator, setExitElevator, ExitElevator, ("Exit Elevator##Add part" + tag).c_str());
		AddPartGUI(freestyle, setFreestyle, Freestyle, ("Freestyle##Add part" + tag).c_str());
		AddPartGUI(health, setHealth, Health, ("Health##Add part" + tag).c_str());
		AddPartGUI(plate, setPressurePlate, PressurePlate, ("Pressure Plate ##Add part" + tag).c_str());
		AddPartGUI(pointLight, setPointLight, PointLight, ("Point Light##Add part" + tag).c_str());
		AddPartGUI(collider, setCollider, PolygonCollider({
				{ +50, +50},
				{ +50, -50},
				{ -50, -50},
				{ -50, +50}
			}, 0.0f), ("Polygon Collider##Add part" + tag).c_str());
		AddPartGUI(rigidbody, setRigidBody, RigidBody(), ("Rigid Body##Add part" + tag).c_str());
		AddPartGUI(renderer, setRenderer, ModelRenderer(), ("Model Renderer##Add part" + tag).c_str());
		if(renderer() != nullptr)
		{ 
			if (shadowWall() == nullptr) {
				if (ImGui::MenuItem(("Shadow Wall##Add part" + tag).c_str())) {
					setShadowWall(new ShadowWall());
				}
			};
		}
		AddPartGUI(spawnManager, setSpawnManager, SpawnManager, ("Spawn Manager ##Add part" + tag).c_str());
		AddPartGUI(spotlight, setSpotlight, Spotlight, ("Spotlight ##Add part" + tag).c_str());
		if (sync() == nullptr && scene->sync->GUID == 0) {
			if (ImGui::MenuItem(("Sync##Add part" + tag).c_str())) {
				setSync();
			}
		}
		AddPartGUI(triggerable, setTriggerable, Triggerable, ("Triggerable##Add part" + tag).c_str());
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup(removePopup.c_str())) {
		RemovePartGUI(animator, setAnimator, ("Animator##Remove part" + tag).c_str());
		RemovePartGUI(bollard, setBollard, ("Bollard##Remove part" + tag).c_str());
		RemovePartGUI(collider, setCollider, ("Collider##Remove part" + tag).c_str());
		RemovePartGUI(decal, setDecal, ("Decal##Remove part" + tag).c_str());
		RemovePartGUI(door, setDoor, ("Door##Remove part" + tag).c_str());
		if (parts & Parts::ecco) {
			if (ImGui::MenuItem(("Ecco##Remove part" + tag).c_str())) {
				setEcco(nullptr);
			}
		}
		RemovePartGUI(enemy, setEnemy, ("Enemy##Remove part" + tag).c_str());
		RemovePartGUI(exitElevator, setExitElevator, ("Exit##Remove part" + tag).c_str());
		RemovePartGUI(freestyle, setFreestyle, ("Freestyle##Remove part" + tag).c_str());
		RemovePartGUI(health, setHealth, ("Health##Remove part" + tag).c_str());
		RemovePartGUI(modelRenderer, setRenderer, ("Model Renderer##Remove part" + tag).c_str());
		RemovePartGUI(plate, setPressurePlate, ("Pressure Plate##Remove part" + tag).c_str());
		RemovePartGUI(pointLight, setPointLight, ("Point Light##Remove part" + tag).c_str());
		RemovePartGUI(rigidBody, setRigidBody, ("Rigid Body##Remove part" + tag).c_str());
		if (parts & Parts::shadowWall) {
			if (ImGui::MenuItem(("Shadow Wall##Remove part" + tag).c_str())) {
				setShadowWall(nullptr);
			}
		};
		RemovePartGUI(spawnManager, setSpawnManager, ("Spawn Manager ##Remove part" + tag).c_str());
		RemovePartGUI(spotlight, setSpotlight, ("Spotlight ##Remove part" + tag).c_str());
		if (parts & Parts::sync) {
			if (ImGui::MenuItem(("Sync##Remove part" + tag).c_str())) {
				setSync(nullptr);
			}
		}
		RemovePartGUI(triggerable, setTriggerable, ("Triggerable##Remove part" + tag).c_str());

		ImGui::EndPopup();
	}
}


void SceneObject::MenuGUI()
{
	std::string tag = Utilities::PointerToString(this);

	if (ImGui::MenuItem(("Delete##RightClick" + tag).c_str())) {
		scene->DeleteSceneObjectAndChildren(GUID);
	}
	if (ImGui::MenuItem((("Duplicate##RightClick") + tag).c_str())) {
		Duplicate();
	}
	if (ImGui::MenuItem(("Add empty child##" + tag).c_str())) {
		(new SceneObject(scene))->transform()->setParent(transform());
	}
	if (ImGui::MenuItem(("Save As Prefab##RightClick" + tag).c_str())) {
		SaveAsPrefab();
	}
	if (ImGui::MenuItem(("Replace with Prefab##RightClick" + tag).c_str())) {
		auto prefab = PrefabManager::loadedPrefabOriginals.find(PrefabManager::selectedPrefab);
		if (prefab != PrefabManager::loadedPrefabOriginals.end()) {
			LoadFromPrefab(prefab->second);
		}
		// Else // TODO: Warning
	}
	if (ImGui::MenuItem(("Refresh Prefab Instance##RightClick" + tag).c_str(), nullptr, nullptr, prefabStatus == PrefabStatus::instance)) {
		RefreshPrefab();
	}
	if (ImGui::MenuItem(("Unlink Prefab Instance##RightClick" + tag).c_str(), nullptr, nullptr, prefabStatus == PrefabStatus::instance)) {
		prefabStatus = PrefabStatus::missing;
	}
	if (ImGui::MenuItem(("Copy GUID##RightClick" + tag).c_str())) {
		ImGui::SetClipboardText(std::to_string(GUID).c_str());
	}
	auto prefabGUID = 0ull;
	if (prefabStatus == PrefabStatus::origin) {
		prefabGUID = GUID;
	}
	else if (prefabStatus == PrefabStatus::instance) {
		prefabGUID = prefabBase;
	}
	if (prefabGUID) {
		if (ImGui::MenuItem(("Select Prefab##RightClick" + tag).c_str())) {
			PrefabManager::selectedPrefab = prefabGUID;
		}
	}
	ImGui::EndPopup();

}

void SceneObject::MultiMenuGUI(std::set<SceneObject*> multiSelectedSceneObjects, bool* setNullSelect)
{
	std::string tag = "MultipleSelectRightClickSceneObjectMenu";

	if (ImGui::MenuItem(("Delete##RightClick" + tag).c_str())) {
		for (auto i : multiSelectedSceneObjects)
		{
			SceneManager::scene->DeleteSceneObjectAndChildren(i->GUID);
		}
		*setNullSelect = true;
	}
	if (ImGui::MenuItem(("Save Any Prefab Origins##RightClick" + tag).c_str())) {
		for (auto i : multiSelectedSceneObjects)
		{
			if (i->prefabStatus == SceneObject::PrefabStatus::origin) {
				i->SaveAsPrefab();
			}
		}
	}
	if (ImGui::MenuItem(("Refresh Prefab Instances##RightClick" + tag).c_str())) {
		for (auto i : multiSelectedSceneObjects) {
			i->RefreshPrefab();
		}
	}
	if (ImGui::MenuItem(("Replace with Prefab##RightClick" + tag).c_str())) {
		auto prefab = PrefabManager::loadedPrefabOriginals.find(PrefabManager::selectedPrefab);
		if (prefab != PrefabManager::loadedPrefabOriginals.end()) {
			for (auto i : multiSelectedSceneObjects) {
				i->LoadFromPrefab(prefab->second);
			}
		}
	}
	if (ImGui::MenuItem(("Unlink Any Prefabs##RightClick" + tag).c_str())) {
		for (auto i : multiSelectedSceneObjects) {
			if (i->prefabStatus == PrefabStatus::instance) {
				i->prefabStatus = PrefabStatus::missing;
			}
		}
	}
	ImGui::EndPopup();
}

bool SceneObject::PartsFilterSelector(const std::string& label, unsigned int& parts)
{
	std::string tag = Utilities::PointerToString(&parts);
	bool partsFilterOpen = ImGui::Button((label + "##Parts filter" + tag).c_str());
	std::string partsFilterPopupID = "partsFilterPopUp" + tag;
	if (parts) {
		ImGui::PopStyleColor();
	}
	if (ImGui::BeginItemTooltip()) {
		ImGui::Text("Parts Filter");
		ImGui::EndTooltip();
	}
	if (partsFilterOpen) {
		ImGui::OpenPopup(partsFilterPopupID.c_str());
	}
	if (!ImGui::BeginPopup(partsFilterPopupID.c_str())) {
		return false;
	}

	ImGui::CheckboxFlags("Animator##Parts Filter", &parts, Parts::animator);
	ImGui::CheckboxFlags("Bollard##Parts Filter", &parts, Parts::bollard);
	ImGui::CheckboxFlags("Collider##Parts Filter", &parts, Parts::collider);
	ImGui::CheckboxFlags("Door##Parts Filter", &parts, Parts::door);
	ImGui::CheckboxFlags("Ecco##Parts Filter", &parts, Parts::ecco);
	ImGui::CheckboxFlags("Enemy##Parts Filter", &parts, Parts::enemy);
	ImGui::CheckboxFlags("Exit Elevator##Parts Filter", &parts, Parts::exitElevator);
	ImGui::CheckboxFlags("Freestyle##Parts Filter", &parts, Parts::freestyle);
	ImGui::CheckboxFlags("Plate##Parts Filter", &parts, Parts::plate);
	ImGui::CheckboxFlags("PointLight##Parts Filter", &parts, Parts::pointLight);
	ImGui::CheckboxFlags("RigidBody##Parts Filter", &parts, Parts::rigidBody);
	ImGui::CheckboxFlags("Spawn Manager##Parts Filter", &parts, Parts::spawnManager);
	ImGui::CheckboxFlags("Spotlight##Parts Filter", &parts, Parts::spotlight);
	ImGui::CheckboxFlags("Sync##Parts Filter", &parts, Parts::sync);

	ImGui::EndPopup();
	return true;
}

void SceneObject::DebugDraw()
{
	Transform* t = &scene->transforms.at(GUID);
	if (parts & Parts::rigidBody)
	{
		scene->rigidBodies.at(GUID).DebugDraw(t);
	}
	if (parts & Parts::collider)
	{
		scene->colliders.at(GUID)->DebugDraw(t);
	}
	if (parts & Parts::modelRenderer && UserPreferences::showSelectedBox) {
		Model* model = renderer()->model;
		if (model) {
			LineRenderer& lines = RenderSystem::lines;
			lines.SetColour({ 1.0f, 1.0f, 1.0f });

			std::array<glm::vec3, 16> v {
				glm::vec3{ model->min.x, model->min.y, model->min.z },
				glm::vec3{ model->min.x, model->min.y, model->max.z },
				glm::vec3{ model->min.x, model->max.y, model->max.z },
				glm::vec3{ model->max.x, model->max.y, model->max.z },
				glm::vec3{ model->max.x, model->max.y, model->min.z },
				glm::vec3{ model->min.x, model->max.y, model->min.z },
				glm::vec3{ model->min.x, model->min.y, model->min.z },
				glm::vec3{ model->max.x, model->min.y, model->min.z },
				glm::vec3{ model->max.x, model->min.y, model->max.z },
				glm::vec3{ model->min.x, model->min.y, model->max.z },
				glm::vec3{ model->min.x, model->max.y, model->max.z },
				glm::vec3{ model->min.x, model->max.y, model->min.z },
				glm::vec3{ model->max.x, model->max.y, model->min.z },
				glm::vec3{ model->max.x, model->min.y, model->min.z },
				glm::vec3{ model->max.x, model->min.y, model->max.z },
				glm::vec3{ model->max.x, model->max.y, model->max.z },
			};
			for (size_t i = 0; i < v.size(); i++)
			{
				lines.AddPointToLine(glm::vec3(transform()->getGlobalMatrix() * glm::vec4(v.at(i), 1.0f)));
			}
			lines.FinishLineStrip();
		}
	}
	//float s = 300.0f;
	//Transform* t = transform();
	//RenderSystem::debugLines.DrawLineSegment(t->getGlobalPosition(), t->getGlobalPosition() + t->right() * s, { 1, 0, 0 });
	//RenderSystem::debugLines.DrawLineSegment(t->getGlobalPosition(), t->getGlobalPosition() + t->up() * s, { 0, 1, 0 });
	//RenderSystem::debugLines.DrawLineSegment(t->getGlobalPosition(), t->getGlobalPosition() + t->forward() * s, { 0, 0, 1 });
}

void SceneObject::TriggerCall(std::string tag, bool toggle)
{
	if (parts & Parts::spawnManager)
		scene->spawnManagers[GUID].TriggerCall(tag, toggle);

	if (parts & Parts::door)
		scene->doors[GUID].TriggerCall(tag, toggle);

	if (parts & Parts::bollard)
		scene->bollards[GUID].TriggerCall(tag, toggle);

	if (parts & Parts::freestyle)
		scene->freestyles[GUID].TriggerCall(tag, toggle);

	if (parts & Parts::pointLight)
		scene->pointLights[GUID].TriggerCall(tag, toggle);

	if (parts & Parts::spotlight)
		scene->spotlights[GUID].TriggerCall(tag, toggle);
}

toml::table SceneObject::Serialise() const
{
	auto parent = transform()->getParent();
	unsigned long long parentGUID = 0;
	if (parent) {
		parentGUID = parent->getSceneObject()->GUID;
	}
	toml::array childrenGUIDs;
	for (auto child : transform()->getChildren())
	{
		childrenGUIDs.push_back(Serialisation::SaveAsUnsignedLongLong(child->getSceneObject()->GUID));
	}
	return toml::table{
		{ "name", name},
		{ "guid", Serialisation::SaveAsUnsignedLongLong(GUID) },
		{ "parts", Serialisation::SaveAsUnsignedIntOLD(parts) },
		{ "parent", Serialisation::SaveAsUnsignedLongLong(parentGUID)},
		{ "children", childrenGUIDs },
		{ "prefabStatus", Serialisation::SaveAsUnsignedIntOLD((unsigned int)prefabStatus)},
		{ "prefabBase", Serialisation::SaveAsUnsignedLongLong(prefabBase) }
	};
}

toml::table SceneObject::SerialiseWithParts() const
{
	auto safetyCheck = parts;

	toml::table table;
	table.emplace("sceneObject", Serialise());

	SavePart("modelRenderer", modelRenderer, renderers);
	if (Parts::animator & parts) {
		table.emplace("animator", scene->animators.at(GUID)->Serialise(GUID)); safetyCheck &= ~Parts::animator;
	};
	SavePart("rigidBody", rigidBody, rigidBodies);
	SavePart("health", health, healths);
	SavePart("enemy", enemy, enemies);
	SavePart("exitElevator", exitElevator, exits);
	SavePart("spawnManager", spawnManager, spawnManagers);
	SavePart("plate", plate, plates);
	SavePart("door", door, doors);
	SavePart("bollard", bollard, bollards);
	SavePart("freestyle", freestyle, freestyles);
	SavePart("triggerable", triggerable, triggerables);
	SavePart("spotlight", spotlight, spotlights); 
	SavePart("decal", decal, decals);
	SavePart("pointLight", pointLight, pointLights);
	SavePart("shadowWall", shadowWall, shadowWalls);

	if (Parts::collider & parts) {
		table.emplace("collider", scene->colliders.at(GUID)->Serialise(GUID));
		safetyCheck &= ~Parts::collider;
	}
	if (Parts::ecco & parts) {
		table.emplace("ecco", scene->ecco->Serialise());
		safetyCheck &= ~Parts::ecco;
	}
	if (Parts::sync & parts) {
		table.emplace("sync", scene->sync->Serialise());
		safetyCheck &= ~Parts::sync;
	}

	table.emplace("transform", transform()->Serialise(GUID));

	// TODO: Probably don't need a whole assert here, could just print a error and continue on
	assert(safetyCheck == 0);

	return table;
}

toml::table SceneObject::SerialiseWithPartsAndChildren() const
{
	toml::table table = SerialiseWithParts();

	toml::array savingChildren;
	auto children = transform()->getChildren();
	for (size_t i = 0; i < children.size(); i++)
	{
		savingChildren.push_back(children.at(i)->so->SerialiseWithPartsAndChildren());
	}

	table.emplace("children", savingChildren);

	return table;
}

SceneObject::SceneObject(Scene* _scene, toml::table* table) :
	scene(_scene)
{
	name = Serialisation::LoadAsString((*table)["name"]);
	GUID = Serialisation::LoadAsUnsignedLongLong((*table)["guid"]);
	parts = Serialisation::LoadAsUnsignedIntOLD((*table)["parts"]);
	scene->transforms[GUID].so = this;
	scene->sceneObjects[GUID] = this;
	prefabStatus = (PrefabStatus)Serialisation::LoadAsUnsignedIntOLD((*table)["prefabStatus"]);
	prefabBase = Serialisation::LoadAsUnsignedLongLong((*table)["prefabBase"]);
}

Transform* SceneObject::transform() const
{
	return &(scene->transforms.at(GUID));
}

// TODO: There is a case for the scene load to ensure that the below is also matched on scene load
// TODO: Try to remove any different special set / get
SetAndGetForPart(ModelRenderer, renderers, Parts::modelRenderer, Renderer, renderer)
void SceneObject::setAnimator(Animator* part) {
	if (part) {
		parts |= Parts::animator; scene->animators[GUID] = part;
		if (parts & Parts::modelRenderer) {
			renderer()->animator = scene->animators[GUID];
		}
	}
	else {
		parts &= ~Parts::animator; scene->animators.erase(GUID);
		if (parts & Parts::modelRenderer) {
			renderer()->animator = nullptr;
		}
	};
} 
Animator* SceneObject::animator() {
	if (parts & Parts::animator) {
		return (scene->animators.at(GUID));
	} return nullptr;
}
SetAndGetForPart(RigidBody, rigidBodies, Parts::rigidBody, RigidBody, rigidbody)
SetAndGetForPart(Health, healths, Parts::health, Health, health)
SetAndGetForPart(Enemy, enemies, Parts::enemy, Enemy, enemy)
SetAndGetForPart(ExitElevator, exits, Parts::exitElevator, ExitElevator, exitElevator)
SetAndGetForPart(SpawnManager, spawnManagers, Parts::spawnManager, SpawnManager, spawnManager)
SetAndGetForPart(PressurePlate, plates, Parts::plate, PressurePlate, plate)
SetAndGetForPart(Door, doors, Parts::door, Door, door)
SetAndGetForPart(Bollard, bollards, Parts::bollard, Bollard, bollard)
SetAndGetForPart(Freestyle, freestyles, Parts::freestyle, Freestyle, freestyle)
SetAndGetForPart(Triggerable, triggerables, Parts::triggerable, Triggerable, triggerable)
SetAndGetForPart(PointLight, pointLights, Parts::pointLight, PointLight, pointLight)
SetAndGetForPart(Decal, decals, Parts::decal, Decal, decal);
SetAndGetForPart(ShadowWall, shadowWalls, Parts::shadowWall, ShadowWall, shadowWall);
void SceneObject::setSpotlight(Spotlight* part) {
	if (part) {
		parts |= Parts::spotlight; scene->spotlights[GUID] = std::move(*part);
	}
	else {
		parts &= ~Parts::spotlight; scene->spotlights.erase(GUID);
	};
} Spotlight* SceneObject::spotlight() {
	if (parts & Parts::spotlight) {
		return &(scene->spotlights.at(GUID));
	} return nullptr;
}

void SceneObject::setCollider(Collider* collider)
{
	if (collider)
	{
		parts |= Parts::collider;
		scene->colliders[GUID] = collider;
	}
	else
	{
		parts &= ~Parts::collider;
		scene->colliders.erase(GUID);
	}
}

Collider* SceneObject::collider()
{
	if (parts & Parts::collider)
		return (scene->colliders[GUID]);
	return nullptr;
}

void SceneObject::setEcco(Ecco* ecco)
{
	if (ecco)
	{
		parts |= Parts::ecco;
		scene->ecco = ecco;
		scene->ecco->GUID = GUID;
	}
	else
	{
		parts &= ~Parts::ecco;
		if (scene->ecco && scene->ecco->GUID == GUID)
			scene->ecco->GUID = 0;
	}
}

void SceneObject::setEcco()
{
	parts |= Parts::ecco;	
	if(scene->ecco)
		scene->ecco->GUID = GUID;
}

Ecco* SceneObject::ecco() const
{
	if (parts & Parts::ecco)
		return scene->ecco;
	else return nullptr;
}

void SceneObject::setSync(Sync* sync)
{
	if (sync)
	{
		parts |= Parts::sync;
		scene->sync = sync;
		scene->sync->GUID = GUID;
	}
	else
	{
		parts &= ~Parts::sync;
		if (scene->sync && scene->sync->GUID == GUID)
			scene->sync->GUID = 0;
	}
}

void SceneObject::setSync()
{
	parts |= Parts::sync;
	if (scene->sync)
		scene->sync->GUID = GUID;
}

Sync* SceneObject::sync() const
{
	if (parts & Parts::sync)
		return scene->sync;
	else return nullptr;
}

void SceneObject::ClearParts(unsigned int toDelete)
{
	if (toDelete & parts & Parts::modelRenderer) { scene->renderers.erase(GUID);     parts &= ~(Parts::modelRenderer); }
	// TODO: If animator removed the model render needs to know about it
	if (toDelete & parts & Parts::animator)      { scene->animators.erase(GUID);     parts &= ~(Parts::animator);}
	if (toDelete & parts & Parts::rigidBody)     { scene->rigidBodies.erase(GUID);   parts &= ~(Parts::rigidBody);}
	if (toDelete & parts & Parts::collider)      { scene->colliders.erase(GUID);     parts &= ~(Parts::collider);}
	if (toDelete & parts & Parts::ecco)          { scene->ecco->GUID = 0;            parts &= ~(Parts::ecco);}
	if (toDelete & parts & Parts::sync)          { scene->sync->GUID = 0;            parts &= ~(Parts::sync);}
	if (toDelete & parts & Parts::health)        { scene->healths.erase(GUID);       parts &= ~(Parts::health);}
	if (toDelete & parts & Parts::enemy)         { scene->enemies.erase(GUID);       parts &= ~(Parts::enemy);}
	if (toDelete & parts & Parts::exitElevator)  { scene->exits.erase(GUID);         parts &= ~(Parts::exitElevator);}
	if (toDelete & parts & Parts::spawnManager)  { scene->spawnManagers.erase(GUID); parts &= ~(Parts::spawnManager);}
	if (toDelete & parts & Parts::plate)		 { scene->plates.erase(GUID);		 parts &= ~(Parts::plate);}
	if (toDelete & parts & Parts::door)		     { scene->doors.erase(GUID);	     parts &= ~(Parts::door);}	
	if (toDelete & parts & Parts::bollard)		 { scene->bollards.erase(GUID);	     parts &= ~(Parts::bollard);}	
	if (toDelete & parts & Parts::triggerable)   { scene->triggerables.erase(GUID);	 parts &= ~(Parts::triggerable);}	
	if (toDelete & parts & Parts::pointLight)    { scene->pointLights.erase(GUID);	 parts &= ~(Parts::pointLight);}	
	if (toDelete & parts & Parts::spotlight)	 { scene->spotlights.erase(GUID);	 parts &= ~(Parts::spotlight);}	
	if (toDelete & parts & Parts::decal)		 { scene->decals.erase(GUID);	     parts &= ~(Parts::decal);}	
	if (toDelete & parts & Parts::shadowWall)	 { scene->shadowWalls.erase(GUID);	 parts &= ~(Parts::shadowWall);}	
	if (toDelete & parts & Parts::freestyle)	 { scene->freestyles.erase(GUID);	 parts &= ~(Parts::freestyle);}	
}

void SceneObject::ClearParts()
{
	ClearParts(Parts::ALL);
	assert(parts == 0);
}

void SceneObject::SaveAsPrefab()
{
	prefabStatus = PrefabStatus::origin;

	std::ofstream file(Paths::prefabsSaveLocation + name + Paths::prefabExtension);

	toml::table table =	SerialiseWithPartsAndChildren();

	file << table << '\n';

	file.close();

	PrefabManager::loadedPrefabOriginals[GUID] = table;
}

void SceneObject::LoadWithParts(toml::table table)
{
	ClearParts();

	toml::table sceneObjectTable = *table["sceneObject"].as_table();
	prefabBase = Serialisation::LoadAsUnsignedLongLong(sceneObjectTable["guid"]);
	prefabStatus = (PrefabStatus)Serialisation::LoadAsUnsignedIntOLD(sceneObjectTable["prefabStatus"]);
	if (prefabStatus == PrefabStatus::origin) {
		prefabStatus = PrefabStatus::instance;
	}

	name = Serialisation::LoadAsString(sceneObjectTable["name"]);

	unsigned long long intendedParts = Serialisation::LoadAsUnsignedIntOLD(sceneObjectTable["parts"]);

	LoadPart("modelRenderer", modelRenderer, setRenderer, ModelRenderer);
	if (intendedParts & Parts::animator) {
		setAnimator(Animator::Load(*table["animator"].as_table()));
	};
	LoadPart("rigidBody", rigidBody, setRigidBody, RigidBody);
	LoadPart("health", health, setHealth, Health);
	LoadPart("enemy", enemy, setEnemy, Enemy);
	LoadPart("exitElevator", exitElevator, setExitElevator, ExitElevator);
	LoadPart("spawnManager", spawnManager, setSpawnManager, SpawnManager);
	LoadPart("plate", plate, setPressurePlate, PressurePlate);
	LoadPart("door", door, setDoor, Door);
	LoadPart("bollard", bollard, setBollard, Bollard);
	LoadPart("freestyle", freestyle, setFreestyle, Freestyle);
	LoadPart("triggerable", triggerable, setTriggerable, Triggerable);
	LoadPart("spotlight", spotlight, setSpotlight, Spotlight);
	LoadPart("pointLight", pointLight, setPointLight, PointLight);
	LoadPart("decal", decal, setDecal, Decal);
	LoadPart("shadowWall", shadowWall, setShadowWall, ShadowWall);

	if (intendedParts & Parts::collider) {
		setCollider(Collider::Load(*table["collider"].as_table()));
	}
	if (intendedParts & Parts::ecco) {
		// TODO: stop leaking memory
		scene->ecco = new Ecco(*table["ecco"].as_table());
		setEcco(scene->ecco);
	}
	if (intendedParts & Parts::sync) {
		// TODO: Stop leaking
		scene->sync = new Sync(*table["sync"].as_table());
		setSync(scene->sync);
	}

	auto loadingTransform = table["transform"];
	if (!loadingTransform) {
		// Transform info wasn't saved, this is okay for old prefabs
	}
	else {
		transform()->Load(*loadingTransform.as_table());
	}

	// TODO: Don't need a whole assert here
	assert(intendedParts == parts);
}

void SceneObject::LoadWithPartsSafe(toml::table table)
{
	// Remember information that should be kept
	std::string exitLevel = "";
	if (parts & Parts::exitElevator) {
		exitLevel = exitElevator()->levelToLoad;
	}
	bool hadSpawnManager = parts & Parts::spawnManager;
	toml::table spawnManagerData;
	if (hadSpawnManager) {
		spawnManagerData = spawnManager()->Serialise(GUID);
	}
	std::string pressurePlateTag = "";
	if (parts & Parts::plate) {
		pressurePlateTag = plate()->triggerTag;
	}
	std::string doorTag = "";
	if (parts & Parts::door) {
		doorTag = door()->triggerTag;
	}
	std::string bollardTag = "";
	if (parts & Parts::bollard) {
		bollardTag = bollard()->triggerTag;
	}
	std::string freestyleTag = "";
	if (parts & Parts::freestyle) {
		freestyleTag = freestyle()->triggerTag;
	}
	std::string triggerableTag = "";
	if (parts & Parts::triggerable) {
		triggerableTag = triggerable()->triggerTag;
	}
	std::string pointLightTag = "";
	if (parts & Parts::pointLight) {
		pointLightTag = pointLight()->triggerTag;
	}
	std::string spotlightTag = "";
	if (parts & Parts::spotlight) {
		spotlightTag = spotlight()->triggerTag;
	}


	// Load parts
	LoadWithParts(table);

	// Apply saved information
	if (parts & Parts::exitElevator && exitLevel != "") {
		exitElevator()->levelToLoad = exitLevel;
	}
	if (parts & Parts::plate && pressurePlateTag != "") {
		plate()->triggerTag = pressurePlateTag;
	}
	if (parts & Parts::door && doorTag != "") {
		door()->triggerTag = doorTag;
	}
	if (parts & Parts::bollard && bollardTag != "") {
		bollard()->triggerTag = bollardTag;
	}
	if (parts & Parts::freestyle && freestyleTag != "") {
		freestyle()->triggerTag = freestyleTag;
	}
	if (parts & Parts::triggerable && triggerableTag != "") {
		triggerable()->triggerTag = triggerableTag;
	}
	if (parts & Parts::spawnManager && hadSpawnManager) {
		spawnManager()->Load(spawnManagerData);
	}
	if (parts & Parts::pointLight && pointLightTag != "") {		
		pointLight()->triggerTag = pointLightTag;
	}
	if (parts & Parts::spotlight && spotlightTag != "") {
		spotlight()->triggerTag = spotlightTag;
	}

}

void SceneObject::LoadWithPartsSafeAndChildren(toml::table table)
{
	LoadWithPartsSafe(table);

	auto previousChildren = transform()->getChildren();
	auto temp = table["children"];
	if (!temp) {
		return;
	}

	auto loadingChildren = temp.as_array();
	if (loadingChildren->size() == 0) {
		return;
	}
	for (size_t i = 0; i < loadingChildren->size(); i++)
	{
		SceneObject* loadInto;
		if (i < previousChildren.size()) {
			// Load over the existing child object
			loadInto = previousChildren.at(i)->so;
		}
		else {
			// Create new sceneobject and load into that
			SceneObject* newChild = new SceneObject(scene);
			transform()->AddChild(newChild->transform());
			loadInto = newChild;
		}
		loadInto->LoadWithPartsSafeAndChildren(*loadingChildren->at(i).as_table());
	}
	// Clear extra children
	for (size_t i = loadingChildren->size(); i < previousChildren.size(); i++)
	{
		scene->DeleteSceneObjectAndChildren(previousChildren.at(i)->so->GUID);
	}
}

void SceneObject::LoadFromPrefab(toml::table table)
{
	// When loading directly from a prefab, the transform (of this object) should not be changed
	glm::mat4 originalTransform = transform()->getLocalMatrix();
	LoadWithPartsSafeAndChildren(table);
	prefabStatus = PrefabStatus::instance;
	transform()->setLocalMatrix(originalTransform);
}

void SceneObject::RefreshPrefab()
{
	auto search = PrefabManager::loadedPrefabOriginals.find(prefabBase);
	if (search != PrefabManager::loadedPrefabOriginals.end()) {
		LoadFromPrefab(PrefabManager::loadedPrefabOriginals.at(prefabBase));
	}
	else {
		prefabStatus = PrefabStatus::missing;
	}
}

void SceneObject::UnlinkFromPrefab()
{
	prefabStatus = PrefabStatus::missing;
}

void SceneObject::Duplicate() const
{
	SceneObject* newSceneObject = new SceneObject(scene);
	newSceneObject->LoadWithPartsSafeAndChildren(SerialiseWithPartsAndChildren());
	// Make the new object a sibling
	newSceneObject->transform()->setParent(transform()->getParent());
}
