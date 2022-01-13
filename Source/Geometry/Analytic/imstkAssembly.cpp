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

#include "imstkAssembly.h"
#include "imstkLogger.h"

namespace imstk
{
void
Assembly::print() const
{
    LOG(INFO) << "Assembly: " << 00000;
}

void
Assembly::applyTransform(const Mat4d& m)
{
    /*const Vec3d s = Vec3d(
        m.block<3, 1>(0, 0).norm(),
        m.block<3, 1>(0, 1).norm(),
        m.block<3, 1>(0, 2).norm());*/
}

void
Assembly::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();
}

void
Assembly::setJoint(const double *v)
{
    for(int i=0; i<6; i++) angle[i] = v[i];
    m_transformApplied = false;
    this->postModified();
}
void
Assembly::getJoint(double *v)
{
    for(int i=0; i<6; i++) v[i] = angle[i];
}

} // imstk
