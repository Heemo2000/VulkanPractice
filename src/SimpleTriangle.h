#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>

#include <Volk/volk.h>

#include <vma/vk_mem_alloc.h>



#include<string>

class SimpleTriangle
{
public:
	SimpleTriangle() = delete;
	SimpleTriangle(const SimpleTriangle& instance) = delete;
	SimpleTriangle(const SimpleTriangle&& instance) = delete;
	SimpleTriangle(uint32_t width, uint32_t height);

public:
	bool Initialize();
	void Run();
	void Shutdown();

private:
	void ShowError(const std::string& errorMessage);
	void Render();
private:
	bool CreateWindow();
private:
	uint32_t m_Width;
	uint32_t m_Height;
	SDL_Window* m_Window;
	bool m_Running = false;
};