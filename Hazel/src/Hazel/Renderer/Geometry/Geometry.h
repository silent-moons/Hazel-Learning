#pragma once

#include "Cube.h"
#include "Quad.h"
#include "Sphere.h"

struct BatchVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
	float TexIndex;
	float TilingFactor;
	int EntityID;
};