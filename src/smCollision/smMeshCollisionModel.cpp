/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#include "smCollision/smMeshCollisionModel.h"

smMeshCollisionModel::smMeshCollisionModel()
{

}

smMeshCollisionModel::~smMeshCollisionModel()
{

}
std::shared_ptr<smMesh> smMeshCollisionModel::getMesh()
{
    return this->mesh;
}
void smMeshCollisionModel::loadTriangleMesh(const std::string& meshName, const smMeshFileType &type)
{
    if(this->mesh = nullptr)
    {
        this->mesh = std::make_shared<smSurfaceMesh>();
    }

    this->mesh->loadMesh(meshName,type);
    this->aabbTree = std::make_shared<AABBTreeType>(std::static_pointer_cast<smSurfaceMesh>(this->mesh).get(), 6);
    this->aabbTree->initStructure();
}
void smMeshCollisionModel::loadTetraMesh(const std::string& meshName)
{
    if(this->mesh = nullptr)
    {
        this->mesh = std::make_shared<smVolumeMesh>();
    }

//     this->mesh->loadMesh(meshName);
}
std::shared_ptr< smMeshCollisionModel::AABBTreeType > smMeshCollisionModel::getAABBTree()
{
    return this->aabbTree;
}
const smVec3f& smMeshCollisionModel::getNormal(size_t i) const
{
    return this->mesh->triNormals[i];
}
std::array<smVec3f,3> smMeshCollisionModel::getTrianglePositions(size_t i) const
{
    std::array<smVec3f, 3> vertices;
    vertices[0] = this->mesh->vertices[this->mesh->triangles[i].vert[0]];
    vertices[1] = this->mesh->vertices[this->mesh->triangles[i].vert[1]];
    vertices[2] = this->mesh->vertices[this->mesh->triangles[i].vert[2]];

    return vertices;
}
