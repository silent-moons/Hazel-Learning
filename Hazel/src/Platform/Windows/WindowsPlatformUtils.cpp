#include "hzpch.h"
#include "Hazel/Utils/PlatformUtils.h"

#include <commdlg.h> // windows api的标准对话框
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // glfw平台特定的功能

#include "Hazel/Core/Application.h"

namespace Hazel 
{
	std::string FileDialogs::OpenFile(const char* filter) // filter为定文件类型过滤器
	{
		OPENFILENAMEA ofn; // 存储文件对话框的各种信息
		CHAR szFile[260] = { 0 }; // 存储用户选择的文件路径
		ZeroMemory(&ofn, sizeof(OPENFILENAME)); // 将ofn结构体的内存清零
		ofn.lStructSize = sizeof(OPENFILENAME);

		// 获取当前窗口的句柄
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile; // 用户选择文件后可以将文件路径写入这个数组中
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter; // 设置ofn.lpstrFilter为传入的filter参数，以定义可见的文件类型
		ofn.nFilterIndex = 1;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = std::strchr(filter, '\0') + 1;

		// 对话框的标志:
		// OFN_PATHMUSTEXIST：用户选择的路径必须存在。
		// OFN_FILEMUSTEXIST：用户选择的文件必须存在。
		// OFN_NOCHANGEDIR：打开对话框时，不更改当前工作目录。
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; 
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
}