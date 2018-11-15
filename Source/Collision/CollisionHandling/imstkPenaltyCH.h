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

#ifndef imstkPenaltyCH_h
#define imstkPenaltyCH_h

// std library
#include <memory>

// imstk
#include "imstkCollisionHandling.h"

namespace imstk
{
class CollidingObject;
class DeformableObject;
class CollisionData;

///
/// \class PenaltyCH
///
/// \brief Implements penalty collision handling
///
class PenaltyCH : public CollisionHandling
{
public:

    ///
    /// \brief Constructor
    ///
    PenaltyCH(const Side& side,
              const CollisionData& colData,
              std::shared_ptr<CollidingObject> obj) :
        CollisionHandling(Type::Penalty, side, colData),
        m_object(obj){}

    PenaltyCH() = delete;

    ///
    /// \brief Destructor
    ///
    ~PenaltyCH() = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    void processCollisionData() override;
    void computeContactForcesAnalyticRigid(std::shared_ptr<CollidingObject> analyticObj);
    void computeContactForcesDiscreteDeformable(std::shared_ptr<DeformableObject> deformableObj);

    ///
    /// \brief Set the contact stiffness
    ///
    void setContactStiffness(const double stiffness)
    {
        m_stiffness = stiffness;
    }

    ///
    /// \brief Set the contact velocity damping
    ///
    void setContactVelocityDamping(const double damping)
    {
        m_damping = damping;
    }

private:

    std::shared_ptr<CollidingObject> m_object;    ///>

    double m_stiffness = 5.0e5; ///> Stiffness of contact
    double m_damping = 0.5;     ///> Damping of the contact
};
}

#endif // ifndef imstkPenaltyCH_h