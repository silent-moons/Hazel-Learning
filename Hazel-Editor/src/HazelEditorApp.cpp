#include "Hazel.h"
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel 
{
	class HazelEditor : public Hazel::Application
	{
	public:
		HazelEditor() : Application("Hazel Editor")
		{
			PushLayer(new EditorLayer());
		}
		~HazelEditor() {}

	private:

	};

	Hazel::Application* Hazel::CreateApplication()
	{
		return new HazelEditor();
	}
}