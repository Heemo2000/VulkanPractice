#include "SimpleTriangle.h"
#include <SDL3/SDL.h>
#include <iostream>

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

VkBool32 SimpleTriangle::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cout << "Validation: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
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

	std::vector<float> queuePriorities{ 1.0f };
	VkDeviceQueueCreateInfo graphicsQueueInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_GraphicsQueueIndex,
		.queueCount = static_cast<uint32_t>(queuePriorities.size()),
		.pQueuePriorities = queuePriorities.data()
	};



	return true;
}
