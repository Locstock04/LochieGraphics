#pragma once

#include "Bone.h"
#include "Model.h"
#include "NodeData.h"

#include "assimp/scene.h"

#include <string>

class Animation
{
private:
	// The duration of the animation, how long the animation goes for in seconds.
	float duration = 0;

	// How many ticks/'frames per second' for this animation.
	float ticksPerSecond = 0.f;

	// The bones apart of the model for this animation
	std::vector<Bone> bones = {};

	// The root node of the bones
	NodeData rootNode;

	// String to bone infos
	std::unordered_map<std::string, BoneInfo> boneInfoMap;

public:
	Animation() {};

	/// <summary>
	/// Creates an animation from the animationPath for a given model.
	/// </summary>
	/// <param name="animationPath">The path to the animation file</param>
	/// <param name="model">The model this animation is for</param>
	Animation(const std::string& animationPath, Model* model);
	
	/// <summary>
	/// Returns a pointer to the bone with the given name.
	/// </summary>
	/// <param name="name">The name of the bone to find</param>
	/// <returns>A Bone pointer</returns>
	Bone* FindBone(const std::string& name);
	
	
	float getTicksPerSecond() const;

	float getDuration() const;

	const NodeData& getRootNode() const;

	const std::unordered_map<std::string, BoneInfo>& getBoneIDMap() const;

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model);

	void ReadHierarchyData(NodeData& dest, const aiNode* src);
};
