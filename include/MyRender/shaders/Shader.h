#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <map>
#include "MyRender/shaders/ShaderProgram.h"
#include "MyRender/Camera.h"

namespace myrender
{
	
class Texture
{

};

enum class TextureType
{
	IMAGE2D,
	IMAGE3D,
	CUBEMAP
};

class Shader
{
public:
    struct ShaderType
    {
        uint32_t size; // in bytes
		GLenum type;
    };

	static Shader loadShader(const std::string& shaderName);

	bool isValid() const { return mValid; }
    
    bool load(const std::string& shaderName);

	template<typename T>
	bool setUniform(const std::string& name, T&& variable);
	template<typename T>
	bool setUniform(const std::string& name, T& variable);
	bool linkUniform(const std::string& name, void* ptr);
	bool hasUniform(const std::string& name) const { return mUniformsInfo.find(name) != mUniformsInfo.end(); }

	template<typename T>
	bool setBufferData(const std::string& name, std::vector<T>& buffer);
	
	void bind(Camera* camera, glm::mat4x4* modelMatrix);

	const ShaderProgram& getProgram() const { return *mProgram; }
private:
    bool mValid = false;
    std::shared_ptr<ShaderProgram> mProgram;
    struct UniformInfo
    {
        uint32_t location;
        ShaderType type;
        uint32_t numElements;
        void* handle;
    };

	struct TextureInfo
	{
		TextureType type;
		std::shared_ptr<Texture> texture;
	};

	struct ShaderBufferVariable
	{
		uint32_t offset;
		ShaderType type;
		uint32_t numElements;
		uint32_t arrayStride;
	};

	struct ShaderBuffer
	{
		uint32_t bindingIndex;
		std::vector<ShaderBufferVariable> variables;
		std::optional<uint32_t> bufferLocation;
	};

    std::map<std::string, UniformInfo> mUniformsInfo;
	std::map<std::string, TextureInfo> mSamplersInfo;
	std::map<std::string, TextureInfo> mImagesInfo;
	std::map<std::string, ShaderBuffer> mBuffersInfo;
};

template<typename T>
bool Shader::setUniform(const std::string& name, T&& variable)
{
	T var = variable;
	return setUniform(name, var);
}

template<typename T>
bool Shader::setUniform(const std::string& name, T& variable)
{
	mProgram->use();
	auto it = mUniformsInfo.find(name);
	if(it == mUniformsInfo.end()) return false;
	uint32_t size = it->second.numElements * it->second.type.size;
	if(sizeof(T) != size) 
	{
		std::cout << "Uniform '" << name << "' was expecting a variable of size " << size << std::endl;
		return false;
	}
	switch(it->second.type.type)
	{
		case GL_FLOAT:
			glUniform1fv(it->second.location, it->second.numElements, reinterpret_cast<const GLfloat*>(&variable));
			break;
		case GL_FLOAT_VEC2:
			glUniform2fv(it->second.location, it->second.numElements, reinterpret_cast<const GLfloat*>(&variable));
			break;
		case GL_FLOAT_VEC3:
			glUniform3fv(it->second.location, it->second.numElements, reinterpret_cast<const GLfloat*>(&variable));
			break;
		case GL_FLOAT_VEC4:
			glUniform4fv(it->second.location, it->second.numElements, reinterpret_cast<const GLfloat*>(&variable));
			break;
		case GL_FLOAT_MAT3:
			glUniformMatrix3fv(it->second.location, it->second.numElements, GL_FALSE, reinterpret_cast<const GLfloat*>(&variable));
			break;
		case GL_FLOAT_MAT4:
			glUniformMatrix4fv(it->second.location, it->second.numElements, GL_FALSE, reinterpret_cast<const GLfloat*>(&variable));
			break;
		default:
			std::cout << "Error: Uniform type not supported." << std::endl;
			break;
	}

	return true;
}

template<typename T>
bool Shader::setBufferData(const std::string& name, std::vector<T>& array)
{
	mProgram->use();
	auto it = mBuffersInfo.find(name);
	if(it == mBuffersInfo.end()) return false;

	it->second.bufferLocation = std::optional<uint32_t>(0);
	uint32_t& ssboLoc = it->second.bufferLocation.value();

	glGenBuffers(1, &ssboLoc);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLoc);
	glBufferData(GL_SHADER_STORAGE_BUFFER, array.size() * sizeof(T), reinterpret_cast<const void*>(array.data()), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, it->second.bindingIndex, ssboLoc);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	std::cout << it->second.bindingIndex << std::endl;
	std::cout << "Done" << std::endl;
	return true;
}
}

#endif