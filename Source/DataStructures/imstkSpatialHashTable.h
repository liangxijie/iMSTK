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

namespace imstk
{
///
/// \class SpatialHashTable
///
/// \brief Abstract class for spatial hash tables
///
class SpatialHashTable
{
public:
    ///
    /// \brief Protected constructor
    /// \param x,y,z Dimensions for each cell
    ///
    virtual void setCellSize(double x, double y, double z) = 0;

protected:
    ///
    /// \brief Rehash the hash table
    ///
    virtual void rehash() = 0;

    ///
    /// \brief Protected constructor
    ///
    SpatialHashTable();

    double m_cellSize[3];
};
}
