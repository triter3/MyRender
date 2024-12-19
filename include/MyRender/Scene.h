#ifndef SCENE_H
#define SCENE_H


#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include "MyRender/Camera.h"
#include "MyRender/System.h"

namespace myrender
{

class Scene 
{
public:
	Scene();
	Scene(std::function<void(Scene&)> startFunc) : startFunc(startFunc) {}
	virtual void start() 
	{
		if(startFunc) (*startFunc)(*this);
	}
	virtual void update(float deltaTime);
	virtual void draw();
	virtual void resize(glm::ivec2 windowSize);

	// Camera set and getter
	void setMainCamera(std::shared_ptr<Camera> ptr ) 
	{
		mainCamera = ptr;
	}

	Camera* getMainCamera() 
	{
		return mainCamera.get();
	}

	void addSystem(std::shared_ptr<System> system)
	{
		system->systemId = nextSystemId++;
		system->start();
		systems.push_back(system);
	}

	template<typename T, class... Types>
	std::shared_ptr<T> createSystem(Types&&... args)
	{
		std::shared_ptr<T> sp = std::make_shared<T>(args...);
		addSystem(sp);
		return sp;
	}

private:
	uint32_t nextSystemId = 0;
	std::shared_ptr<Camera> mainCamera;
	std::vector<std::shared_ptr<System>> systems;
	std::optional<std::function<void(Scene&)>> startFunc;

};

}

#endif // SCENE_H