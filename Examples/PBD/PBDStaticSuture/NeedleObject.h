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

#include "imstkRigidObject2.h"

using namespace imstk;

class NeedleObject : public RigidObject2
{
public:
    enum class CollisionState
    {
        REMOVED,
        TOUCHING,
        INSERTED
    };

public:
    NeedleObject();
    virtual ~NeedleObject() = default;

    virtual const std::string getTypeName() const override { return "NeedleObject"; }

public:
    // *INDENT-OFF*
    SIGNAL(NeedleObject, inserted);
    SIGNAL(NeedleObject, removed);
    // *INDENT-ON*

public:
    void setCollisionState(const CollisionState state)
    {
        // If current state is inserted and previous was not inserted
        if (m_collisionState == CollisionState::INSERTED && state != CollisionState::INSERTED)
        {
            this->postEvent(Event(removed()));
        }
        // If current state not inserted and previous inserted
        else if (m_collisionState != CollisionState::INSERTED && state == CollisionState::INSERTED)
        {
            this->postEvent(Event(inserted()));
        }
        m_collisionState = state;
    }

    CollisionState getCollisionState() const { return m_collisionState; }

    ///
    /// \brief Set the force threshold for the needle
    ///
    void setForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    double getForceThreshold() const { return m_forceThreshold; }

    void setArc(const Vec3d& arcCenter, const Mat3d& arcBasis,
                double arcRadius, double beginRad, double endRad)
    {
        m_arcCenter = arcCenter;
        m_arcBasis  = arcBasis;
        m_beginRad  = beginRad;
        m_endRad    = endRad;
        m_arcRadius = arcRadius;
    }

    ///
    /// \brief Get the basis post transformation of the rigid body
    ///
    const Mat3d getArcBasis();
    ///
    /// \brief Get the arc center post transformation of the rigid body
    ///
    const Vec3d getArcCenter();
    const double getBeginRad() const { return m_beginRad; }
    const double getEndRad() const { return m_endRad; }
    const double getArcRadius() const { return m_arcRadius; }

protected:
    CollisionState m_collisionState = CollisionState::REMOVED;
    double m_forceThreshold = 5.0;

    Mat3d  m_arcBasis  = Mat3d::Identity();
    Vec3d  m_arcCenter = Vec3d::Zero();
    double m_arcRadius = 1.0;
    double m_beginRad  = 0.0;
    double m_endRad    = PI * 2.0;
};