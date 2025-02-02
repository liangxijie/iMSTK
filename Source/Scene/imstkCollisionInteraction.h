/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#pragma once

#include "imstkSceneObject.h"

namespace imstk
{
class CollisionData;
class CollisionDetectionAlgorithm;
class CollisionHandling;
class CollidingObject;

///
/// \class CollisionInteraction
///
/// \brief Abstract class for defining collision interactions between objects
///
class CollisionInteraction : public SceneObject
{
public:
    virtual ~CollisionInteraction() override = default;

protected:
    CollisionInteraction(std::string objName,
                         std::shared_ptr<CollidingObject> obj1, std::shared_ptr<CollidingObject> obj2);

public:
    void setCollisionDetection(std::shared_ptr<CollisionDetectionAlgorithm> colDetect);

    ///
    /// \brief Set the Collision Handling for object A
    ///
    void setCollisionHandlingA(std::shared_ptr<CollisionHandling> colHandlingA);

    ///
    /// \brief Set the Collision Handling for object B
    ///
    void setCollisionHandlingB(std::shared_ptr<CollisionHandling> colHandlingB);

    ///
    /// \brief Set the two-way Collision Handling for both objects
    ///
    void setCollisionHandlingAB(std::shared_ptr<CollisionHandling> colHandlingAB);

    std::shared_ptr<CollisionDetectionAlgorithm> getCollisionDetection() const { return m_colDetect; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingA() const { return m_colHandlingA; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingB() const { return m_colHandlingB; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingAB() const { return m_colHandlingA; }

    std::shared_ptr<TaskNode> getCollisionDetectionNode() const { return m_collisionDetectionNode; }
    std::shared_ptr<TaskNode> getCollisionHandlingANode() const { return m_collisionHandleANode; }
    std::shared_ptr<TaskNode> getCollisionHandlingBNode() const { return m_collisionHandleBNode; }

    void updateCollisionGeometry();

protected:
    ///
    /// \brief Update collision
    ///
    void updateCD();

    ///
    /// \brief Update handler A
    ///
    void updateCHA();

    ///
    /// \brief Update handler B
    ///
    void updateCHB();

protected:
    std::shared_ptr<CollidingObject> m_objA = nullptr;
    std::shared_ptr<CollidingObject> m_objB = nullptr;

    std::shared_ptr<CollisionDetectionAlgorithm> m_colDetect = nullptr; ///< Collision detection algorithm
    std::shared_ptr<CollisionHandling> m_colHandlingA = nullptr;
    std::shared_ptr<CollisionHandling> m_colHandlingB = nullptr;

    std::shared_ptr<TaskNode> m_collisionDetectionNode      = nullptr;
    std::shared_ptr<TaskNode> m_collisionHandleANode        = nullptr;
    std::shared_ptr<TaskNode> m_collisionHandleBNode        = nullptr;
    std::shared_ptr<TaskNode> m_collisionGeometryUpdateNode = nullptr;
};
}