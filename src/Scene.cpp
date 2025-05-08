#include "MyRender/Scene.h"
#include <imgui.h>

namespace myrender
{

void Scene::update(float deltaTime)
{
    for(auto& s : systems)
    {
        if(s->callUpdate) s->update(deltaTime);
    }

    for(auto& s : systems)
    {
        if(s->callDrawGui)
        {
            ImGui::PushID(s->systemId);
            s->drawGui();
            ImGui::PopID();
        }
    }
}

void Scene::draw()
{
    if(mainCamera != nullptr)
    {
        for(auto& s : systems)
        {
            if(s->callDraw) s->draw(mainCamera.get());
        }
    }
}

void Scene::resize(glm::ivec2 windowSize)
{
    for(auto& s : systems)
    {
        s->resize(windowSize);
    }
}

}