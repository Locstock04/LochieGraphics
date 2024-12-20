#pragma once

#include "Maths.h"

#include <vector>
#include <string>

struct aiNodeAnim;

struct Key {
	glm::vec3 position;
	glm::quat orientation;
	glm::vec3 scale;
	
	float timeStamp;
};

struct PosKey {
	glm::vec3 value;
	float timeStamp;
};
struct RotKey {
	glm::quat orientation;
	float timeStamp;
};
struct ScaleKey {
	glm::vec3 scale;
	float timeStamp;
};

// TODO: Rethink if want this store the current bone transform, that should probably be apart of the animator
/// <summary>
/// Stores the animation data and transform for a specific bone and the current bone transform
/// </summary>
class Bone {
public:
	// The keys
	//std::vector<Key> keys;
	std::vector<PosKey> posKeys;
	std::vector<RotKey> rotKeys;
	std::vector<ScaleKey> scaleKeys;

	// TODO: This shouldn't be stored here, this class should only hold the animation data, there should still be an interpolate/update, but the result should be stored elsewhere, such as on the animator
	// The local transformation of the bone.
	glm::mat4 localTransform;

	// The name of the bone.
	std::string name;

	// TODO: Not being used, could be removed if not needed
	// The ID of the bone
	int ID;

	/// <summary>
	/// Creates a bone and populates the positions, rotations and scales vectors using the aiNodeAnim.
	/// </summary>
	/// <param name="_name">The name of the bone</param>
	/// <param name="_ID">The bone ID</param>
	/// <param name="channel">The assimp node anim channel</param>
	Bone(const std::string& _name, int _ID, const aiNodeAnim* channel);

	/// <summary>
	/// Interpolates the bones to recalculate the local transform.
	/// </summary>
	/// <param name="animationTime">The time since the start of the animation</param>
	void Update(float animationTime);

	/// <summary>
	/// Returns a const reference to the local transform.
	/// </summary>
	/// <returns>const reference of the mat4 local transform</returns>
	const glm::mat4& getLocalTransform() const;

	/// <summary>
	/// Returns a const reference to the bone name.
	/// </summary>
	/// <returns>const reference of the string bone name</returns>
	const std::string& getBoneName() const;

	/// <summary>
	/// Retuns the bone index.
	/// </summary>
	/// <returns>The bone index as an int</returns>
	int getBoneID() const;

	int getPosIndex(float animationTime) const;
	int getRotIndex(float animationTime) const;
	int getScaleIndex(float animationTime) const;

	void GUI();

private:
	
	float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const;

	glm::mat4 Interpolate(float animationTime);

};