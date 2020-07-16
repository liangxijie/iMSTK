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

#include "imstkSimulationManager.h"
#include "imstkSPHObject.h"
#include "imstkSPHModel.h"
#include "imstkPointSet.h"
#include "imstkScene.h"

using namespace imstk;

///
/// \brief Generate a sphere-shape fluid object
///
StdVectorOfVec3d
generateSphereShapeFluid(const double particleRadius)
{
    const double sphereRadius = 2.0;
    const Vec3d  sphereCenter(0, 1, 0);

    const auto  sphereRadiusSqr = sphereRadius * sphereRadius;
    const auto  spacing         = 2.0 * particleRadius;
    const auto  N               = static_cast<size_t>(2.0 * sphereRadius / spacing);              // Maximum number of particles in each dimension
    const Vec3d lcorner         = sphereCenter - Vec3d(sphereRadius, sphereRadius, sphereRadius); // Cannot use auto here, due to Eigen bug

    StdVectorOfVec3d particles;
    particles.reserve(N * N * N);

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            for (size_t k = 0; k < N; ++k)
            {
                Vec3d ppos = lcorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
                Vec3d cx   = ppos - sphereCenter;
                if (cx.squaredNorm() < sphereRadiusSqr)
                {
                    particles.push_back(ppos);
                }
            }
        }
    }

    return particles;
}

///
/// \brief Generate a box-shape fluid object
///
StdVectorOfVec3d
generateBoxShapeFluid(const double particleRadius)
{
    const double boxWidth = 4.0;
    const Vec3d  boxLowerCorner(-2, -3, -2);

    const auto spacing = 2.0 * particleRadius;
    const auto N       = static_cast<size_t>(boxWidth / spacing);

    StdVectorOfVec3d particles;
    particles.reserve(N * N * N);

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            for (size_t k = 0; k < N; ++k)
            {
                Vec3d ppos = boxLowerCorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
                particles.push_back(ppos);
            }
        }
    }

    return particles;
}

#if SCENE_ID == 3
StdVectorOfVec3d getBunny(); // Defined in Bunny.cpp
#endif
///
/// \brief Generate a bunny-shape fluid object
///
StdVectorOfVec3d
generateBunnyShapeFluid(const double particleRadius)
{
    LOG_IF(FATAL, (std::abs(particleRadius - 0.08) > 1e-6)) << "Particle radius for this scene must be 0.08";
    StdVectorOfVec3d particles;
#if SCENE_ID == 3
    particles = getBunny();
#endif
    return particles;
}

///
/// \brief Generate fluid for pipe flow
///
StdVectorOfVec3d
generatePipeFluid(const double particleRadius)
{
  const double pipeRadius = 1.0;
  const double pipeLength = 5.0;
  const Vec3d  lcorner(-5, 5, 0);
  const Vec3d pipeLeftCenter = lcorner + Vec3d(0, pipeRadius, pipeRadius);

  const auto spacing = 2.0 * particleRadius;
  const auto N_width = static_cast<size_t>(2.0 * pipeRadius / spacing); // Maximum number of particles in width dimension
  const auto N_length = static_cast<size_t>(pipeLength / spacing); // Maximum number of particles in length dimension

  StdVectorOfVec3d particles;
  particles.reserve(N_width * N_width * N_length);

  for (size_t i = 0; i < N_length; ++i)
  {
    for (size_t j = 0; j < N_width; ++j)
    {
      for (size_t k = 0; k < N_width; ++k)
      {
        Vec3d ppos = lcorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
        //const double cx = ppos.x() - pipeBottomCenter.x();
        //const double cy = ppos.y() - pipeBottomCenter.y();
        Vec3d cx = ppos - Vec3d(spacing * double(i), 0, 0) - pipeLeftCenter;
        if (cx.squaredNorm() < pipeRadius)
        {
          particles.push_back(ppos);
        }
      }
    }
  }

   return particles;
}

StdVectorOfVec3d
initializeNonZeroVelocities(const size_t numParticles)
{
  StdVectorOfVec3d initialVelocities(numParticles, Vec3d(10, 0, 0));
  return initialVelocities;
}

std::shared_ptr<SPHObject>
generateFluid(const std::shared_ptr<Scene>& scene, const double particleRadius)
{
    StdVectorOfVec3d particles;
    StdVectorOfVec3d initialVelocities;
    switch (SCENE_ID)
    {
    case 1:
        particles = generateSphereShapeFluid(particleRadius);
        initialVelocities.resize(particles.size());
        break;
    case 2:
        particles = generateBoxShapeFluid(particleRadius);
        initialVelocities.resize(particles.size());
        break;
    case 3:
        particles = generateBunnyShapeFluid(particleRadius);
        initialVelocities.resize(particles.size());
        break;
    case 4:
      particles = generatePipeFluid(particleRadius);
      initialVelocities = initializeNonZeroVelocities(particles.size());
      break;
    default:
        LOG(FATAL) << "Invalid scene index";
    }

    LOG(INFO) << "Number of particles: " << particles.size();

    // Create a geometry object
    auto fluidGeometry = std::make_shared<PointSet>();
    fluidGeometry->initialize(particles);

    // Create a fluids object
    auto fluidObj = std::make_shared<SPHObject>("Sphere");

    // Create a visual model
    auto fluidVisualModel = std::make_shared<VisualModel>(fluidGeometry);
    auto fluidMaterial    = std::make_shared<RenderMaterial>();
    fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Points);
    fluidMaterial->setVertexColor(Color::Orange);
    fluidMaterial->setPointSize(10.);
    fluidVisualModel->setRenderMaterial(fluidMaterial);

    // Create a physics model
    auto sphModel = std::make_shared<SPHModel>();
    sphModel->setModelGeometry(fluidGeometry);

    sphModel->setInitialVelocities(initialVelocities);

    // configure model
    auto sphParams = std::make_shared<SPHModelConfig>(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    if (SCENE_ID == 2)   // highly viscous fluid
    {
        sphParams->m_kernelOverParticleRadiusRatio = 6.0;
        sphParams->m_dynamicViscosityCoeff         = 0.5;
        sphParams->m_surfaceTensionStiffness       = 5.0;
    }

    if (SCENE_ID == 3)   // bunny-shaped fluid
    {
        sphParams->m_frictionBoundary= 0.3;
    }

    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Add the component models
    fluidObj->addVisualModel(fluidVisualModel);
    fluidObj->setCollidingGeometry(fluidGeometry);
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->setPhysicsGeometry(fluidGeometry); // TODO: Look into API duplication and resulting conflicts
    scene->addSceneObject(fluidObj);

    return fluidObj;
}
