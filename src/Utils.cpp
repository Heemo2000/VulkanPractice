#include "Utils.h"
#include <fstream>
#include <sstream>

std::string ReadTextFile(const std::string& filePath)
{
	std::ifstream inputFileStream(filePath);
	if (inputFileStream.is_open())
	{
		std::stringstream stringStream;
		stringStream << inputFileStream.rdbuf();
		const std::string output = stringStream.str();
		inputFileStream.close();
		return output;
	}
	return std::string();
}

void ShowError(SDL_Window* window, const std::string& errorMessage)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), window);
}
