#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>

#include <Volk/volk.h>

#include <vma/vk_mem_alloc.h>
#include<string>
#include<vector>

class SimpleTriangle
{
	constexpr static uint32_t VulkanVersion = VK_API_VERSION_1_4;
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
	static VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
								  VkDebugUtilsMessageTypeFlagsEXT messageType,
								  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
								  void* pUserData);

private:
	void ShowError(const std::string& errorMessage);
	void Render();
private:
	bool CreateWindow();
	bool CreateInstance();
	bool CreateSurface();
	VkPhysicalDevice FindPhysicalDevice();
	bool FindGraphicsQueueIndex();
private:
	uint32_t m_Width;
	uint32_t m_Height;
	SDL_Window* m_Window;
	bool m_Running = false;

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	uint32_t m_GraphicsQueueIndex = -1;
};