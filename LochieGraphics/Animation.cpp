#include "Animation.h"

#include "AssimpMatrixToGLM.h"

#include <assimp/Importer.hpp>

#include <iostream>

Animation::Animation(const std::string& animationPath, Model* model)
{
	Assimp::Importer importer;
	// TODO: what flags should be set here
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	
	// If either the scene or the root node is null, then the animation has failed to load
	if (!(scene && scene->mRootNode)) {
		std::cout << "Error: Failed to load animation at: " << animationPath << "\n";
		return;
	}

	// Get animation info from the aiAnimation in the scene
	aiAnimation* animation = scene->mAnimations[0];
	duration = (float)animation->mDuration;
	ticksPerSecond = (float)animation->mTicksPerSecond;
	
	// TODO: use this
	aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
	globalTransformation = globalTransformation.Inverse();


	ReadHierarchyData(rootNode, scene->mRootNode);
	ReadMissingBones(animation, *model);
}

Bone* Animation::FindBone(const std::string& name)
{
	// TODO: A lot of iterators are used just to find stuff, see if it is something to be using more
	auto search = std::find_if(bones.begin(), bones.end(), [&](const Bone& bone) {
		return bone.getBoneName() == name;
	});

	if (search == bones.end()) {
		return nullptr;
	}
	return &(*search);

	//for (auto i = bones.begin(); i != bones.end(); i++)
	//{
	//	if (i->getBoneName() == name) {
	//		return &(*i);
	//	}
	//}
	//return nullptr;
}

float Animation::getTicksPerSecond() const
{
	return ticksPerSecond;
}

float Animation::getDuration() const
{
	return duration;
}

const NodeData& Animation::getRootNode() const
{
	return rootNode;
}

const std::unordered_map<std::string, BoneInfo>& Animation::getBoneIDMap() const
{
	return boneInfoMap;
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
	std::unordered_map<std::string, BoneInfo>& newBoneInfoMap = model.boneInfoMap;

	for (int i = 0; i < animation->mNumChannels; i++)
	{
		aiNodeAnim* channel = animation->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (newBoneInfoMap.find(boneName) == newBoneInfoMap.end()) {
			newBoneInfoMap[boneName].ID = boneInfoMap.size() + 1;
		}
		bones.push_back(Bone(channel->mNodeName.data, newBoneInfoMap[channel->mNodeName.data].ID, channel));
	}
	boneInfoMap = newBoneInfoMap;
}

void Animation::ReadHierarchyData(NodeData& dest, const aiNode* src)
{
	if (!src) {
		std::cout << "Error, Failed to read animation hierarchy\n";
		return;
	}

	dest.name = src->mName.data;
	dest.transformation = AssimpMatrixToGLM(src->mTransformation);
	dest.children.reserve(src->mNumChildren);

	for (unsigned int i = 0; i < src->mNumChildren; i++)
	{
		NodeData newData;
		ReadHierarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}