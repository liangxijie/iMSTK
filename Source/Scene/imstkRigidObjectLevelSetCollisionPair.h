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

#include "imstkCollisionPair.h"
#include "imstkCollisionDetection.h"

namespace imstk
{
class LevelSetDeformableObject;

namespace expiremental
{
class RigidObject2;

///
/// \class RigidObjectObjectCollisionPair
///
/// \brief This class defines a collision interaction between a RigidObject
/// and levelset.
///
class RigidObjectLevelSetCollisionPair : public CollisionPair
{
public:
    RigidObjectLevelSetCollisionPair(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<LevelSetDeformableObject> obj2);
    virtual ~RigidObjectLevelSetCollisionPair() override = default;
};
}
}