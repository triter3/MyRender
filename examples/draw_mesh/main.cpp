#include <iostream>
#include <array>
#include "Scene.h"
#include "MainLoop.h"
#include "slang.h"
#include "slang-com-ptr.h"
#include "RenderMesh.h"
#include "utils/PrimitivesFactory.h"
#include "shaders/Shader.h"
#include "NavigationCamera.h"
#include "Window.h"

using namespace MyRender;

int main()
{
    MainLoop loop;
    Scene scene([](Scene& s) {
        auto nCamera = s.createSystem<NavigationCamera>();
        nCamera->setDiableMouseOnRotation(false);
        s.setMainCamera(nCamera);
        auto myCube = PrimitivesFactory::getCube();
        myCube->computeNormals();
        auto mesh = s.createSystem<RenderMesh>();
        mesh->setMeshData(*myCube);
        mesh->setShader(Shader::loadShader("LightRender"));
        std::vector<float> v = { 0.0f };
        mesh->getShader().setBufferData("mydata", v);
        mesh->systemName = "My Mesh";
        Window::getCurrentWindow().setBackgroudColor(glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
    });

    loop.start(scene);
}