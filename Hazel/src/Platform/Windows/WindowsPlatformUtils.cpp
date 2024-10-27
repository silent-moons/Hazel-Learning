#include "hzpch.h"
#include "Hazel/Utils/PlatformUtils.h"

#include <commdlg.h> // windows api�ı�׼�Ի���
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // glfwƽ̨�ض��Ĺ���

#include "Hazel/Core/Application.h"

namespace Hazel 
{
	std::string FileDialogs::OpenFile(const char* filter) // filterΪ���ļ����͹�����
	{
		OPENFILENAMEA ofn; // �洢�ļ��Ի���ĸ�����Ϣ
		CHAR szFile[260] = { 0 }; // �洢�û�ѡ����ļ�·��
		ZeroMemory(&ofn, sizeof(OPENFILENAME)); // ��ofn�ṹ����ڴ�����
		ofn.lStructSize = sizeof(OPENFILENAME);

		// ��ȡ��ǰ���ڵľ��
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile; // �û�ѡ���ļ�����Խ��ļ�·��д�����������
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter; // ����ofn.lpstrFilterΪ�����filter�������Զ���ɼ����ļ�����
		ofn.nFilterIndex = 1;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = std::strchr(filter, '\0') + 1;

		// �Ի���ı�־:
		// OFN_PATHMUSTEXIST���û�ѡ���·��������ڡ�
		// OFN_FILEMUSTEXIST���û�ѡ����ļ�������ڡ�
		// OFN_NOCHANGEDIR���򿪶Ի���ʱ�������ĵ�ǰ����Ŀ¼��
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