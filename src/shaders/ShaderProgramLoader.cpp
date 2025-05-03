#include "MyRender/shaders/ShaderProgramLoader.h"

namespace myrender
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

bool ShaderProgramLoader::reloadProgram(const std::string& name)
{
    auto it = mPrograms.find(name);
    if(it != mPrograms.end() && !it->second.expired())
    {
        auto* ptr = new ShaderProgram(name);
        *(it->second.lock()) = std::move(*ptr);
    } else return false;
    return true;
}

}