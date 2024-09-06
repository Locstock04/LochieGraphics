#include "LevelEditor.h"

#include "ResourceManager.h"

// TODO: This is only here for the window reference
#include "SceneManager.h"

void LevelEditor::RefreshWalls()
{
	auto walls = wallTileParent->transform()->getChildren();
	
	while (walls.size())
	{
		unsigned long long GUID = walls.front()->getSceneObject()->GUID;
		delete sceneObjects[GUID];
		sceneObjects.erase(GUID);
		walls.erase(walls.begin());
	}
	wallCount = 0;


	float offset = (gridSize + wallThickness) / 2;
	for (auto& i : tiles)
	{
		glm::vec3 pos = i.second->transform()->getGlobalPosition();
		glm::vec2 tileCell = glm::vec2{ pos.x, pos.z } / gridSize;
		tileCell = { roundf(tileCell.x), roundf(tileCell.y) };
		if (!CellAt(tileCell.x - 1, tileCell.y)) {
			PlaceWallAt(pos.x - offset, pos.z, 0.0f);
		}
		if (!CellAt(tileCell.x, tileCell.y - 1)) {
			PlaceWallAt(pos.x, pos.z - offset, 90.0f);
		}
		if (!CellAt(tileCell.x + 1, tileCell.y)) {
			PlaceWallAt(pos.x + offset, pos.z, 0.0f);
		}
		if (!CellAt(tileCell.x, tileCell.y + 1)) {
			PlaceWallAt(pos.x, pos.z + offset, 90.0f);
		}
	}
}

SceneObject* LevelEditor::CellAt(float x, float z)
{
	auto tile = tiles.find({(int)x, (int)z});
	if (tile == tiles.end()) { return nullptr; }
	else { return tile->second; }
}

SceneObject* LevelEditor::PlaceWallAt(float x, float z, float direction)
{
	SceneObject* newWall = new SceneObject(this, "newWall" + std::to_string(++wallCount));
	newWall->transform()->setPosition({ x, 0.0f, z });
	newWall->transform()->setEulerRotation({0.0f, direction, 0.0f});
	newWall->transform()->setParent(wallTileParent->transform());
	newWall->setRenderer(new ModelRenderer(wall, (unsigned long long)0));
	// TODO: Add collider
	return newWall;
}

SceneObject* LevelEditor::PlaceTileAt(float x, float z)
{
	SceneObject* newTile = new SceneObject(this, "tile " + std::to_string(++tileCount));
	newTile->setRenderer(new ModelRenderer(ground, (unsigned long long)0));
	newTile->transform()->setPosition({ x * gridSize, 0.0f, z * gridSize });
	newTile->transform()->setParent(groundTileParent->transform());
	tiles[{(int)x, (int)z}] = newTile;
	return newTile;
}

LevelEditor::LevelEditor()
{

}

void LevelEditor::Start()
{
	lights.insert(lights.end(), {
	&directionalLight,
	&spotlight,
	&pointLights[0],
	&pointLights[1],
	&pointLights[2],
	&pointLights[3],
		});

	gui.showHierarchy = true;
	gui.showSceneObject = true;
	gui.showCameraMenu = true;

	camera->transform.setPosition({ 0, 50, 0 });
	camera->orthoScale = 300;
	
	camera->editorSpeed.move = 300;
	camera->farPlane = 100000;
	camera->nearPlane = 10;

	// TODO: Don't load directly here, should be saved as an art asset
	ground = ResourceManager::LoadModel("models/SM_FloorTile.fbx");
	wall = ResourceManager::LoadModel("models/adjustedOriginLocWall.fbx");

	syncSo = new SceneObject(this, "Sync");
	eccoSo = new SceneObject(this, "Ecco");


	RigidBody* hRb = new RigidBody();
	hRb->setMass(1.0f);
	hRb->addCollider({ new PolygonCollider({{0.0f, 0.0f}}, syncRadius) });
	hRb->setMomentOfInertia(5.0f);

	RigidBody* rRb = new RigidBody();
	rRb->addCollider({ new PolygonCollider(
			{
				{40.0f, 40.0f},
				{40.0f, -40.0f},
				{-40.0f, -40.0f},
				{-40.0f, 40.0f},
			}, 0.0f) }
	);
	rRb->setMass(0.1f);
	rRb->setMomentOfInertia(5.0f);

	syncSo->transform()->setPosition({ 1.0f,0.0f,1.0f });

	input.Initialise();

	syncSo->setRigidBody(hRb);
	eccoSo->setRigidBody(rRb);

	hRb = &rigidBodies[sync->GUID];
	rRb = &rigidBodies[ecco->GUID];
	syncSo->setSync(sync);
	eccoSo->setEcco(ecco);
	ecco->wheelDirection = { eccoSo->transform()->forward().x, eccoSo->transform()->forward().y };

	gameCamSystem.cameraPositionDelta = { -150.0f, 100.0f, 150.0f };

	// TODO: Should be using an art asset
	eccoSo->setRenderer(new ModelRenderer(ResourceManager::LoadModel("models/EccoBlockout_RevisedScale.fbx"), (unsigned long long)0));
	camera->transform.setRotation(glm::quat(0.899f, -0.086f, 0.377f, -0.205f));

}

void LevelEditor::Update(float delta)
{
	LineRenderer& lines = renderSystem->lines;
	input.Update();

	physicsSystem.CollisionCheckPhase(transforms, rigidBodies, colliders);
	physicsSystem.UpdateRigidBodies(transforms, rigidBodies, delta);
	if (input.inputDevices.size() > 0)
	{
		ecco->Update(
			*input.inputDevices[0],
			*eccoSo->transform(),
			*eccoSo->rigidbody(),
			delta
		);

		if (input.inputDevices.size() > 1)
		{
			sync->Update(
				*input.inputDevices[1],
				*syncSo->transform(),
				*syncSo->rigidbody(),
				delta
			);
		}
	}

	gameCamSystem.Update(*camera, *eccoSo->transform(), *syncSo->transform(), 0.5f);

	lines.DrawLineSegment({ 0, 0, 0 }, { 1, 1, 1 }, { 1.0f, 0.5f, 0.2f });
	lines.DrawLineSegment(testPos1, testPos2, {1.0f, 1.0f, 1.0f});
	
	lines.SetColour({ 1, 1, 1 });
	lines.AddPointToLine({ gridSize * gridMinX - gridSize - gridSize / 2.0f, 0.0f, gridSize * gridMinZ - gridSize - gridSize / 2.0f });
	lines.AddPointToLine({ gridSize * gridMinX - gridSize - gridSize / 2.0f, 0.0f, gridSize * gridMaxZ + gridSize + gridSize / 2.0f });
	lines.AddPointToLine({ gridSize * gridMaxX + gridSize + gridSize / 2.0f, 0.0f, gridSize * gridMaxZ + gridSize + gridSize / 2.0f });
	lines.AddPointToLine({ gridSize * gridMaxX + gridSize + gridSize / 2.0f, 0.0f, gridSize * gridMinZ - gridSize - gridSize / 2.0f});
	lines.FinishLineLoop();

	glm::vec3 camPoint = camera->transform.getGlobalPosition();
	camPoint = glm::vec3(camPoint.x, camPoint.y, camPoint.z);

	glm::vec2 adjustedCursor = *cursorPos - glm::vec2{ 0.5f, 0.5f };
	glm::vec3 temp = (camPoint + glm::vec3(adjustedCursor.x * camera->getOrthoWidth(), 0.0f, -adjustedCursor.y * camera->getOrthoHeight())) / gridSize;
	glm::vec2 targetCell = glm::vec2{ roundf(temp.x), roundf(temp.z) };

	// TODO: Skip if imgui wants mouse input
	if (ImGui::GetIO().WantCaptureMouse) { return; }
	if (state != BrushState::none && glfwGetMouseButton(SceneManager::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		SceneObject* alreadyPlaced = CellAt(targetCell.x, targetCell.y);
		if (!alreadyPlaced) {
			PlaceTileAt(targetCell.x, targetCell.y);
			// other setup here
			gridMinX = (int)fminf(targetCell.x, (float)gridMinX);
			gridMinZ = (int)fminf(targetCell.y, (float)gridMinZ);
			gridMaxX = (int)fmaxf(targetCell.x, (float)gridMaxX);
			gridMaxZ = (int)fmaxf(targetCell.y, (float)gridMaxZ);

			if (alwaysRefreshWallsOnPlace) { RefreshWalls(); }
		}
	}
	// TODO: Refresh mins and maxes
	if (state != BrushState::none && glfwGetMouseButton(SceneManager::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		SceneObject* alreadyPlaced = CellAt(targetCell.x, targetCell.y);
		if (alreadyPlaced) {
			unsigned long long GUID = alreadyPlaced->GUID;
			// TODO: Might be better to have a mark for deletion type of thing and gets deleted afterwards
			// Could get put into some collection somewhere
			delete sceneObjects[GUID];
			sceneObjects.erase(GUID);
			tiles.erase({ (int)targetCell.x, (int)targetCell.y });

			if (alwaysRefreshWallsOnPlace) { RefreshWalls(); }
		}
	}
}

void LevelEditor::Draw()
{
	renderSystem->Update(
		renderers,
		transforms,
		renderers,
		animators,
		camera
	);
}


void LevelEditor::GUI()
{
	if (ImGui::Begin("Level Editor")) {
		if (ImGui::Combo("Brush Mode", (int*)&state, "None\0Brush\0\0")) {
			switch (state)
			{
			case LevelEditor::BrushState::none:
				camera->state = Camera::State::editorMode;
				break;
			case LevelEditor::BrushState::brush:
				camera->state = Camera::State::tilePlacing;
				camera->transform.setEulerRotation({ -90.0f, 0.0f, -90.0f });
				glm::vec3 pos = camera->transform.getPosition();
				pos.y = fmaxf(pos.y, 600);
				camera->transform.setPosition(pos);
				break;
			default:
				break;
			}
		}

		ImGui::Checkbox("Always refresh walls", &alwaysRefreshWallsOnPlace);
		if (!alwaysRefreshWallsOnPlace) {
			if (ImGui::Button("Refresh Walls")) {
				RefreshWalls();
			}
		}
		if (ImGui::DragFloat("Wall offset", &wallThickness, 0.5f)) {
			if (alwaysRefreshWallsOnPlace) { RefreshWalls(); }
		}
	}
	ImGui::End();

	SaveAsPrompt();
	LoadPrompt();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save As")) {
				openSaveAs = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	input.GUI();
}

void LevelEditor::SaveAsPrompt()
{
	if (openSaveAs) {
		ImGui::OpenPopup("Save as");
	}
	if (!ImGui::BeginPopupModal("Save as", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (openSaveAs) {
		ImGui::SetKeyboardFocusHere();
		openSaveAs = false;
	}
	if (ImGui::InputText("Filename##Save", &windowName, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
		ImGui::CloseCurrentPopup();
		previouslySaved = true;
		SaveLevel();
	}
	if (ImGui::Button("Save")) {
		ImGui::CloseCurrentPopup();
		previouslySaved = true;
		SaveLevel();
	}
	if (ImGui::Button("Cancel##Load")) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void LevelEditor::LoadPrompt()
{
	if (openLoad) {
		ImGui::OpenPopup("Load Level");
	}
	if (!ImGui::BeginPopupModal("Load Level", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (openLoad) {
		ImGui::SetKeyboardFocusHere();
		openLoad = false;
	}
	if (ImGui::InputText("Filename##Load", &windowName, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
		ImGui::CloseCurrentPopup();
		previouslySaved = true;
		LoadLevel();
	}
	if (ImGui::Button("Cancel##Load")) {
		ImGui::CloseCurrentPopup();
	}



	ImGui::EndPopup();
}

void LevelEditor::SaveLevel()
{
	std::ofstream file("Levels/" + windowName + ".level");

	file << SaveSceneObjectsAndParts();

	file.close();
}

void LevelEditor::LoadLevel()
{
	std::ifstream file("Levels/" + windowName + ".level");

	toml::table data = toml::parse(file);

	DeleteAllSceneObjects();

	LoadSceneObjectsAndParts(data);


	// TODO:
	
	groundTileParent = FindSceneObjectOfName("Ground Tiles");
	wallTileParent = FindSceneObjectOfName("Wall Tiles");

	// Refresh the tiles collection
	tiles.clear();
	auto children = groundTileParent->transform()->getChildren();
	for (size_t i = 0; i < children.size(); i++)
	{
		glm::vec3 adjustedPos = children[i]->getPosition() / gridSize;
		tiles[{(int)adjustedPos.x, (int)adjustedPos.z}] = children[i]->getSceneObject();
	}



	file.close();
}

LevelEditor::~LevelEditor()
{
}

void LevelEditor::Save()
{
	if (!previouslySaved) {
		openSaveAs = true;
	}
	else {
		SaveLevel();
	}
}

void LevelEditor::Load()
{
	openLoad = true;
}