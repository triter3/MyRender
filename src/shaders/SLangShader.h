/*
#ifndef SHADER_H
#define SHADER_H

#include <array>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <map>
#include "IShader.h"
#include "slang.h"
#include "slang-com-ptr.h"

namespace MyRender
{

const std::string SHADER_PATH = "./shaders/";

template <class T>
class AShader : public IShader 
{
public:
	Shader(const std::string& vertexShaderName, const std::string& fragmentShaderName) : IShader(vertexShaderName, fragmentShaderName) {}
	static T* getInstance() {
		if (instance == nullptr) instance = new T();
		return instance;
	}

private:
	static T* instance;
};

template <class T>
T* AShader<T>::instance = nullptr;

class ShaderManager
{
public:
	static slang::IGlobalSession* getSlangGlobalSession()
	{
		if(slangGlobalSession.get() == nullptr)
		{
			slang::createGlobalSession(slangGlobalSession.writeRef());
		}
		return slangGlobalSession.get();
	}

	static ShaderManager& getInstance()
	{
		if(instance == nullptr)
		{
			instance = std::make_unique<ShaderManager>();
		}
		return *instance;
	}

	slang::ISession* getmSlangSession()
	{
		if(mSlangSession.get() == nullptr)
		{
			// Define session
			slang::SessionDesc sessionDesc;
			slang::TargetDesc targetDesc;
			targetDesc.format = SLANG_GLSL;
			targetDesc.profile = getSlangGlobalSession()->findProfile("glsl_450");
			if(targetDesc.profile == SLANG_PROFILE_UNKNOWN)
			{
				std::cout << "Error slang profile not avaliable" << std::endl;
			}
			sessionDesc.targets = &targetDesc;
			sessionDesc.targetCount = 1;
			// Set shader path using __FILE__ definition
			std::array<const char*, 1> searchPaths = {"shaders/"};
			sessionDesc.searchPaths = searchPaths.data();
			sessionDesc.searchPathCount = 1;

			getSlangGlobalSession()->createSession(sessionDesc, mSlangSession.writeRef());
		}
		return mSlangSession.get();
	}

	template<typename S>
	bool getShaderProgram(const std::string& moduleName, const std::string& entryPointName, S& outShader)
	{
		ShaderManager& sManager = ShaderManager::getInstance();

		// Load module
		Slang::ComPtr<slang::IBlob> diagnostics;
		Slang::ComPtr<slang::IModule> myModule(sManager.getmSlangSession()->loadModule(moduleName.c_str(), diagnostics.writeRef()));

		if(diagnostics)
    	{
			std::cout << "Error loading slang module '" << moduleName << "'" << std::endl;
			std::cout << reinterpret_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
			return false;
		}

		// Find entry point
		Slang::ComPtr<slang::IEntryPoint> myVertex;
		myModule->findEntryPointByName(vertexEntryPoint.c_str(), myVertex.writeRef());

		// Link component
		Slang::ComPtr<slang::IComponentType> myVertexLinked;
		myVertex->link(myVertexLinked.writeRef(), diagnostics.writeRef());

		if(diagnostics)
		{
			std::cout << "Error loading slang entry point '" << vertexEntryPoint << "'" << std::endl;
			std::cout << reinterpret_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
			return false;
		}

		return outShader.createShader(myVertexLinked);
	}

	template<typename S>
	std::unique_ptr<S> getShaderProgram(const std::string& moduleName, const std::string& entryPointName)
	{
		std::unique_ptr<S> obj = std::make_unique<S>();
		bool res = getShader(moduleName, entryPointName, *obj);
		return (res) ? obj : nullptr;
	}

private:
	static Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;
	static std::unique_ptr<ShaderManager> instance;

	Slang::ComPtr<slang::ISession> mSlangSession;
	std::map<std::tuple<std::string, std::string>, uint32_t> mShaderToProgramId;
};

class ShaderProgram
{
public:
	bool createProgram(slang::IComponentType* component)
	{
		const uint32_t maxNumShaders = 5;
		std::array<Shader, maxNumShaders> shaders;
		slang::ProgramLayout* layout = component->getLayout();
		const uint32_t numEntryPoints = glm::max(static_cast<uint32_t>(layout->getEntryPointCount()), maxNumShaders);
		bool foundComputeShader = false;
		bool foundVertexShader = false;
		bool foundFragmentShader = false;
		bool res = true;
		for(uint32_t i=0; i < numEntryPoints; i++)
		{
			res = res && shaders[i].createShader(component, i);
			foundComputeShader = shaders[i].getShaderType() == GL_COMPUTE_SHADER;
			foundVertexShader = shaders[i].getShaderType() == GL_COMPUTE_SHADER;
			foundFragmentShader = shaders[i].getShaderType() == GL_COMPUTE_SHADER;
		}

		if(!res)
		{
			std::cout << "Error creating the program" << std::endl;
			return false;
		}

		// Read parameters
		for(uint32_t i=0; i < layout->getParameterCount(); i++)
		{
			slang::VariableLayoutReflection* variable = layout->getParameterByIndex(i);
			switch(variable->getCategory())
			{
				case slang::ParameterCategory::Uniform:
					break;
				case slang::ParameterCategory::ShaderResource:
					break;
			}
			std::cout << variable->getName() << std::endl;
			if(variable->getCategory() == slang::ParameterCategory::Uniform)
			{
				std::cout << "uniform " << variable->getBindingIndex() << " " << variable->getBindingSpace() << " " << variable->getOffset() << std::endl;
			}
			else if(variable->getCategory() == slang::ParameterCategory::VaryingInput)
			{
				std::cout << "varying input" << std::endl;
			}
			else
			{
				std::cout << "other" << std::endl;
			}

			slang::TypeLayoutReflection* typeLayout = variable->getTypeLayout();
			std::cout << typeLayout->getSize() << std::endl;

			std::cout << std::endl;

		}


		return true;
	}
private:
	bool mCreated = false;
};

class Shader
{
public:
	bool createShader(slang::IComponentType* component, uint32_t entryPointIndex)
	{
		// Get code
		Slang::ComPtr<slang::IBlob> myVertexBlob;
		Slang::ComPtr<slang::IBlob> diagnostics;
		component->getEntryPointCode(entryPointIndex, 0, myVertexBlob.writeRef(), diagnostics.writeRef());

		if(diagnostics)
		{
			std::cout << "Error generating code for entry point '" << component->getLayout()->getEntryPointByIndex(entryPointIndex)->getName() << "'" << std::endl;
			std::cout << reinterpret_cast<const char*>(diagnostics->getBufferPointer()) << std::endl;
			return false;
		}

		switch(component->getLayout()->getEntryPointByIndex(0)->getStage())
		{
			case SlangStage::SLANG_STAGE_COMPUTE:
				mShaderType = GL_COMPUTE_SHADER;
				break;
			case SlangStage::SLANG_STAGE_VERTEX:
				mShaderType = GL_VERTEX_SHADER;
				break;
			case SlangStage::SLANG_STAGE_FRAGMENT:
				mShaderType = GL_FRAGMENT_SHADER;
				break;
			default:
				std::cout << "Unknown shder type" << std::endl;
				return false;
		}

		mShaderId = glCreateShader(mShaderType);

		const char* codePtr = reinterpret_cast<const char*>(myVertexBlob->getBufferPointer());
		glShaderSource(mShaderId, 1, &codePtr, NULL);
		glCompileShader(mShaderId);

		int success;
		glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(mShaderId, 512, NULL, infoLog);
			std::cout << "-> Vertex Shader error" << std::endl;
			std::cout << infoLog << std::endl;
			return false;
		}

		mCreated = true;
		return true;
	}

	GLenum getShaderType() const { return mShaderType; }
	GLuint getShaderId() const { return mShaderId; }
private:
	bool mCreated = false;
	GLuint mShaderId;
	GLenum mShaderType;
};

}

#endif
*/