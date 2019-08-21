#include "hzpch.h"
#include "Application.h"

#include "Hazel/Log.h"

#include "Hazel/Renderer/Renderer.h"

#include "Input.h"

#include <GLFW/glfw3.h>

namespace Hazel {

	Application* Application::s_Instance = nullptr;
	std::string Application::s_AbsoluteDirectoryPath;

	Application::Application()
	{
		HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		s_AbsoluteDirectoryPath = CreateAbsoluteDirectoryPath();
		HZ_CORE_TRACE("Running application from '{0}'", GetApplicationDirectory().c_str());

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timestep);

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	std::string Application::CreateAbsoluteDirectoryPath()
	{
	#ifdef HZ_PLATFORM_WINDOWS
		// When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
		HMODULE hModule = GetModuleHandle(NULL);
		HZ_CORE_ASSERT(hModule, "Module handle is NULL");

		// Extract the path to this
		TCHAR ownPath[MAX_PATH];
		GetModuleFileName(hModule, ownPath, (sizeof(ownPath)));

		// Convert TCHAR to std::string
		#ifndef UNICODE
			std::string absOwnPath(ownPath);
		#else
			std::wstring absOwnPathW(ownPath);
			std::string absOwnPath(absOwnPathW.begin(), absOwnPathW.end());
		#endif

		// absOwnPath could contain \.. in its path, we remove it
		// example: "C:\subDir\subsubDir\..\Sandbox" => "C:\subDir\Sandbox"
		size_t foundIndex = absOwnPath.find("\\..");
		while (foundIndex < absOwnPath.length())
		{
			size_t prevIndex = absOwnPath.rfind("\\", foundIndex - 1);
			absOwnPath.erase(prevIndex, foundIndex - prevIndex + 3);

			// search again
			foundIndex = absOwnPath.find("\\..");
		}

		// absOwnPath contains for example "C:\Sandbox\Sandbox.exe"
		// Remove the Sandbox.exe part from the end
		size_t absOwnDirIndex = absOwnPath.rfind("\\");
		if (absOwnDirIndex < absOwnPath.length())
			return absOwnPath.substr(0, absOwnDirIndex + 1);
		return absOwnPath;

	#elif def HZ_PLATFORM_LINUX
		#error Linux abosulute directory path not implemented

	#else
		#error Unsupported platform!
	#endif
	}

	std::string Application::ResolvePath(const std::string& path)
	{
		// This will always return an absolute path
	#ifdef HZ_PLATFORM_WINDOWS
		std::filesystem::path fsPath(path);
		if (fsPath.is_absolute())
			return path;
		return GetApplicationDirectory() + path;

	#elif def HZ_PLATFORM_LINUX
		#error Linux resolve directory path not implemented

	#else
		#error Unsupported platform!
	#endif
	}

}
