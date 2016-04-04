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

#include "imstkGeometryMap.h"

namespace imstk {
    void GeometryMap::muteMap()
    {
        m_isActive = false;
    }

    void GeometryMap::activateMap()
    {
        m_isActive = true;
    }

    const GeometryMapType& GeometryMap::getType() const
    {
        return m_type;
    }

    void GeometryMap::setMaster(std::shared_ptr<Geometry> master)
    {
        m_master = master;
    }

    std::shared_ptr<Geometry> GeometryMap::getMaster() const
    {
        return m_master;
    }

    void GeometryMap::setSlave(std::shared_ptr<Geometry> slave)
    {
        m_slave = slave;
    }

    std::shared_ptr<Geometry> GeometryMap::getSlave() const
    {
        return m_slave;
    }

    bool GeometryMap::isActive() const
    {
        return m_isActive;
    }
}