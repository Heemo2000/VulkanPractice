#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>

#include <volk.h>
#include <vk_mem_alloc.h>

#include<string>
#include<vector>

class SimpleTriangle
{
	constexpr static uint32_t VulkanVersion = VK_API_VERSION_1_4;
	constexpr static VkFormat ExpectedSwapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
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
	bool CreateLogicalDevice(VkPhysicalDevice physicalDevice);
	bool InitializeVMA();
	bool CreateSwapchain(uint32_t width, uint32_t height);
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
	VkDevice m_LogicalDevice = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	VmaAllocator m_Allocator = nullptr;
	
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;
	std::vector<VkSemaphore> m_RenderCompleteSemaphores;
	bool m_RequireSwapchainRecreate = false;
	uint32_t m_SwapchainWidth = 0;
	uint32_t m_SwapchainHeight = 0;
	VkFormat m_RequiredSwapchainFormat = VK_FORMAT_UNDEFINED;
};