#include <iostream>
#include <array>
#include "MyRender/Scene.h"
#include "MyRender/MainLoop.h"
#include "MyRender/RenderMesh.h"
#include "MyRender/utils/PrimitivesFactory.h"
#include "MyRender/shaders/Shader.h"
#include "MyRender/NavigationCamera.h"
#include "MyRender/Window.h"

using namespace myrender;

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