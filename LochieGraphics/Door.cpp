#include "Door.h"
#include "Serialisation.h"
#include "ExtraEditorGUI.h"
#include "SceneManager.h"
#include "Scene.h"

Door::Door(toml::table table)
{
	left = Serialisation::LoadAsBool(table["left"]);
	startClosed = Serialisation::LoadAsBool(table["closed"]);
	ignoresFalse = Serialisation::LoadAsBool(table["ignoresFalse"]);
	amountToMove = Serialisation::LoadAsFloat(table["amountToMove"]);
	timeToClose = Serialisation::LoadAsFloat(table["timeToClose"]);
	timeToOpen = Serialisation::LoadAsFloat(table["timeToOpen"]);
	triggerTag = Serialisation::LoadAsString(table["triggerTag"]);
}

toml::table Door::Serialise(unsigned long long guid)
{
	return toml::table{
		{"guid", Serialisation::SaveAsUnsignedLongLong(guid)},
		{"left", left},
		{"closed", startClosed},
		{"ignoresFalse", ignoresFalse},
		{"amountToMove", amountToMove},
		{"timeToClose", timeToClose},
		{"timeToOpen", timeToOpen},
		{"triggerTag", triggerTag},
	};
}

void Door::TriggerCall(std::string tag, bool toggle)
{
	if (ignoresFalse && !toggle) return;
	if (tag == triggerTag)
	{
		if (state != toggle)
		{
			float randPercentage = std::rand() / (float)RAND_MAX;

			int index = floor(randPercentage * 26.0f);
			SceneManager::scene->audio.PlaySound((Audio::SoundIndex)(index + (int)Audio::dabMove00));
		}
		state = toggle;
	}
}

void Door::GUI()
{
	std::string tag = Utilities::PointerToString(this);
	if (ImGui::CollapsingHeader(("Door##" + tag).c_str()))
	{
		ImGui::Indent();
		ImGui::Checkbox("Starts Closed", &startClosed);
		ImGui::Checkbox("Moves Left", &left);
		ImGui::SameLine();
		bool right = !left;
		if (ImGui::Checkbox("Moves Right", &right)) left = !right;
		ImGui::Checkbox("Ignores False", &ignoresFalse);
		ImGui::InputText("Trigger Tag", &triggerTag);
		ImGui::DragFloat("Amount To Move", &amountToMove, 2.0f, 0.0f);
		ImGui::DragFloat("Time To Close", &timeToClose, 0.02f, 0.0f);
		ImGui::DragFloat("Time To Open", &timeToOpen, 0.02f, 0.0f);

		ImGui::BeginDisabled();
		ImGui::DragFloat(("Time in movement##" + tag).c_str(), &timeInMovement);
		ImGui::DragFloat3(("Starting Pos##" + tag).c_str(), &startingPos.x);
		ImGui::Checkbox(("State##" + tag).c_str(), &state);
		ImGui::EndDisabled();

		ImGui::Unindent();
	}
}
