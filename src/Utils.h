#pragma once
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

std::string ReadTextFile(const std::string& filePath);

void ShowError(SDL_Window* window, const std::string& errorMessage);
