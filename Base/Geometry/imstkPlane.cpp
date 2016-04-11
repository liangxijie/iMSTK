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

#include "imstkPlane.h"

namespace imstk {
Vec3d
Plane::getNormal() const
{
    return m_orientation._transformVector(UP_VECTOR);
}

void
Plane::setNormal(const Vec3d& normal)
{
    this->setOrientation(Quatd::FromTwoVectors(UP_VECTOR, normal));
}

const double&
Plane::getWidth() const
{
    return m_width;
}

void
Plane::setWidth(const double& width)
{
    m_width = width;
}

double
Plane::getVolume() const
{
    return 0.0;
}
}
