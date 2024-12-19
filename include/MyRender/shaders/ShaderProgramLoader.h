#ifndef SHADERPROGRAMLOADER_H
#define SHADERPROGRAMLOADER_H

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "MyRender/shaders/ShaderProgram.h"

namespace myrender
{

class ShaderProgramLoader
{
public:
	ShaderProgramLoader() : mSearchPaths({"./shaders"}) {}

	static ShaderProgramLoader* getInstance()
	{
		if(instance == nullptr)
		{
			instance = std::make_unique<ShaderProgramLoader>();
		}
		return instance.get();
	}

	const std::vector<std::string>& getSearchPaths() { return mSearchPaths; }
	void addSearchPath(const std::string& path) { mSearchPaths.push_back(path); }

	std::shared_ptr<ShaderProgram> loadProgram(const std::string& name);
private:
	inline static std::unique_ptr<ShaderProgramLoader> instance = nullptr;
	std::vector<std::string> mSearchPaths;
    std::map<std::string, std::weak_ptr<ShaderProgram>> mPrograms;
};

}

#endif