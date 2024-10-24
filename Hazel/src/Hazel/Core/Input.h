#pragma once

#include <glm/glm.hpp>

#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"

namespace Hazel 
{
	class Input
	{
	protected:
		Input() = default;
	public:
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}