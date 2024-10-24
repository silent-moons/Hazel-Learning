#pragma once

#include "Hazel/Core/Core.h"

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();

int main(int argc, char** argv)
{
	Hazel::Log::Init();
	HZ_CORE_WARN("Initialized Log!");

	auto app = Hazel::CreateApplication();
	app->Run();
	delete app;
	return 0;
}

#endif // HZ_PLATFORM_WINDOWS
