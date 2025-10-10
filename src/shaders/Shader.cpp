#include "MyRender/shaders/Shader.h"
#include <iostream>
#include <map>
#include <glm/gtc/matrix_inverse.hpp>
#include "MyRender/shaders/ShaderProgramLoader.h"
#include "MyRender/Camera.h"

namespace myrender
{

namespace internal
{
    const std::map<uint32_t, Shader::ShaderType> uniformTypes = {
        {GL_FLOAT, {1*4, GL_FLOAT}},
        {GL_FLOAT_VEC2, {2*4, GL_FLOAT_VEC2}},
        {GL_FLOAT_VEC3, {3*4, GL_FLOAT_VEC3}},
        {GL_FLOAT_VEC4, {4*4, GL_FLOAT_VEC4}},
        {GL_DOUBLE, {8, GL_DOUBLE}},
        {GL_DOUBLE_VEC2, {2*8, GL_DOUBLE_VEC2}},
        {GL_DOUBLE_VEC3, {3*8, GL_DOUBLE_VEC3}},
        {GL_DOUBLE_VEC4, {4*8, GL_DOUBLE_VEC4}},
        {GL_INT, {1*4, GL_INT}},
        {GL_INT_VEC2, {2*4, GL_INT_VEC2}},
        {GL_INT_VEC3, {3*4, GL_INT_VEC3}},
        {GL_INT_VEC4, {4*4, GL_INT_VEC4}},
        {GL_UNSIGNED_INT, {1*4, GL_UNSIGNED_INT}},
        {GL_UNSIGNED_INT_VEC2, {2*4, GL_UNSIGNED_INT_VEC2}},
        {GL_UNSIGNED_INT_VEC3, {3*4, GL_UNSIGNED_INT_VEC3}},
        {GL_UNSIGNED_INT_VEC4, {4*4, GL_UNSIGNED_INT_VEC4}},
        {GL_BOOL, {1*4, GL_BOOL}},
        {GL_BOOL_VEC2, {2*4, GL_BOOL_VEC2}},
        {GL_BOOL_VEC3, {3*4, GL_BOOL_VEC3}},
        {GL_BOOL_VEC4, {4*4, GL_BOOL_VEC4}},
        {GL_FLOAT_MAT2, {2*2*4, GL_FLOAT_MAT2}},
        {GL_FLOAT_MAT3, {3*3*4, GL_FLOAT_MAT3}},
        {GL_FLOAT_MAT4, {4*4*4, GL_FLOAT_MAT4}},
        {GL_FLOAT_MAT2x3, {2*3*4, GL_FLOAT_MAT2x3}},
        {GL_FLOAT_MAT2x4, {2*4*4, GL_FLOAT_MAT2x4}},
        {GL_FLOAT_MAT3x2, {3*2*4, GL_FLOAT_MAT3x2}},
        {GL_FLOAT_MAT3x4, {3*4*4, GL_FLOAT_MAT3x4}},
        {GL_FLOAT_MAT4x2, {4*2*4, GL_FLOAT_MAT4x2}},
        {GL_FLOAT_MAT4x3, {4*3*4, GL_FLOAT_MAT4x3}},
        {GL_DOUBLE_MAT2, {2*2*8, GL_DOUBLE_MAT2}},
        {GL_DOUBLE_MAT3, {3*3*8, GL_DOUBLE_MAT3}},
        {GL_DOUBLE_MAT4, {4*4*8, GL_DOUBLE_MAT4}},
        {GL_DOUBLE_MAT2x3, {2*3*8, GL_DOUBLE_MAT2x3}},
        {GL_DOUBLE_MAT2x4, {2*4*8, GL_DOUBLE_MAT2x4}},
        {GL_DOUBLE_MAT3x2, {3*2*8, GL_DOUBLE_MAT3x2}},
        {GL_DOUBLE_MAT3x4, {3*4*8, GL_DOUBLE_MAT3x4}},
        {GL_DOUBLE_MAT4x2, {4*2*8, GL_DOUBLE_MAT4x2}},
        {GL_DOUBLE_MAT4x3, {4*3*8, GL_DOUBLE_MAT4x3}}
    };

    const std::map<uint32_t, TextureType> samplerTypes = {
        {GL_SAMPLER_2D, TextureType::IMAGE2D},
        {GL_SAMPLER_3D, TextureType::IMAGE3D},
        {GL_SAMPLER_CUBE, TextureType::CUBEMAP}
    };

    const std::map<uint32_t, TextureType> imageTypes = {
        {GL_IMAGE_2D, TextureType::IMAGE2D},
        {GL_IMAGE_3D, TextureType::IMAGE3D},
        {GL_IMAGE_CUBE, TextureType::CUBEMAP}
    };
}

Shader Shader::loadShader(const std::string& shaderName)
{
    Shader s;
    s.load(shaderName);
    return std::move(s);
}


bool Shader::load(const std::string& shaderName)
{
    mProgram = ShaderProgramLoader::getInstance()->loadProgram(shaderName);
    const uint32_t pId = mProgram->getId();

    if(!mProgram->isValid()) return false;

    char nameBuffer[256];

    // Load all uniforms
    std::vector<std::pair<uint32_t, std::string>> atomicCounters;

    GLint numUniforms = 0;
    glGetProgramInterfaceiv(pId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    for(uint32_t sId = 0; sId < numUniforms; sId++)
    {
        GLsizei nameLength; GLint numElem; GLenum type;
        glGetActiveUniform(pId, sId, 256, &nameLength, &numElem, &type, nameBuffer);
        std::string uniformName(nameBuffer, nameLength);

        if(type == GL_UNSIGNED_INT_ATOMIC_COUNTER) // Special treatment for atomic counters
        {
            uint32_t bId = glGetProgramResourceIndex(pId, GL_UNIFORM, uniformName.c_str());
            atomicCounters.push_back(std::pair(bId, uniformName));
            continue;
        }
        auto uniTypeIt = internal::uniformTypes.find(type);
        auto samplerTypeIt = internal::samplerTypes.find(type);
        auto imgTypeIt = internal::imageTypes.find(type);
        if(uniTypeIt != internal::uniformTypes.end())
        {
            mUniformsInfo.emplace(std::make_pair(std::move(uniformName), 
                                    UniformInfo{sId, uniTypeIt->second, static_cast<uint32_t>(numElem), nullptr}));
        }
        else if(samplerTypeIt != internal::samplerTypes.end())
        {
            // Its a sampler
            mSamplersInfo.emplace(std::make_pair(std::move(uniformName), TextureInfo{samplerTypeIt->second, nullptr}));

        }
        else if(imgTypeIt != internal::imageTypes.end())
        {
            // Its a texture
            mImagesInfo.emplace(std::make_pair(std::move(uniformName), TextureInfo{imgTypeIt->second, nullptr}));
        }
        else
        {
            std::cout << "Cannot recognize uniform '" << uniformName << "' with type " << type << std::endl;
        }
    }

    std::sort(atomicCounters.begin(), atomicCounters.end(), [](const auto& v1, const auto& v2) { return v1.first < v2.first; });
    uint32_t nextAtomicCounterIdx = 0;

    // Load all SSBOs
    GLint numBuffers = 0;
    // for(uint32_t bufferType : {GL_SHADER_STORAGE_BLOCK, GL_UNIFORM_BUFFER})
    for(std::pair<GLenum, GLenum> bufferType : {std::make_pair(GL_SHADER_STORAGE_BLOCK, GL_SHADER_STORAGE_BUFFER), std::make_pair(GL_ATOMIC_COUNTER_BUFFER, GL_ATOMIC_COUNTER_BUFFER)})
    {
        glGetProgramInterfaceiv(pId, bufferType.first, GL_ACTIVE_RESOURCES, &numBuffers);
        for(uint32_t bId = 0; bId < numBuffers; bId++)
        {
            ShaderBuffer sb;
            sb.bufferType = bufferType.second;
            sb.buffer = nullptr;

            // Get name
            std::string bufferName;
            if(bufferType.first == GL_ATOMIC_COUNTER_BUFFER)
            {
                bufferName = atomicCounters[nextAtomicCounterIdx++].second;
            }
            else
            {
                GLint nameLength = 0;
                glGetProgramResourceName(pId, bufferType.first, bId, 255, &nameLength, nameBuffer);
                bufferName = std::string(nameBuffer, nameLength);
            }

            // Get binding
            const GLenum bufferBindingArray[1] = {GL_BUFFER_BINDING};
            glGetProgramResourceiv(pId, bufferType.first, bId, 1, bufferBindingArray, 1, NULL, reinterpret_cast<GLint*>(&sb.bindingIndex));

            // Iterate buffer variables
            GLint numActiveBuffers = 0;
            const GLenum numActiveBufferArray[1] = {GL_NUM_ACTIVE_VARIABLES};
            glGetProgramResourceiv(pId, bufferType.first, bId, 1, numActiveBufferArray, 1, NULL, &numActiveBuffers);

            const GLenum activeBufferArray[1] = {GL_ACTIVE_VARIABLES};
            std::vector<GLint> blockVarId(numActiveBuffers);
            glGetProgramResourceiv(pId, bufferType.first, bId, 1, activeBufferArray, numActiveBuffers, NULL, &blockVarId[0]);
            sb.variables.resize(numActiveBuffers);
            for(uint32_t i=0; i < numActiveBuffers; i++)
            {
                const GLenum varPropsArray[4] = {GL_OFFSET, GL_TYPE, GL_ARRAY_SIZE, GL_ARRAY_STRIDE};
                GLint props[4];
                glGetProgramResourceiv(pId, GL_BUFFER_VARIABLE, blockVarId[i], 4, varPropsArray, 4, NULL, props);
                auto tIt = internal::uniformTypes.find(static_cast<uint32_t>(props[1]));
                sb.variables[i] = ShaderBufferVariable{static_cast<uint32_t>(props[0]), tIt->second,
                                                    static_cast<uint32_t>(props[2]), static_cast<uint32_t>(props[3])};
            }
            mBuffersInfo.emplace(std::make_pair(std::move(bufferName), std::move(sb)));
        }
    }

    mValid = true;
    return true;
}

void Shader::bind(Camera* camera, glm::mat4x4* modelMatrix)
{
    mProgram->use();

    glm::mat4x4 res(1.0);
    if(modelMatrix != nullptr)
    {
        res = *modelMatrix;
    }

    // Set basic camera matrices
    if(camera != nullptr)
    {
        setUniform("modelMatrix", res);
        setUniform("normalModelMatrix", glm::inverseTranspose(glm::mat3(res)));
        setUniform("viewMatrix", camera->getViewMatrix());
        res = camera->getViewMatrix() * res;
        setUniform("viewModelMatrix", res);
        setUniform("normalViewModelMatrix", glm::inverseTranspose(glm::mat3(res)));
        setUniform("projectionMatrix", camera->getProjectionMatrix());
        res = camera->getProjectionMatrix() * res;
        setUniform("projectionViewModelMatrix", res);
    }

    for(auto const& bInfo : mBuffersInfo)
    {
        glBindBufferBase(bInfo.second.bufferType, bInfo.second.bindingIndex, bInfo.second.buffer->getId());
        glBindBuffer(bInfo.second.bufferType, 0);
    }
    // Iterate all uniforms, textures, and images. For setting the value
    // TODO
}

bool Shader::setBufferSize(const std::string& name, uint32_t sizeInBytes)
{
    mProgram->use();
	auto it = mBuffersInfo.find(name);
	if(it == mBuffersInfo.end()) return false;

	if(it->second.buffer == nullptr) // Create buffer if not exists
	{
		it->second.buffer = std::make_shared<Buffer>(it->second.bufferType);
	}

	it->second.buffer->resize(sizeInBytes);
	const uint32_t ssboLoc = it->second.buffer->getId();
	glBindBufferBase(it->second.bufferType, it->second.bindingIndex, ssboLoc);
	glBindBuffer(it->second.bufferType, 0);
	return true;
}

void Shader::Buffer::resize(uint32_t sizeInBytes)
{
    const uint32_t ssboLoc = mLocId;
	glBindBuffer(mBufferType, ssboLoc);
	glBufferData(mBufferType, sizeInBytes, NULL, GL_STATIC_DRAW);
}

size_t Shader::getBufferSize(const std::string& name)
{
    mProgram->use();
	auto it = mBuffersInfo.find(name);
	if(it == mBuffersInfo.end() || it->second.buffer == nullptr) return 0;
    return it->second.buffer->getSize();
}

size_t Shader::Buffer::getSize()
{
    const uint32_t ssboLoc = mLocId;
	glBindBuffer(mBufferType, ssboLoc);
    int64_t size = 0;
    glGetBufferParameteri64v(mBufferType, GL_BUFFER_SIZE, &size);

    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "Error " << err << std::endl;
    }

    return static_cast<size_t>(size);
}

bool Shader::setBuffer(const std::string& name, std::shared_ptr<Shader::Buffer> buffer)
{
    auto it = mBuffersInfo.find(name);
	if(it == mBuffersInfo.end() || (buffer != nullptr && buffer->getType() != it->second.bufferType)) return false;
    it->second.buffer = std::move(buffer);
    return true;
}

std::shared_ptr<Shader::Buffer> Shader::getBuffer(const std::string& name)
{
    auto it = mBuffersInfo.find(name);
	if(it == mBuffersInfo.end()) return nullptr;
    else return it->second.buffer;
}

}