#include "Input.h"

#include "SceneManager.h"

#include "Graphics.h"

#include <iostream>

Input* Input::input = nullptr;

void Input::Initialise()
{
	input = this;
	glfwSetJoystickCallback(JoystickCallback);

	for (int i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; i++)
	{
		if (glfwJoystickPresent(i)) {
			if (glfwJoystickIsGamepad(i)) {
				AddGamepad(i);
			}
			else {
				// Unrecognised controller
			}
		}
	}
}

void Input::JoystickCallback(int id, int event)
{
	input->JoystickChange(id, event);
}

void Input::ShowAllControllerSlotStatuses()
{
	std::cout << "JOYSTICK STATUSES\n";
	for (int i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; i++)
	{
		int present = glfwJoystickPresent(i);
		if (present == GLFW_TRUE) {
			std::cout << "Joystick #" << i << " is connected\n";
			if (glfwJoystickIsGamepad(i)) {
				std::cout << "\tRecognised as a gamepad\n";
				std::cout << "\tJoystick name: " << glfwGetJoystickName(i) << "\n";
				std::cout << "\tGamepad name:  " << glfwGetGamepadName(i) << "\n";
			}
		}
	}
}

void Input::Update()
{
	for (Inputter* i : inputters)
	{
		if (i->getType() != Type::Controller) {
			continue;
		}
		Controller* c = (Controller*)i;
		c->previousState = c->currentState;
		glfwGetGamepadState(c->id, &c->currentState);
	}
}

void Input::AddGamepad(int id)
{
	inputters.push_back(new Controller(id));
}

void Input::AddKeyboard()
{
	inputters.push_back(new Keyboard());
}

void Input::JoystickChange(int id, int event) {
	if (event == GLFW_CONNECTED) {
		std::cout << "Joystick: " << id << " connected\n";
		if (glfwJoystickIsGamepad(id)) {
			AddGamepad(id);
		}
		else {
			// Unrecognised controller
		}
	}
	else if (event == GLFW_DISCONNECTED) {
		std::cout << "Joystick: " << id << " disconnected\n";
		for (auto i = inputters.begin(); i != inputters.end(); i++)
		{
			if ((*i)->getType() != Type::Controller) {
				continue;
			}
			Controller* c = (Controller*)(*i);
			if (c->id == id) {
				inputters.erase(i);
				break;
			}
		}
	}
}

Input::Controller::Controller(int _id) :
	id(_id)
{
	glfwGetGamepadState(id, &currentState);
	previousState = currentState;
}

glm::vec2 Input::Controller::getMove() const
{
	return { currentState.axes[GLFW_GAMEPAD_AXIS_LEFT_X], currentState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] };
}

glm::vec2 Input::Controller::getLook() const
{
	return { currentState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], currentState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] };
}

Input::Type Input::Controller::getType() const
{
	return Type::Controller;
}

glm::vec2 Input::Keyboard::getMove() const
{
	int horizontal = glfwGetKey(SceneManager::window, keyMoveRight) - glfwGetKey(SceneManager::window, keyMoveLeft);
	int vertical = glfwGetKey(SceneManager::window, keyMoveUp) - glfwGetKey(SceneManager::window, keyMoveDown);
	return glm::vec2(horizontal, vertical);
}

glm::vec2 Input::Keyboard::getLook() const
{
	int horizontal = glfwGetKey(SceneManager::window, keyLookRight) - glfwGetKey(SceneManager::window, keyLookLeft);
	int vertical = glfwGetKey(SceneManager::window, keyLookUp) - glfwGetKey(SceneManager::window, keyLookDown);
	return glm::vec2(horizontal, vertical);
}

Input::Type Input::Keyboard::getType() const
{
	return Type::Keyboard;
}