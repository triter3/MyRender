#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <glm/glm.hpp>

namespace myrender
{

class Camera;
class Scene;

class System
{
public:
    std::string systemName = "";
    bool callUpdate = true;
    bool callDraw = true;
    bool callDrawGui = true;
    uint32_t getSystemId() { return systemId; }

    virtual void start() {};
    virtual void update(float deltaTime) {};
    virtual void resize(glm::ivec2 windowSize) {};
	virtual void draw(Camera* camera) {};
	virtual void drawGui() {};
private:
    uint32_t systemId = 0;
    friend Scene;
};

}

#endif // SYSTEM_H