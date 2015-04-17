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

#ifndef SMTOOLSIMULATOR_H
#define SMTOOLSIMULATOR_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventData.h"
#include "smSimulators/smStylusObject.h"


/// \brief Example tool simulator
class smToolSimulator: public smObjectSimulator
{

public:
    /// \brief constructor
    smToolSimulator(std::shared_ptr<smErrorLog> p_errorLog);

    /// \brief update everything related to tool
    void updateTool(std::shared_ptr<smStylusRigidSceneObject> p_tool);

    /// \brief !!
    virtual void initCustom();

    /// \brief run the tool simulator in a loop here
    virtual void run();

    /// \brief synchronize the buffers in the object..do not call by yourself.
    void syncBuffers();

    /// \brief handle the events such as button presses related to tool
    void handleEvent(std::shared_ptr<smEvent> p_event);
};

#endif
