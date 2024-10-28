#pragma once

#include <string>

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
struct Enemy;
namespace toml {
	inline namespace v3 {
		class table;
	}
}
// TODO: Look into how instancing should be handled

// TODO: There might be issues later due to the parts being saved as an signed int, not unsigned
// Changing how they are saved can affect other saves and isn't worth it at the moment of writing
enum Parts : unsigned int
{
	modelRenderer = 1 << 1,
	animator =      1 << 2,
	rigidBody =     1 << 3,
	light =         1 << 4,
	collider =      1 << 5,
	ecco =          1 << 6,
	sync =          1 << 7,
	health =        1 << 8,
	enemy =         1 << 9,
	exitElevator =	1 << 10,
	spikes =		1 << 11,
	plate = 		1 << 12,
	spawnManager =	1 << 13,
	door		 =	1 << 14,
	bollard		 =	1 << 15,
	triggerable	 =	1 << 16,
	ALL = (1 << 16) - 1 - 1,
};

class Scene;

class SceneObject
{
public:
	enum class PrefabStatus {
		none,
		prefabOrigin,
		prefabInstance
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

	void DebugDraw();

	void TriggerCall(std::string tag, bool toggle);

	toml::table Serialise() const;

	void SaveAsPrefab();
	void LoadFromPrefab(toml::table table);

#pragma region Part Get and Set

	// TODO: Consider getting rid of the setTransform?
	void setTransform(Transform* transform);
	Transform* transform() const;

	void setRenderer(ModelRenderer* renderer);
	ModelRenderer* renderer();

	void setAnimator(Animator* animator);
	Animator* animator();

	//void setLight(Light light);
	//Light* light();

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

	void setTriggerable(Triggerable* triggerable);
	Triggerable* triggerable();

#pragma endregion

	// Make this take a mask for stuff to delete
	void ClearParts();
	void ClearParts(unsigned int toDelete);
};



