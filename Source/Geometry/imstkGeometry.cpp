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

#include "imstkGeometry.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"

namespace imstk
{
Geometry::Geometry(const std::string& name) :
    m_name(name), m_geometryIndex(Geometry::getUniqueID()), m_transform(Mat4d::Identity())
{
    // If the geometry name is empty, enumerate it by name (which will not be duplicated)
    if (m_name.empty())
    {
        m_name = std::string("unnamed_geometry-") + std::to_string(m_geometryIndex);
    }
}

void
Geometry::print() const
{
    LOG(INFO) << getTypeName();
    LOG(INFO) << "Transform: " << m_transform;
}

void
Geometry::computeBoundingBox(Vec3d& imstkNotUsed(min), Vec3d& imstkNotUsed(max), const double imstkNotUsed(padding))
{
    LOG(WARNING) << "computeBoundingBox() must be called from an instance of a specific geometry class";
}

void
Geometry::translate(const Vec3d& t, TransformType type)
{
    if (t.isApprox(Vec3d::Zero()))
    {
        return;
    }

    transform(AffineTransform3d(Eigen::Translation3d(t)).matrix(), type);
}

void
Geometry::translate(double x, double y, double z, TransformType type)
{
    this->translate(Vec3d(x, y, z), type);
}

void
Geometry::rotate(const Mat3d& r, TransformType type)
{
    if (r.isApprox(Mat3d::Identity()))
    {
        return;
    }

    Mat4d m = Mat4d::Identity();
    m.block<3, 3>(0, 0) = r;
    transform(m, type);
}

void
Geometry::rotate(const Quatd& q, TransformType type)
{
    this->rotate(q.toRotationMatrix(), type);
}

void
Geometry::rotate(const Vec3d& axis, double radians, TransformType type)
{
    this->rotate(Rotd(radians, axis.normalized()).toRotationMatrix(), type);
}

void
Geometry::scale(const Vec3d& s, TransformType type)
{
    Mat4d m = Mat4d::Identity();
    m(0, 0) = s[0];
    m(1, 1) = s[1];
    m(2, 2) = s[2];
    transform(m, type);
}

void
Geometry::scale(const double s, TransformType type)
{
    Mat4d m = Mat4d::Identity();
    m(0, 0) = s;
    m(1, 1) = s;
    m(2, 2) = s;
    transform(m, type);
}

void
Geometry::transform(const Mat4d& T, TransformType type)
{
    if (type == TransformType::ConcatenateToTransform)
    {
        m_transform = T * m_transform;
    }
    else
    {
        applyTransform(T);
        this->postModified();
    }
    m_transformApplied = false;
}

Vec3d
Geometry::getTranslation() const
{
    return m_transform.block<3, 1>(0, 3);
}

void
Geometry::setTranslation(const Vec3d& t)
{
    m_transform.block<3, 1>(0, 3) = t;
    m_transformApplied = false;
}

void
Geometry::setTranslation(const double x, const double y, const double z)
{
    this->setTranslation(Vec3d(x, y, z));
}

void
Geometry::setRotation(const Mat3d& m)
{
    // Decompose trs, getRotation assumes no shear
    const Vec3d s = getScaling();
    const Vec3d t = getTranslation();
    m_transform = Mat4d::Identity();
    m_transform.block<3, 3>(0, 0) = m;
    m_transform.block<3, 1>(0, 3) = t;
    m_transform(0, 0) *= s[0];
    m_transform(1, 1) *= s[1];
    m_transform(2, 2) *= s[2];
    m_transformApplied = false;
}

void
Geometry::setRotation(const Quatd& q)
{
    this->setRotation(q.toRotationMatrix());
}

void
Geometry::setRotation(const Vec3d& axis, const double angle)
{
    this->setRotation(Rotd(angle, axis).toRotationMatrix());
}

void
Geometry::setScaling(const Vec3d& s)
{
    // Applying 0 scales will destroy the basis, would need another transform
    if (s == Vec3d::Zero())
    {
        LOG(WARNING) << "Cannot apply 0 scales";
        return;
    }
    m_transform.block<3, 1>(0, 0) = m_transform.block<3, 1>(0, 0).normalized() * s[0];
    m_transform.block<3, 1>(0, 1) = m_transform.block<3, 1>(0, 1).normalized() * s[1];
    m_transform.block<3, 1>(0, 2) = m_transform.block<3, 1>(0, 2).normalized() * s[2];
    m_transformApplied = false;
}

void
Geometry::setScaling(const double s)
{
    setScaling(Vec3d(s, s, s));
}

Mat3d
Geometry::getRotation() const
{
    // Assumes affine, no shear
    const Vec3d& x = m_transform.block<3, 1>(0, 0);
    const Vec3d& y = m_transform.block<3, 1>(0, 1);
    const Vec3d& z = m_transform.block<3, 1>(0, 2);

    Mat3d r;
    r.block<3, 1>(0, 0) = x.normalized();
    r.block<3, 1>(0, 1) = y.normalized();
    r.block<3, 1>(0, 2) = z.normalized();

    return r;
}

Vec3d
Geometry::getScaling() const
{
    return Vec3d(
        m_transform.block<3, 1>(0, 0).norm(),
        m_transform.block<3, 1>(0, 1).norm(),
        m_transform.block<3, 1>(0, 2).norm());
}

// Static mutex lock
ParallelUtils::SpinLock
Geometry::s_GeomGlobalLock;

// Static counter
uint32_t
Geometry::s_NumGeneratedGegometries = 0;

uint32_t
Geometry::getUniqueID()
{
    s_GeomGlobalLock.lock();
    const auto idx = s_NumGeneratedGegometries;
    ++s_NumGeneratedGegometries;
    s_GeomGlobalLock.unlock();
    return idx;
}
} // imstk
