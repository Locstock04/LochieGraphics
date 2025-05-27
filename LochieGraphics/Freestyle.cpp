#include "Freestyle.h"
#include "Serialisation.h"
#include "ExtraEditorGUI.h"

Freestyle::Freestyle(toml::table table)
{
	triggerTag = Serialisation::LoadAsString(table["triggerTag"]);
	boolean = Serialisation::LoadAsBool(table["boolean"]);
	vector2 = Serialisation::LoadAsVec2(table["vector2"]);
	vector3 = Serialisation::LoadAsVec3(table["vector3"]);
	colour = Serialisation::LoadAsVec4(table["colour"]);
	i = Serialisation::LoadAsInt(table["i"]);
	f = Serialisation::LoadAsFloat(table["f"]);
	string = Serialisation::LoadAsString(table["string"]);
}

void Freestyle::TriggerCall(std::string tag, bool toggle)
{
	if (tag == triggerTag)
	{
		//DO SOMETHING
		//DO SOMETHING
	}
}

toml::table Freestyle::Serialise(unsigned long long guid)
{
	return toml::table
	{
		{"guid", Serialisation::SaveAsUnsignedLongLong(guid)},
		{"triggerTag", triggerTag},
		{"boolean", boolean},
		{"vector2", Serialisation::SaveAsVec2(vector2)},
		{"vector3", Serialisation::SaveAsVec3(vector3)},
		{"colour", Serialisation::SaveAsVec4(colour)},
		{"i", i},
		{"f", f},
		{"string", string},
	};
}

void Freestyle::GUI()
{
	std::string tag = Utilities::PointerToString(this);
	if (ImGui::CollapsingHeader(("Freestyle##" + tag).c_str()))
	{
		ImGui::Indent();
		ImGui::InputText("Trigger Tag", &triggerTag);
		ImGui::ColorPicker4(("Colour##" + tag).c_str(), &colour[0]);
		ImGui::DragFloat2(("Vector2##" + tag).c_str(), &vector2[0]);
		ImGui::DragFloat3(("Vector3##" + tag).c_str(), &vector3[0]);
		ImGui::DragInt(("Integer##" + tag).c_str(), &i);
		ImGui::DragFloat(("Float##" + tag).c_str(), &f);
		ImGui::InputText("String", &string);
		ImGui::Checkbox("Boolean", &boolean);
		ImGui::Unindent();
	}
}