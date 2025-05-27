#pragma once

#include <string>

#include <set>

struct Collider;
class Scene;
class Shader;
class Transform;
class ModelRenderer;
class Ecco;
class Sync;
class Health;
class Animator;
class RigidBody;
class ExitElevator;
class SpawnManager;
class LineRenderer;
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

namespace toml {
	inline namespace v3 {
		class table;
	}
}

// TODO: There might be issues later due to the parts being saved as an signed int, not unsigned
// Changing how they are saved can affect other saves and isn't worth it at the moment of writing
enum Parts : unsigned int
{
	modelRenderer = 1 << 1,
	animator =      1 << 2,
	rigidBody =     1 << 3,
	pointLight =    1 << 4,
	collider =      1 << 5,
	ecco =          1 << 6,
	sync =          1 << 7,
	health =        1 << 8,
	enemy =         1 << 9,
	exitElevator =	1 << 10,
	spotlight =		1 << 11,
	plate = 		1 << 12,
	spawnManager =	1 << 13,
	door		 =	1 << 14,
	bollard		 =	1 << 15,
	triggerable	 =	1 << 16,
	decal		 =  1 << 17,
	shadowWall	 =  1 << 18,
	freestyle	 =  1 << 19,
	ALL = (1 << 20) - 1 - 1,
}; // NOTE: The all has a minus 1 because it needs to be flipped to be all 1s, and another minus one as there is no part with the number 1

class Scene;

class SceneObject
{
public:
	enum class PrefabStatus : unsigned int{
		none = 0,
		origin = 1,
		instance = 2,
		missing = 3,
	};

	std::string name = "Unnamed Scene Object";
	unsigned long long GUID = 0ull;
	unsigned int parts = 0u;
	Scene* scene = nullptr;

	PrefabStatus prefabStatus = PrefabStatus::none;
	unsigned long long prefabBase = 0ull;

	SceneObject(Scene* _scene, std::string name = "Unnamed Scene Object");
	SceneObject(Scene* _scene, toml::table* table);

	~SceneObject();

	//SceneObject(const SceneObject& other) = delete;
	//SceneObject& operator=(const SceneObject& other) = delete;

	void GUI();
	// This is the right click menu
	void MenuGUI();
	static void MultiMenuGUI(std::set<SceneObject*> multiSelectedSceneObjects, bool* setNullSelect);

	static bool PartsFilterSelector(const std::string& label, unsigned int& parts);

	void DebugDraw();

	void TriggerCall(std::string tag, bool toggle);

	toml::table Serialise() const;
	toml::table SerialiseWithParts() const;
	toml::table SerialiseWithPartsAndChildren() const;

	void SaveAsPrefab();
	void LoadWithParts(toml::table table);
	// Keeps certain information about the sceneobjects parts that shouldn't be replaced (like tags)
	void LoadWithPartsSafe(toml::table table);
	void LoadWithPartsSafeAndChildren(toml::table table);
	void LoadFromPrefab(toml::table table);
	void RefreshPrefab();

	void UnlinkFromPrefab();

	void Duplicate() const;

#pragma region Part Get and Set
	Transform* transform() const;

	void setRenderer(ModelRenderer* renderer);
	ModelRenderer* renderer();

	void setAnimator(Animator* animator);
	Animator* animator();

	void setPointLight(PointLight* pointLight);
	PointLight* pointLight();

	void setSpotlight(Spotlight* spotlight);
	Spotlight* spotlight();

	void setDecal(Decal* decal);
	Decal* decal();

	void setRigidBody(RigidBody* rb);
	// TODO: Should be a cap b to match the rest?
	RigidBody* rigidbody();

	void setCollider(Collider* collider);
	Collider* collider();

	void setEcco(Ecco* ecco);
	void setEcco();
	Ecco* ecco() const;

	void setSync(Sync* sync);
	void setSync();
	Sync* sync() const;

	void setHealth(Health* health);
	Health* health();

	void setEnemy(Enemy* enemy);
	Enemy* enemy();

	void setExitElevator(ExitElevator* exitElevator);
	ExitElevator* exitElevator();

	void setSpawnManager(SpawnManager* spawnManager);
	SpawnManager* spawnManager();

	void setPressurePlate(PressurePlate* plate);
	PressurePlate* plate();

	void setDoor(Door* door);
	Door* door();

	void setBollard(Bollard* bollard);
	Bollard* bollard();

	void setFreestyle(Freestyle* freestyle);
	Freestyle* freestyle();

	void setTriggerable(Triggerable* triggerable);
	Triggerable* triggerable();

	void setShadowWall(ShadowWall* shadowWall);
	ShadowWall* shadowWall();

#pragma endregion

	void ClearParts();
	void ClearParts(unsigned int toDelete);
};



