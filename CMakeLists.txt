cmake_minimum_required(VERSION 3.24)
project(VulkanPractice LANGUAGES CXX)
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)


add_executable(vulkanPractice
               "src/main.cpp"
               "src/SimpleTriangle.h"
               "src/SimpleTriangle.cpp"
               "src/Utils.h"
               "src/Utils.cpp"
               "src/vmaUsage.cpp"
               "src/volkUsage.cpp")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(vulkanPractice PRIVATE /std:c++20)
endif()

add_subdirectory(third-parties/glm)
add_subdirectory(third-parties/SDL EXCLUDE_FROM_ALL)
add_subdirectory(third-parties/volk-1.4.350)
add_subdirectory(third-parties/VulkanMemoryAllocator-3.4.0)


target_link_libraries(vulkanPractice PRIVATE
                      Vulkan::Vulkan
                      SDL3::SDL3
                      glm::glm
                      volk::volk
                      Vulkan::shaderc_combined
                      GPUOpen::VulkanMemoryAllocator)


add_custom_command(TARGET vulkanPractice POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying shaders..."
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/shaders"
                   "$<TARGET_FILE_DIR:vulkanPractice>/shaders"
                   
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying SDL3.dll"
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "$<TARGET_FILE:SDL3::SDL3>"
                   "$<TARGET_FILE_DIR:vulkanPractice>"
                   )


# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vulkanPractice)