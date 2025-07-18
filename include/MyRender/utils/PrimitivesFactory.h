#ifndef PRIMITIVES_FACTORY_H
#define PRIMITIVES_FACTORY_H

#include <memory>
#include "MyRender/utils/Mesh.h"

namespace myrender
{

namespace PrimitivesFactory
{
    std::shared_ptr<Mesh> getIsosphere(uint32_t subdivisions = 0);
    std::shared_ptr<Mesh> getPlane();
    std::shared_ptr<Mesh> getCube();
}

}

#endif