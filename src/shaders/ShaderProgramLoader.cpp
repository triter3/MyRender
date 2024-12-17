#include "shaders/ShaderProgramLoader.h"

namespace MyRender
{

std::shared_ptr<ShaderProgram> ShaderProgramLoader::loadProgram(const std::string& name)
{
    auto it = mPrograms.find(name);
    if(it != mPrograms.end() && !it->second.expired())
    {
        return it->second.lock();
    }
    else
    {
        auto ptr = std::make_shared<ShaderProgram>(name);
        mPrograms[name] = ptr;
        return ptr;
    }
}

}