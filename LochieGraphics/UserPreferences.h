#pragma once

#include <string>

#include "Maths.h"

class UserPreferences
{
public:
	static float camMove;
	static float camRotate;
	static float camOrbit;
	static float camBoomTruck;
	static float camMoveDolly;
	static float camScrollDolly;
	static float orthScrollSpeed;

	static std::string filename;

	static bool escapeCloses;

	static bool clearSearchBar;
	
	enum class ModelSelectMode {
		loaded,
		assets
	};
	static ModelSelectMode modelSelectMode;

	static bool saveOnLevelPlay;
	static bool immortal;
	static bool loadDefaultLevel;
	static std::string defaultLevelLoad;
	static bool rememberLastLevel;
	static bool enterPlayModeOnStart;
	static int windowedStartMode;

	static std::string defaultCameraSystemLoad;
	static std::string defaultEnemySystemLoad;
	static std::string defaultHealthSystemLoad;
	static std::string defaultStyleLoad;

	static bool advancedTransformInfo;
	static bool showModelHierarchy;
	static bool showSelectedBox;

	static float defaultGlobalVolume;

	static glm::vec3 loadedDirectionalLightDirection;
	static glm::vec3 loadedDirectionalLightColour;

	static void GUI();

	static void Initialise();

	static void RefreshPreferenceFile();

	static void Save();
	static bool Load();
};

