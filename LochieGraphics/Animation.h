#pragma once

#include "Bone.h"

#include <unordered_map>

class Model;
class ModelHierarchyInfo;
struct aiAnimation;
struct BoneInfo;

// TODO: Needs a GUID
class Animation
{
public:
	// TODO: This comment might be wrong, unit might be ticks
	// The duration of the animation, how long the animation goes for in seconds.
	float duration = 0;

	// How many ticks/'frames per second' for this animation.
	float ticksPerSecond = 0.f;

	// The animation bones with animation key data
	std::vector<Bone> bones = {};

	Model* model = nullptr;
	unsigned long long modelGUID = 0;

	Animation() {};

	/// <summary>
	/// Creates an animation from the animationPath for a given model.
	/// </summary>
	/// <param name="animationPath">The path to the animation file</param>
	/// <param name="model">The model this animation is for</param>
	Animation(const std::string& animationPath, Model* _model);
	
	/// <summary>
	/// Returns a pointer to the bone with the given name.
	/// </summary>
	/// <param name="name">The name of the bone to find</param>
	/// <returns>A Bone pointer</returns>
	Bone* FindBone(const std::string& name);
	
	float getTicksPerSecond() const;

	float getDuration() const;

	const ModelHierarchyInfo* getRootNode() const;

	void GUI();

private:
	void ReadMissingBones(const aiAnimation* animation, Model* model);
};

