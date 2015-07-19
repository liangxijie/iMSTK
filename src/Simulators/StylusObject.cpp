// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "StylusObject.h"
#include "Core/CollisionModel.h"
#include "Collision/SurfaceTree.h"
#include "Collision/OctreeCell.h"
#include "Mesh/Mesh.h"
#include "Rendering/GLUtils.h"

smStylusSceneObject::smStylusSceneObject(std::shared_ptr<smErrorLog>/*p_log*/) : smSceneObject()
{
    type = core::ClassType::StylusSceneObject;
    toolEnabled = true;
    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createConcreteClass(
        "StylusRenderDelegate"));
}

smStylusRigidSceneObject::smStylusRigidSceneObject(std::shared_ptr<smErrorLog>/*p_log*/)
{
    type = core::ClassType::StylusRigidSceneObject;
    updateViewerMatrixEnabled = true;
    rootIterator = meshes.begin();
    posCallBackEnabledForEntireObject = false;
    enableDeviceManipulatedTool = false;
}

smStylusDeformableSceneObject::smStylusDeformableSceneObject(
                                            std::shared_ptr<smErrorLog>/*p_log*/)
                                            : smStylusSceneObject()
{
    type = core::ClassType::StylusSeformableSceneObject;
}

smMeshContainer *smStylusRigidSceneObject::getMeshContainer(std::string p_string) const
{
    tree<smMeshContainer*>::iterator iter = meshes.begin_leaf();

    while (iter != meshes.end_leaf())
    {
        if (iter.node->data->name == p_string)
        {
            return (*iter);
        }

        iter++;
    }
    return nullptr;
}

void smStylusRigidSceneObject::posTraverseCallBack(smMeshContainer &p_container)
{
    if (p_container.colModel != NULL)
    {
        p_container.colModel->transRot = p_container.currentMatrix;
        p_container.colModel->translateRot();
    }
}

smMeshContainer::smMeshContainer( std::string p_name )
{
    name = p_name;
    offsetRotX = 0.0;
    offsetRotY = 0.0;
    offsetRotZ = 0.0;
    preOffsetPos = smVec3d::Zero();
    posOffsetPos = smVec3d::Zero();
    mesh = NULL;
    colModel = NULL;
}

smMeshContainer::smMeshContainer( std::string p_name, smMesh */*p_mesh*/, smVec3d p_prePos, smVec3d p_posPos, float p_offsetRotX, float p_offsetRotY, float p_offsetRotZ )
{
    offsetRotX = p_offsetRotX;
    offsetRotY = p_offsetRotY;
    offsetRotZ = p_offsetRotZ;
    preOffsetPos = p_prePos;
    posOffsetPos = p_posPos;
    name = p_name;
    colModel = NULL;
}

void smMeshContainer::computeCurrentMatrix()
{
    Eigen::Affine3d preTranslate( Eigen::Translation3d( preOffsetPos[0], preOffsetPos[1], preOffsetPos[2] ) );
    Eigen::Affine3d posTranslate( Eigen::Translation3d( posOffsetPos[0], posOffsetPos[1], posOffsetPos[2] ) );
    Eigen::Affine3d rx( Eigen::Affine3d( Eigen::AngleAxisd( SM_PI_TWO * offsetRotX, smVec3d::UnitX() ) ) );
    Eigen::Affine3d ry( Eigen::Affine3d( Eigen::AngleAxisd( SM_PI_TWO * offsetRotY, smVec3d::UnitY() ) ) );
    Eigen::Affine3d rz( Eigen::Affine3d( Eigen::AngleAxisd( SM_PI_TWO * offsetRotZ, smVec3d::UnitZ() ) ) );

    smMatrix44d transform = ( preTranslate * rx * ry * rz * posTranslate ).matrix();
    tempCurrentMatrix *= transform;
    tempCurrentDeviceMatrix *= transform;
}

smStylusPoints::smStylusPoints()
{
    point = smVec3d::Zero();
    container = NULL;
}

void smStylusSceneObject::serialize( void */*p_memoryBlock*/ )
{
}

void smStylusSceneObject::unSerialize( void */*p_memoryBlock*/ )
{
}

void smStylusSceneObject::handleEvent(std::shared_ptr<mstk::Event::smEvent>/*p_event*/ ) {}
void smStylusRigidSceneObject::posTraverseCallBack()
{
}

tree< smMeshContainer * >::iterator smStylusRigidSceneObject::addMeshContainer( smMeshContainer *p_meshContainer )
{
    tree<smMeshContainer *>::iterator iter;

    if ( meshes.size() > 1 )
    {
        iter = meshes.append_child( rootIterator, p_meshContainer );
    }

    else
    {
        iter = meshes.insert( rootIterator, p_meshContainer );
    }

    indexIterators[p_meshContainer->name] = iter;
    return iter;
}

bool smStylusRigidSceneObject::addMeshContainer( std::string p_ParentName, smMeshContainer *p_meshContainer )
{
    tree<smMeshContainer *>::iterator iter;

    if ( p_ParentName.size() > 0 )
    {
        if ( indexIterators.count( p_ParentName ) > 0 )
        {
            iter = indexIterators[p_ParentName];
            meshes.append_child( iter, p_meshContainer );
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

tree< smMeshContainer * >::iterator smStylusRigidSceneObject::addMeshContainer( tree< smMeshContainer * >::iterator p_iterator, smMeshContainer *p_meshContainer )
{
    return meshes.insert( p_iterator, p_meshContainer );
}

void smStylusRigidSceneObject::handleEvent(std::shared_ptr<mstk::Event::smEvent>/*p_event*/ ) {}

std::shared_ptr<smSceneObject> smStylusRigidSceneObject::clone()
{
    // WARNING: What is the purpose of this function
    std::shared_ptr<smStylusRigidSceneObject> ret = std::make_shared<smStylusRigidSceneObject>();
    return ret;
}

//void smStylusSceneObject::init() {}
