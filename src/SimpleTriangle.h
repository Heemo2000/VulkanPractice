#pragma once
#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>

#include <volk.h>
#include <vk_mem_alloc.h>
#include <shaderc/shaderc.hpp>

#include<string>
#include<vector>
#include<array>

struct FrameResources
{
	VkCommandPool CommandPool = VK_NULL_HANDLE;
	VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
	VkSemaphore ImageAcquiredSemaphore = VK_NULL_HANDLE;
};

class SimpleTriangle
{
	constexpr static uint32_t VulkanVersion = VK_API_VERSION_1_4;
	constexpr static VkFormat ExpectedSwapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
	constexpr static VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT;
	constexpr static uint32_t MaxFramesInFlight = 3;

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
	void Render();

private:
	void DestroySwapchain();
private:
	bool CreateWindow();
	bool CreateInstance();
	bool CreateSurface();
	VkPhysicalDevice FindPhysicalDevice();
	bool FindGraphicsQueueIndex();
	bool CreateLogicalDevice(VkPhysicalDevice physicalDevice);
	bool InitializeVMA();
	bool CreateSwapchain(uint32_t width, uint32_t height);
	bool CreateShaders();
	VkShaderModule CreateShaderModule(const std::string& fileName, shaderc_shader_kind kind) const;
	VkPipeline CreateGraphicsPipeline();
	bool CreateSyncResources();
	bool CreateCommandBuffers();

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

	VkImage m_DepthImage = VK_NULL_HANDLE;
	VmaAllocation m_DepthImageAllocation = nullptr;
	VkImageView m_DepthImageView = VK_NULL_HANDLE;

	VkShaderModule m_VertexShader = VK_NULL_HANDLE;
	VkShaderModule m_FragmentShader = VK_NULL_HANDLE;

	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;

	VkSemaphore m_TimelineSemaphore = VK_NULL_HANDLE;
	std::array<FrameResources, MaxFramesInFlight> m_FrameResources;

	uint32_t m_FrameIndex = 0;
	uint64_t m_NextSignalValue = MaxFramesInFlight + 1;

};