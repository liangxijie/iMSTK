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

#include "imstkSPHState.h"
#include "imstkLogger.h"
#include "imstkVecDataArray.h"

namespace imstk
{
SPHState::SPHState(const int numElements) :
    m_positions(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_fullStepVelocities(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_halfStepVelocities(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_velocities(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_BDPositions(std::make_shared<VecDataArray<double, 3>>()),
    m_Densities(std::make_shared<DataArray<double>>(numElements)),
    m_Normals(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_Accels(std::make_shared<VecDataArray<double, 3>>(numElements)),
    m_DiffuseVelocities(std::make_shared<VecDataArray<double, 3>>(numElements))
{
    std::fill_n(m_Densities->getPointer(), m_Densities->size(), 1.0);
    std::fill_n(m_Accels->getPointer(), m_Accels->size(), Vec3d(0, 0, 0));
    std::fill_n(m_DiffuseVelocities->getPointer(), m_DiffuseVelocities->size(), Vec3d(0, 0, 0));
    std::fill_n(m_velocities->getPointer(), m_velocities->size(), Vec3d(0.0, 0.0, 0.0));
    std::fill_n(m_halfStepVelocities->getPointer(), m_halfStepVelocities->size(), Vec3d(0.0, 0.0, 0.0));
    std::fill_n(m_fullStepVelocities->getPointer(), m_fullStepVelocities->size(), Vec3d(0.0, 0.0, 0.0));

    m_NeighborInfo.resize(static_cast<size_t>(numElements));
    m_NeighborLists.resize(static_cast<size_t>(numElements));
}

void
SPHState::setState(std::shared_ptr<SPHState> rhs)
{
    *m_positions  = *rhs->getPositions();
    *m_velocities = *rhs->getVelocities();
    *m_halfStepVelocities = *rhs->getHalfStepVelocities();
    *m_fullStepVelocities = *rhs->getFullStepVelocities();
    *m_BDPositions       = *rhs->getBoundaryParticlePositions();
    *m_Densities         = *rhs->getDensities();
    *m_Normals           = *rhs->getNormals();
    *m_Accels            = *rhs->getAccelerations();
    *m_DiffuseVelocities = *rhs->getDiffuseVelocities();

    m_NeighborLists   = rhs->getFluidNeighborLists();
    m_BDNeighborLists = rhs->getBoundaryNeighborLists();
    m_NeighborInfo    = rhs->getNeighborInfo();

    m_positions->postModified();
}

size_t
SPHState::getNumParticles() const
{
    return m_positions->size();
}
} // end namespace imstk
