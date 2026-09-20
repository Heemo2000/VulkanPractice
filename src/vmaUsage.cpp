#define VMA_IMPLEMENTATION

// 1. Tell VMA to use dynamic function pointers
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

// 2. Include the Volk header so VMA can see its definitions
#include <volk.h>

#include <vk_mem_alloc.h>