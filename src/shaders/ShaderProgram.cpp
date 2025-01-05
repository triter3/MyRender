#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <filesystem>
#include <functional>
#include <map>
#include <regex>
#include "MyRender/shaders/ShaderProgram.h"
#include "MyRender/shaders/ShaderProgramLoader.h"

namespace myrender
{

ShaderProgram::ShaderProgram(const std::string& shaderName)
{
	const std::map<std::string, GLenum> extensionToShaderType = {
		{".vert", GL_VERTEX_SHADER},
		{".frag", GL_FRAGMENT_SHADER},
		{".comp", GL_COMPUTE_SHADER},
		{".geom", GL_GEOMETRY_SHADER},
		{".tesc", GL_TESS_CONTROL_SHADER},
		{".tese", GL_TESS_EVALUATION_SHADER}
	};

	//create the program
	mProgramId = glCreateProgram();
	std::optional<ProgramType> programType;
	std::vector<std::tuple<std::filesystem::path, GLenum, uint32_t>> inputShaders;
	std::function<void(const std::filesystem::path&)> searchShaders;
	searchShaders = [&](const std::filesystem::path& path)
	{
		if(!std::filesystem::is_directory(path)) return;
		auto pitr = std::filesystem::directory_iterator(path);
		for(std::filesystem::directory_entry entry : pitr)
		{
			if(entry.is_directory())
			{
				searchShaders(entry.path());
			}
			else if(entry.is_regular_file() && entry.path().filename().stem() == shaderName)
			{
				auto it = extensionToShaderType.find(entry.path().extension().string());
				if(it != extensionToShaderType.end())
				{
					const GLenum type = it->second;
					if(!programType)
					{
						programType = (type == GL_COMPUTE_SHADER) ? ProgramType::COMPUTE_SHADER : ProgramType::GRAPHICS_PIPELINE;
					}
					else if(programType.value() == ProgramType::COMPUTE_SHADER)
					{
						if(type == GL_COMPUTE_SHADER)
						{
							std::cout << "Warning: Name '" << shaderName << "' has more than one compute shader file" << std::endl;
						}
						continue;
					}
					else if(programType.value() == ProgramType::GRAPHICS_PIPELINE)
					{
						if(type == GL_COMPUTE_SHADER)
						{
							std::cout << "Warning: Name '" << shaderName << "' has graphics pipeline and compute shaders" << std::endl;
							continue;
						}

						if(std::any_of(inputShaders.begin(), inputShaders.end(), 
							[&] (const auto& t) { return std::get<1>(t) == type; } ))
						{
							std::cout << "Warning: Name '" << shaderName << "' has two files with the same extension '" << entry.path().extension() << "'" << std::endl;
							continue;
						}
					}

					std::optional<uint32_t> shaderId = loadShader(entry.path().string(), type);
					if(!shaderId)
					{
						std::cout << "Error: could not load the shader '" << entry.path().string() << "'" << std::endl;
						mValid = false;
						continue;
					}

					glAttachShader(mProgramId, *shaderId);
					inputShaders.push_back(std::make_tuple(entry.path(), type, *shaderId));
				}
			}
		}
	};

	for(const std::string& path : ShaderProgramLoader::getInstance()->getSearchPaths())
	{
		searchShaders(path);
	}

	if(inputShaders.size() == 0)
	{
		std::cout << "Error: shader with name '" << shaderName << "' not found" << std::endl;
		return;
	}

	glLinkProgram(mProgramId);

	for(auto iShader : inputShaders)
	{
		glDeleteShader(std::get<2>(iShader));
	}

	mValid = true;
}

std::optional<std::filesystem::path> ShaderProgram::getModulePath(const std::string moduleName)
{
	std::function<std::optional<std::filesystem::path>(const std::filesystem::path&)> searchModule;
	searchModule = [&](const std::filesystem::path& path) -> std::optional<std::filesystem::path>
	{
		auto pitr = std::filesystem::directory_iterator(path);
		for(std::filesystem::directory_entry entry : pitr)
		{
			if(entry.is_directory())
			{
				auto p = searchModule(entry.path());
				if(p) return p;
			}
			else if(entry.is_regular_file() && entry.path().filename().stem() == moduleName && entry.path().extension() == ".glsl")
			{
				return entry.path();
			}
		}

		return std::nullopt;
	};

	for(const std::string& path : ShaderProgramLoader::getInstance()->getSearchPaths())
	{
		auto p = searchModule(path);
		if(p) return p;
	}

	return std::nullopt;
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(mProgramId);
}

std::optional<uint32_t> ShaderProgram::loadShader(const std::string& shaderPath, GLenum shaderType)
{
	std::vector<char*> shaderFile;
	std::regex regex("(#include ([a-zA-z0-9]+)([ ]*)(\r?)\n)");
	std::match_results<const char*> m;

	std::function<bool(const std::string&)> loadFile;
	loadFile = [&](const std::string& path) -> bool
	{
		size_t length;
		char* shaderTxt = loadFromFile(path, &length);
		if(shaderTxt == nullptr)
		{
			std::cout << "Error: File " << path << " could not be found or opened" << std::endl;
			return false;
		}

		char* shaderPtr = shaderTxt;
		shaderFile.push_back(shaderPtr);
		const char* regexStartPoint = shaderTxt;
		bool res = true;
		while(res && std::regex_search(regexStartPoint, m, regex))
		{
			*(shaderTxt + static_cast<size_t>(m[0].first - shaderTxt)) = '\0';
			shaderPtr = shaderTxt + static_cast<size_t>(m[0].second - shaderTxt - 1);
			regexStartPoint = m[0].second;

			// Import Module
			std::string moduleName = m.str(2);
			std::optional<std::filesystem::path> modulePath = getModulePath(moduleName);
			if(!modulePath)
			{
				std::cout << "Module '" << moduleName <<  "' could not be found"  << std::endl;
				return false;
			}
			res = res && loadFile(modulePath.value().string());

			shaderFile.push_back(shaderPtr);
		}

		return res;
	};

	if(!loadFile(shaderPath))
	{
		return std::nullopt;
	}

	unsigned int shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, shaderFile.size(), shaderFile.data(), NULL);
	glCompileShader(shaderId);

	int success;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
		std::cout << "-> Shader error ( " << shaderPath << " ):" << std::endl;
		std::cout << infoLog << std::endl;
		return std::nullopt;
	}
	return shaderId;
}

char* ShaderProgram::loadFromFile(std::string path, size_t* length)
{
    std::ifstream file;
	file.open(path, std::ios_base::in | std::ios_base::binary);
	if (!file.good()) return nullptr;
	file.seekg(0, std::ios::end);
	*length = file.tellg();
	(*length)++;
	char* ret = new char[*length];
	file.seekg(0, std::ios::beg);
	file.read(ret, *length);
	file.close();
	ret[(*length) - 1] = 0;
	return ret;
}

}