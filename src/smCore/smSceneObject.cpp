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

#include "smCore/smSceneObject.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smSDK.h"

///attach the simulator to the  object
void smSceneObject::attachObjectSimulator(std::shared_ptr<smObjectSimulator> p_objectSim)
{
    p_objectSim->addObject(safeDownCast<smSceneObject>());
}

///release the simulator from the object
void smSceneObject::releaseObjectSimulator()
{
    objectSim->removeObject(safeDownCast<smSceneObject>());
    objectSim = nullptr;
}

///get a reference to the simulator
std::shared_ptr<smObjectSimulator> smSceneObject::getObjectSimulator()
{
    return objectSim;
}

///attach the custom renderer to the scene object
void smSceneObject::attachCustomRenderer(std::shared_ptr<smCustomRenderer> p_customeRenderer)
{
    customRender = p_customeRenderer;
}

///release the custom renderer from the class
void smSceneObject::releaseCustomeRenderer()
{
    customRender = nullptr;
}

smSceneObject::smSceneObject()
{
    type = SIMMEDTK_SMSCENEBOJECT;
    objectSim = nullptr;
    customRender = nullptr;
//     smSDK::getInstance()->registerSceneObject(safeDownCast<smSceneObject>());
    flags.isViewerInit = false;
    flags.isSimulatorInit = false;
    name = "SceneObject" + std::to_string(uniqueId.ID);
}
