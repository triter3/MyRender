#ifndef SHADER_H
#define SHADER_H

#include <iostream>
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

	class Buffer
	{
		public:
			Buffer(uint32_t bufferType) : mBufferType(bufferType)
			{
				glGenBuffers(1, &mLocId);
			}

			~Buffer()
			{
				glDeleteBuffers(1, &mLocId);
			}
			uint32_t getId() { return mLocId; }
			uint32_t getType() { return mBufferType; }
			
			template<typename T>
			void setData(std::vector<T>& buffer);
			void resize(uint32_t sizeInBytes);
			template<typename T>
			void getData(std::vector<T>& buffer, size_t byteOffset);
			size_t getSize();
		private:
			uint32_t mLocId;
			uint32_t mBufferType;
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
	bool setBufferSize(const std::string& name, uint32_t sizeInBytes);
	template<typename T>
	bool getBufferData(const std::string& name, std::vector<T>& buffer, uint32_t startIndex);
	template<typename T>
	bool getBufferDataByteOffset(const std::string& name, std::vector<T>& buffer, size_t byteOffset);
	size_t getBufferSize(const std::string& name);
	bool setBuffer(const std::string& name, std::shared_ptr<Buffer> buffer);
	std::shared_ptr<Buffer> getBuffer(const std::string& name);
	
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
		uint32_t bufferType;
		uint32_t bindingIndex;
		std::vector<ShaderBufferVariable> variables;
		std::shared_ptr<Buffer> buffer;
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
		case GL_BOOL:
			glUniform1iv(it->second.location, it->second.numElements, reinterpret_cast<const GLint*>(&variable));
			break;
		case GL_UNSIGNED_INT:
			glUniform1uiv(it->second.location, it->second.numElements, reinterpret_cast<const GLuint*>(&variable));
			break;
		case GL_UNSIGNED_INT_VEC2:
			glUniform2uiv(it->second.location, it->second.numElements, reinterpret_cast<const GLuint*>(&variable));
			break;
		case GL_UNSIGNED_INT_VEC3:
			glUniform3uiv(it->second.location, it->second.numElements, reinterpret_cast<const GLuint*>(&variable));
			break;
		case GL_UNSIGNED_INT_VEC4:
			glUniform4uiv(it->second.location, it->second.numElements, reinterpret_cast<const GLuint*>(&variable));
			break;
		case GL_INT:
			glUniform1iv(it->second.location, it->second.numElements, reinterpret_cast<const GLint*>(&variable));
			break;
		case GL_INT_VEC2:
			glUniform2iv(it->second.location, it->second.numElements, reinterpret_cast<const GLint*>(&variable));
			break;
		case GL_INT_VEC3:
			glUniform3iv(it->second.location, it->second.numElements, reinterpret_cast<const GLint*>(&variable));
			break;
		case GL_INT_VEC4:
			glUniform4iv(it->second.location, it->second.numElements, reinterpret_cast<const GLint*>(&variable));
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

	if(it->second.buffer == nullptr) // Create buffer if not exists
	{
		it->second.buffer = std::make_shared<Buffer>(it->second.bufferType);
	}

	it->second.buffer->setData(array);
	const uint32_t ssboLoc = it->second.buffer->getId();
	glBindBufferBase(it->second.bufferType, it->second.bindingIndex, ssboLoc);
	glBindBuffer(it->second.bufferType, 0);
	return true;
}

template<typename T>
void Shader::Buffer::setData(std::vector<T>& array)
{
	const uint32_t ssboLoc = mLocId;
	glBindBuffer(mBufferType, ssboLoc);
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "Error Sub" << err << std::endl;
	}
	glBufferData(mBufferType, array.size() * sizeof(T), reinterpret_cast<const void*>(array.data()), GL_STATIC_DRAW);
}

template<typename T>
bool Shader::getBufferData(const std::string& name, std::vector<T>& buffer, uint32_t startIndex)
{
	return getBufferDataByteOffset(name, buffer, static_cast<size_t>(startIndex * sizeof(T)));
}

template<typename T>
bool Shader::getBufferDataByteOffset(const std::string& name, std::vector<T>& buffer, size_t byteOffset)
{
	auto it = mBuffersInfo.find(name);
	if(it == mBuffersInfo.end() || it->second.buffer == nullptr) return false;
	it->second.buffer->getData(buffer, byteOffset);
	return true;
}

template<typename T>
void Shader::Buffer::getData(std::vector<T>& buffer, size_t byteOffset)
{
	size_t buffSize = getSize();
	const uint32_t ssboLoc = mLocId;
	buffSize = std::min(buffSize, buffer.size() * sizeof(T));
	glBindBuffer(mBufferType, ssboLoc);
	glGetBufferSubData(mBufferType, byteOffset, buffSize, buffer.data());
}
}

#endif