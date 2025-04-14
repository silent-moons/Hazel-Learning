#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/Application.h"
#include "Hazel/Core/Layer.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"

#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Math/Math.h"

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Scene/SceneSerializer.h"

//--------------- Renderer -------------------------
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/RenderStats.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/OrthoGraphicCamera.h"
#include "Hazel/Renderer/OrthoGraphicCameraController.h"

// Èë¿Úµã
//#include "Hazel/Core/EntryPoint.h"