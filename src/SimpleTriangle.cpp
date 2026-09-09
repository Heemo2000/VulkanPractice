#include "SimpleTriangle.h"
#include <SDL3/SDL.h>


SimpleTriangle::SimpleTriangle(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
{
}

bool SimpleTriangle::Initialize()
{
	if (!CreateWindow())
	{
		ShowError("Failed to initialize a window!!");
		return false;
	}
	return true;
}

void SimpleTriangle::Run()
{
	m_Running = true;
	while (m_Running)
	{
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				m_Running = false;
				break;
			}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED)
			{
				m_Width = event.display.data1;
				m_Height = event.display.data2;
				break;
			}
		}

		Render();
	}
}

void SimpleTriangle::Shutdown()
{
}

void SimpleTriangle::ShowError(const std::string& errorMessage)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), m_Window);
}

void SimpleTriangle::Render()
{

}

bool SimpleTriangle::CreateWindow()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		ShowError("Failed to initialize SDL!!");
		return false;
	}

	m_Window = SDL_CreateWindow("Simple Triangle", m_Width, m_Height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
	if (m_Window == nullptr)
	{
		ShowError("Can't create a window!!");
		return false;
	}
	return true;
}
