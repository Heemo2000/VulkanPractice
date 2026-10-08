#include "SimpleTriangle.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <format>
#include <fstream>
#include <sstream>
#include "Utils.h"

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

	if (!CreateInstance())
	{
		ShowError("Error while creating an instance!!");
		return false;
	}

	if (!CreateSurface())
	{
		ShowError("Failed to create a surface!!");
		return false;
	}

	if (m_PhysicalDevice = FindPhysicalDevice(); m_PhysicalDevice == VK_NULL_HANDLE)
	{
		ShowError("Failed to find physical device");
		return false;
	}

	VkPhysicalDeviceProperties deviceProp{};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProp);

	std::cout << "GPU we are using: " << deviceProp.deviceName << std::endl;

	if (!FindGraphicsQueueIndex())
	{
		ShowError("Failed to get graphics queue index!!");
		return false;
	}

	std::cout << "Graphics Queue Index: " << m_GraphicsQueueIndex << std::endl;

	if (!CreateLogicalDevice(m_PhysicalDevice))
	{
		ShowError("Couldn't create a logical device for selected GPU!!");
		return false;
	}

	if (!InitializeVMA())
	{
		ShowError("Failed to initialize VMA!!");
		return false;
	}

	if (!CreateSwapchain(m_Width, m_Height))
	{
		ShowError("Couldn't create a swapchain!!");
		return false;
	}

	if (!CreateShaders())
	{
		ShowError("Failed to create shaders!!");
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
	vkDeviceWaitIdle(m_LogicalDevice);

	DestroySwapchain();

	if (m_Allocator != nullptr)
	{
		vmaDestroyAllocator(m_Allocator);
	}

	if (m_LogicalDevice != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	if (m_Surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	}

	if (m_Instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_Instance, nullptr);
	}

	volkFinalize();
}

VkBool32 SimpleTriangle::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cout << "Validation: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}

void SimpleTriangle::ShowError(const std::string& errorMessage) const
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), m_Window);
}

void SimpleTriangle::Render()
{

}

void SimpleTriangle::DestroySwapchain()
{
	for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
	{
		vkDestroyImageView(m_LogicalDevice, m_SwapchainImageViews[i], nullptr);
	}

	m_SwapchainImageViews.clear();

	for (size_t i = 0; i < m_RenderCompleteSemaphores.size(); i++)
	{
		vkDestroySemaphore(m_LogicalDevice, m_RenderCompleteSemaphores[i], nullptr);
	}


	if (m_Swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);
		m_Swapchain = nullptr;
	}


	if (m_DepthImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(m_LogicalDevice, m_DepthImageView, nullptr);
		vmaDestroyImage(m_Allocator, m_DepthImage, m_DepthImageAllocation);
		m_DepthImageView = nullptr;
	}
}

bool SimpleTriangle::CreateWindow()
{
	//First, we initialize SDL with video support.
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		ShowError("Failed to initialize SDL!!");
		return false;
	}

	//Then, create a window with Vulkan support.
	m_Window = SDL_CreateWindow("Simple Triangle", m_Width, m_Height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
	if (m_Window == nullptr)
	{
		ShowError("Can't create a window!!");
		return false;
	}
	return true;
}

bool SimpleTriangle::CreateInstance()
{
	//First initialize the Volk Library.
	if (volkInitialize() != VK_SUCCESS)
	{
		ShowError("Failed to initialize Vulkan!!");
		return false;
	}
	
	//Setup app info for making an instance
	VkApplicationInfo appInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Simple Triangle",
		.applicationVersion = VK_MAKE_API_VERSION(0,0,1,0),
		.engineVersion = VK_MAKE_API_VERSION(0,0,1,0),
		.apiVersion = VulkanVersion
	};

	//This is Debug Messenger Create Info, this is helpful in setting up a debugger which
	//we can use for checking various types of warnings and errors.

	const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,

		.pfnUserCallback = DebugCallback,

		.pUserData = this
	};


	//First, grab instance extensions from SDL, which are actually features that 
	//extend the global functionality of Vulkan API.
	//Because Vulkan is entirely platform-agnostic, the
	//core API doesn't know how to talk to your OS's window manager
	//or how to output debug messages
	//Instance extensions bridge this gap.

	uint32_t instanceExtensionsCount = 0;
	const char* const* instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount);

	std::cout << "Instance Extensions: " << std::endl;

	for (int i = 0; i < instanceExtensionsCount; i++)
	{
		std::cout << instanceExtensions[i] << std::endl;
	}

	std::cout << std::endl;
	std::vector<const char*> extensions(instanceExtensions, instanceExtensions + instanceExtensionsCount);
	
	//As I have talked before, Vulkan is platform-agnostic, it doesn't know about how to output messages.
	//So, we will push one identifier, called VK_EXT_DEBUG_UTILS_EXTENSION_NAME.
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	//Now, we will request some layers, in which we request validation layers.
	//Validation layers are responsible for checking the warnings and errors.

	//Note that, validation layers and debug messenger are different.
	//Vulkan keeps them separate for modularity, performance and flexibility.
	std::vector<const char*> requestedLayers
	{
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstanceCreateInfo instanceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &debugMessengerCreateInfo,
		.pApplicationInfo = &appInfo,
		
		.enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
		.ppEnabledLayerNames = requestedLayers.data(),

		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance) != VK_SUCCESS)
	{
		ShowError("Failed to create an instance!!");
		return false;
	}

	//This call loads all required global and instance-level functions for Vulkan instance.
	//If we don't call this, then any subsequent call to Vulkan would result in failure.
	volkLoadInstance(m_Instance);

	return true;
}

bool SimpleTriangle::CreateSurface()
{
	if (!SDL_Vulkan_CreateSurface(m_Window, m_Instance, nullptr, &m_Surface))
	{
		return false;
	}
	return true;
}

VkPhysicalDevice SimpleTriangle::FindPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, VK_NULL_HANDLE);

	if (physicalDeviceCount == 0)
	{
		return VK_NULL_HANDLE;
	}

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

	vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.data());

	VkPhysicalDevice finalDeviceToUse = physicalDevices[0];

	for (VkPhysicalDevice& device : physicalDevices)
	{
		VkPhysicalDeviceProperties deviceProp{};
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			finalDeviceToUse = device;
			break;
		}
	}

	for (VkPhysicalDevice& device : physicalDevices)
	{
		VkPhysicalDeviceProperties deviceProp{};
		vkGetPhysicalDeviceProperties(device, &deviceProp);
		
		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			finalDeviceToUse = device;
			break;
		}
	}


	//Search for compatible surface format.
	uint32_t surfaceFormatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(finalDeviceToUse, m_Surface, &surfaceFormatsCount, VK_NULL_HANDLE);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(finalDeviceToUse, m_Surface, &surfaceFormatsCount, surfaceFormats.data());
	
	//If we get the expected swapchain format, then it's good,
	//otherwise, we will just set required swapchain format to the first one supported format.
	for (const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format == ExpectedSwapchainFormat)
		{
			m_RequiredSwapchainFormat = ExpectedSwapchainFormat;
			break;
		}
	}

	if (m_RequiredSwapchainFormat == VK_FORMAT_UNDEFINED)
	{
		m_RequiredSwapchainFormat = surfaceFormats[0].format;
	}

	return finalDeviceToUse;
}

bool SimpleTriangle::FindGraphicsQueueIndex()
{
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(m_PhysicalDevice, &queueFamilyPropertyCount, VK_NULL_HANDLE);
	
	std::vector<VkQueueFamilyProperties2> queueFamilyProperties(queueFamilyPropertyCount, {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});

	vkGetPhysicalDeviceQueueFamilyProperties2(m_PhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

	int index = 0;
	for (const VkQueueFamilyProperties2& prop : queueFamilyProperties)
	{
		VkBool32 hasPresentationSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, index, m_Surface, &hasPresentationSupport);
		if (hasPresentationSupport && (prop.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			m_GraphicsQueueIndex = index;
			return true;
		}
		index++;
	}


	m_GraphicsQueueIndex = -1;
	return false;
}

bool SimpleTriangle::CreateLogicalDevice(VkPhysicalDevice physicalDevice)
{
	//Here, we will ask for what features will the current GPU can support.

	VkPhysicalDeviceVulkan14Features vk14Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
		.pNext = nullptr
	};

	VkPhysicalDeviceVulkan13Features vk13Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vk14Features
	};

	VkPhysicalDeviceVulkan12Features vk12Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &vk13Features
	};

	VkPhysicalDeviceFeatures2 vk11Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &vk12Features
	};

	vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &vk11Features);

	if (!vk13Features.dynamicRendering || !vk13Features.synchronization2 || !vk12Features.timelineSemaphore)
	{
		if (!vk13Features.dynamicRendering)
		{
			ShowError("Your GPU doesn't support dynamic rendering");
		}

		if (!vk13Features.synchronization2)
		{
			ShowError("Your GPU doesn't support Synchronization2");
		}

		if (!vk12Features.timelineSemaphore)
		{
			ShowError("Your GPU doesn't support Timeline Semaphores");
		}

		return false;
	}

	//Here, we will ask for what features will the current GPU will support for the application.
	//Remember, request only needed features, not more than that, otherwise, your GPU may explode. Who knows ?
	VkPhysicalDeviceVulkan14Features vk14FeaturesToSupport
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
		.pNext = nullptr
	};

	VkPhysicalDeviceVulkan13Features vk13FeaturesToSupport
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vk14FeaturesToSupport,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE
	};

	VkPhysicalDeviceVulkan12Features vk12FeaturesToSupport
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &vk13FeaturesToSupport,
		.timelineSemaphore = VK_TRUE
	};

	VkPhysicalDeviceFeatures2 vk11FeaturesToSupport
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &vk12FeaturesToSupport
	};

	//Request the queues we will be using.
	std::vector<float> queuePriorities{ 1.0f };
	VkDeviceQueueCreateInfo graphicsQueueInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_GraphicsQueueIndex,
		.queueCount = 1,
		.pQueuePriorities = queuePriorities.data()
	};

	//Get the device extensions.
	std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	//Now, we will create logical device based on create info.
	const VkDeviceCreateInfo deviceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &vk11FeaturesToSupport, //We have set 'features to support' here, since the type is VkPhysicalDeviceFeatures2 and also we provide a chain of features, 
										 //not just a single feature to support.
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &graphicsQueueInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = nullptr
	};


	if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
	{
		return false;
	}

	vkGetDeviceQueue(m_LogicalDevice, m_GraphicsQueueIndex, 0, &m_GraphicsQueue);

	if (m_GraphicsQueue == VK_NULL_HANDLE)
	{
		ShowError("Couldn't get the graphics queue");
		return false;
	}

	return true;
}

bool SimpleTriangle::InitializeVMA()
{
	VmaVulkanFunctions funcInfo{};

	const VmaAllocatorCreateInfo allocatorCreateInfo
	{
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, // It simply means that we can access VRAM directly via pointers.
		.physicalDevice = m_PhysicalDevice,
		.device = m_LogicalDevice,
		.pVulkanFunctions = &funcInfo,
		.instance = m_Instance,
		.vulkanApiVersion = VulkanVersion
	};

	vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &funcInfo);
	
	if (vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator) != VK_SUCCESS)
	{
		ShowError("Could not create VMA allocator!!");
		return false;
	}
	return true;
}

bool SimpleTriangle::CreateSwapchain(uint32_t width, uint32_t height)
{
	m_SwapchainWidth = width;
	m_SwapchainHeight = height;

	//We request the number of images using surface capabilities.
	VkSurfaceCapabilitiesKHR surfaceCaps{};
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &surfaceCaps) != VK_SUCCESS)
	{
		ShowError("Couldn't get surface capabilities");
		return false;
	}

	uint32_t requestedImageCount = std::max(2u, surfaceCaps.minImageCount);
	//If maxImageCount is 0, then we can ask for unlimited number of images.
	//But if it's value is other than 0, then clamp to minimum of requestedImageCount and maxImageCount.
	if (surfaceCaps.maxImageCount > 0)
	{
		requestedImageCount = std::min(requestedImageCount, surfaceCaps.maxImageCount);
	}

	//Now, create the swapchain.
	VkSwapchainCreateInfoKHR swapchainCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_Surface,
		.minImageCount = requestedImageCount,
		.imageFormat = m_RequiredSwapchainFormat,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent { .width = m_SwapchainWidth, .height = m_SwapchainHeight },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = surfaceCaps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};

	if (vkCreateSwapchainKHR(m_LogicalDevice, &swapchainCreateInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
	{
		return false;
	}

	//Now, get the swapchain images.
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, VK_NULL_HANDLE);

	m_SwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, m_SwapchainImages.data());

	m_SwapchainImageViews.resize(imageCount);

	//Now, create swapchain image views.

	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_SwapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_RequiredSwapchainFormat,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		if (vkCreateImageView(m_LogicalDevice, &imageViewCreateInfo, nullptr, &m_SwapchainImageViews[i]) != VK_SUCCESS)
		{
			ShowError(std::format("Not able to create image view {0}", i));
			return false;
		}
	}

	//Now, create the render complete semaphores which are signals when the GPU has finished rendering and gives the presentation engine permission to show the image on the screen.
	m_RenderCompleteSemaphores.resize(m_SwapchainImages.size());

	int index = 0;
	for (VkSemaphore& semaphore : m_RenderCompleteSemaphores)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		if (vkCreateSemaphore(m_LogicalDevice, &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS)
		{
			ShowError(std::format("Not able to create render complete semaphore {0}", index));
			return false;
		}
		index++;
	}


	//Now, we will create a depth/Z-buffer which will decide which object is near or far to the screen,
	//and will discard pixels for the object when that object is behind another object.
	VkImageCreateInfo depthImageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = DepthFormat,
		.extent { .width = m_SwapchainWidth, .height = m_SwapchainHeight, .depth = 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,//This tells Vulkan to choose tiling optimal for the image.
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo allocInfo
	{
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,//This tells VMA that we want memory dedicated specifically for the depth buffer and not the piece of larger allocation.
		.usage = VMA_MEMORY_USAGE_AUTO //This tell VMA to select best memory type for the depth buffer automatically.
	};

	if (vmaCreateImage(m_Allocator, &depthImageCreateInfo, &allocInfo, &m_DepthImage, &m_DepthImageAllocation, nullptr) != VK_SUCCESS)
	{
		ShowError("Unable to create depth image!!");
		return false;
	}

	//Here, just setting up depth buffer/image is not enough, we need to create image view for this.
	VkImageViewCreateInfo depthImageViewCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = m_DepthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = DepthFormat,
		.subresourceRange { .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
	};

	if (vkCreateImageView(m_LogicalDevice, &depthImageViewCreateInfo, nullptr, &m_DepthImageView) != VK_SUCCESS)
	{
		ShowError("Unable to create depth image view!!");
		return false;
	}

	return true;
}

bool SimpleTriangle::CreateShaders()
{
	if (m_VertexShader = CreateShaderModule("shader.vert", shaderc_vertex_shader); m_VertexShader == VK_NULL_HANDLE)
	{
		ShowError("Vertex shader does not get created!!");
		return false;
	}

	if (m_FragmentShader = CreateShaderModule("shader.frag", shaderc_fragment_shader); m_FragmentShader == VK_NULL_HANDLE)
	{
		ShowError("Fragment shader does not get created!!");
		return false;
	}
	

	return true;
}

VkShaderModule SimpleTriangle::CreateShaderModule(const std::string& fileName, shaderc_shader_kind kind) const
{
	const std::string realPath = std::format("shaders/{0}", fileName);
	const std::string src = ReadTextFile(realPath);
	if (src.empty())
	{
		ShowError(std::format("Specified shader not found at file path {0} !", realPath));
		return VK_NULL_HANDLE;
	}

	std::cout << "Compiling shader " << realPath << std::endl;
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
	options.SetTargetSpirv(shaderc_spirv_version_1_6);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	shaderc::CompilationResult result = compiler.CompileGlslToSpv(src, kind, fileName.c_str(), options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cerr << "Shader compilation error: " << result.GetErrorMessage() << std::endl;
		return VK_NULL_HANDLE;
	}

	const size_t shaderSize = (result.cend() - result.cbegin()) * sizeof(uint32_t);

	VkShaderModuleCreateInfo shaderCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderSize,
		.pCode = result.cbegin()
	};

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	if (vkCreateShaderModule(m_LogicalDevice, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		ShowError(std::format("Creation of shader module {0} failed!!", fileName));
		return VK_NULL_HANDLE;
	}

	std::cout << "Compilation successful." << std::endl;
	return shaderModule;
}


