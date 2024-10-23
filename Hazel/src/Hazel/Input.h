#pragma once

#include <glm/glm.hpp>

#include "Hazel/KeyCodes.h"
#include "Hazel/MouseCodes.h"

namespace Hazel 
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}