cmake_minimum_required(VERSION 3.24)
project(VulkanPractice LANGUAGES CXX)
find_package(Vulkan REQUIRED)


add_executable(vulkanPractice
               "src/main.cpp"
               "src/SimpleTriangle.h"
               "src/SimpleTriangle.cpp"
               "src/vmaUsage.cpp"
               "src/volkUsage.cpp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third-parties/glm)
add_subdirectory(third-parties/SDL EXCLUDE_FROM_ALL)
add_subdirectory(third-parties/volk-1.4.350)
add_subdirectory(third-parties/VulkanMemoryAllocator-3.4.0)

set(SLANG_ROOT "${CMAKE_SOURCE_DIR}/third-parties/slang")

add_library(slang SHARED IMPORTED)

set_target_properties(slang PROPERTIES
                     IMPORTED_LOCATION "${SLANG_ROOT}/bin/slang.dll"
                     IMPORTED_IMPLIB "${SLANG_ROOT}/lib/slang.lib"
                     INTERFACE_INCLUDE_DIRECTORIES "${SLANG_ROOT}/include")


target_link_libraries(vulkanPractice PRIVATE
                      Vulkan::Vulkan
                      SDL3::SDL3
                      glm::glm
                      volk::volk
                      slang
                      GPUOpen::VulkanMemoryAllocator)


add_custom_command(TARGET vulkanPractice POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying shaders..."
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/shaders"
                   "$<TARGET_FILE_DIR:vulkanPractice>/shaders"
                   
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying slang.dll"
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${SLANG_ROOT}/bin/slang.dll"
                   "${SLANG_ROOT}/bin/slang-compiler.dll"
                   "$<TARGET_FILE_DIR:vulkanPractice>"

                   COMMAND ${CMAKE_COMMAND} -E echo "Copying SDL3.dll"
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "$<TARGET_FILE:SDL3::SDL3>"
                   "$<TARGET_FILE_DIR:vulkanPractice>"
                   )


# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vulkanPractice)