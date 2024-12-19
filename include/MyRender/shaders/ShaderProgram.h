#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <string>
#include <optional>
#include <filesystem>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace myrender
{

class ShaderProgram 
{
public:
	enum class ProgramType
	{
		GRAPHICS_PIPELINE,
		COMPUTE_SHADER
	};

	ShaderProgram(const std::string& shaderName);
	~ShaderProgram();
	bool isValid() const { return mValid; }
	const std::string& getName() const  { return mProgramName; }
	ProgramType getType() const { return mProgramType; }
	unsigned int getId() const  { return mProgramId; }
	void use() const { glUseProgram(mProgramId); }
	
private:
	bool mValid = false;
	std::string mProgramName;
	ProgramType mProgramType;
	unsigned int mProgramId;

	std::optional<uint32_t> loadShader(const std::string& path, GLenum shaderType);
	GLenum shaderTypeFromStr(const std::string& extension);
	char* loadFromFile(std::string path, size_t* length);
	std::optional<std::filesystem::path> getModulePath(const std::string moduleName);
};

}

#endif // ISHADER_H