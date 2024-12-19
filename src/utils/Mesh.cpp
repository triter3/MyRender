#include <iostream>
#include <assert.h>
#include <MyRender/utils/Mesh.h>

namespace myrender
{

Mesh::Mesh(glm::vec3* vertices, uint32_t numVertices,
           uint32_t* indices, uint32_t numIndices)
{
    mVertices.resize(numVertices);
    std::memcpy(mVertices.data(), vertices, sizeof(glm::vec3) * numVertices);

    mIndices.resize(numIndices);
    std::memcpy(mIndices.data(), indices, sizeof(uint32_t) * numIndices);
}

void Mesh::computeBoundingBox()
{
    glm::vec3 min(INFINITY);
    glm::vec3 max(-INFINITY);
    for(glm::vec3& vert : mVertices)
    {
        min.x = glm::min(min.x, vert.x);
        max.x = glm::max(max.x, vert.x);

        min.y = glm::min(min.y, vert.y);
        max.y = glm::max(max.y, vert.y);

        min.z = glm::min(min.z, vert.z);
        max.z = glm::max(max.z, vert.z);
    }
    mBBox = BoundingBox(min, max);
}

void Mesh::computeNormals()
{
    mNormals.clear();
    mNormals.assign(mVertices.size(), glm::vec3(0.0f));

    for (int i = 0; i < mIndices.size(); i += 3)
    {
        const glm::vec3 v1 = mVertices[mIndices[i]];
        const glm::vec3 v2 = mVertices[mIndices[i + 1]];
        const glm::vec3 v3 = mVertices[mIndices[i + 2]];
        const glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

        mNormals[mIndices[i]] += glm::acos(glm::dot(glm::normalize(v2 - v1), glm::normalize(v3 - v1))) * normal;
        mNormals[mIndices[i + 1]] += glm::acos(glm::dot(glm::normalize(v1 - v2), glm::normalize(v3 - v2))) * normal;
        mNormals[mIndices[i + 2]] += glm::acos(glm::dot(glm::normalize(v1 - v3), glm::normalize(v2 - v3))) * normal;
    }

    for(glm::vec3& n : mNormals)
    {
        n = glm::normalize(n);
    }
}

void Mesh::applyTransform(glm::mat4 trans)
{
    for(glm::vec3& vert : mVertices)
    {
        vert = glm::vec3(trans * glm::vec4(vert, 1.0));
    }

    computeBoundingBox();
}

}