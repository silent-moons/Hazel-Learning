#pragma once

#include "Hazel/Events/Event.h"

namespace Hazel
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			:m_Width(width), m_Height(height) {}

		inline unsigned int GetWidth() const { return m_Width; };
		inline unsigned int GetHeight() const { return m_Height; };

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryApplication)
			EVENT_CLASS_TYPE(WindowResize)
	private:
		unsigned int m_Width, m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_CATEGORY(EventCategoryApplication)
			EVENT_CLASS_TYPE(WindowClose)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		EVENT_CLASS_CATEGORY(EventCategoryApplication)
			EVENT_CLASS_TYPE(AppTick)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		EVENT_CLASS_CATEGORY(EventCategoryApplication)
			EVENT_CLASS_TYPE(AppUpdate)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_CATEGORY(EventCategoryApplication)
			EVENT_CLASS_TYPE(AppRender)
	};
}