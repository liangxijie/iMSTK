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

#include "imstkDynamicalModel.h"
#include "imstkSPHState.h"
#include "imstkSPHKernels.h"
#include "imstkNeighborSearch.h"

#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"

#include "imstkTetrahedralMesh.h"


namespace imstk
{
class ComputeNode;
class PointSet;

///
/// \class SPHModelConfig
/// \brief Class that holds the SPH model parameters
///
class SPHModelConfig
{
private:
    void initialize();

public:
    explicit SPHModelConfig(const Real particleRadius);

    /// \todo Move this to solver or time integrator in the future
    Real m_minTimestep = Real(1e-6);
    Real m_maxTimestep = Real(1e-3);
    Real m_CFLFactor   = Real(1.0);

    // particle parameters
    Real m_particleRadius    = Real(0);
    Real m_particleRadiusSqr = Real(0); ///> \note derived quantity

    // material parameters
    Real m_restDensity       = Real(1000.0);
    Real m_restDensitySqr    = Real(1000000.0);    ///> \note derived quantity
    Real m_restDensityInv    = Real(1.0 / 1000.0); ///> \note derived quantity
    Real m_particleMass      = Real(1);
    Real m_particleMassScale = Real(1.06);         ///> scale particle mass to a smaller value to maintain stability

    bool m_bNormalizeDensity    = false;
    bool m_bDensityWithBoundary = false;

    // pressure
    Real m_pressureStiffness = Real(50000.0);

    // viscosity and surface tension/cohesion
    Real m_dynamicViscosityCoeff   = Real(1e-2);
    Real m_viscosityBoundary       = Real(1e-5);
    Real m_surfaceTensionStiffness = Real(1);
    Real m_frictionBoundary        = Real(0.1);

    // kernel properties
    Real m_kernelOverParticleRadiusRatio = Real(4.0);
    Real m_kernelRadius;    ///> \note derived quantity
    Real m_kernelRadiusSqr; ///> \note derived quantity

    // gravity
    Vec3r m_gravity = Vec3r(0, -9.81, 0);

    // neighbor search
    NeighborSearch::Method m_NeighborSearchMethod = NeighborSearch::Method::UniformGridBasedSearch;
};

///
/// \class SPHModel
/// \brief SPH fluid model
///
class SPHModel : public DynamicalModel<SPHKinematicState>
{
public:
    ///
    /// \brief Constructor
    ///
    SPHModel();

    ///
    /// \brief Destructor
    ///
    virtual ~SPHModel() override = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<SPHModelConfig>& params) { m_modelParameters = params; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Update states
    ///
    virtual void updateBodyStates(const Vectord&, const StateUpdateType) override {}

    ///
    /// \brief Update positions of point set geometry
    ///
    virtual void updatePhysicsGeometry() override;

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState() override
    { this->m_currentState->setState(this->m_initialState); }

    ///
    /// \brief Get the simulation parameters
    ///
    const std::shared_ptr<SPHModelConfig>& getParameters() const
    { assert(m_modelParameters); return m_modelParameters; }

    ///
    /// \brief Get the kinematics particle data (positions + velocities)
    ///
    SPHKinematicState& getKinematicsState()
    {
        assert(this->m_currentState);
        return *this->m_currentState;
    }

    const SPHKinematicState& getKinematicsState() const
    {
        assert(this->m_currentState);
        return *this->m_currentState;
    }

    ///
    /// \brief Get particle simulation data
    ///
    SPHSimulationState& getState() { return m_simulationState; }
    const SPHSimulationState& getState() const { return m_simulationState; }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    virtual void setTimeStep(const double timeStep) override
    { setDefaultTimeStep(static_cast<Real>(timeStep)); }

    ///
    /// \brief Set the default time step size,
    /// valid only if using a fixed time step for integration
    ///
    void setDefaultTimeStep(const Real timeStep)
    { m_defaultDt = static_cast<Real>(timeStep); }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override
    { return static_cast<double>(m_dt); }

    void setInitialVelocities(StdVectorOfVec3d& initialVelocities);
    StdVectorOfVec3d getInitialVelocities();

    Real particlePressure(const double density);
    void periodicBCs(const size_t p);

    void setWallPointIndices(std::vector<size_t>& wallPointIndices) { m_wallPointIndices = wallPointIndices; }
    std::vector<size_t>& getWallPointIndices() { return m_wallPointIndices; }

    void setBufferParticleIndices(std::vector<size_t>& bufferParticleIndices) { m_bufferParticleIndices = bufferParticleIndices; }
    std::vector<size_t>& getBufferParticleIndices() { return m_bufferParticleIndices; }

    void setMinMaxXCoords(const double minXCoord, const double maxXCoord) { m_minXCoord = minXCoord; m_maxXCoord = maxXCoord; }

    void writeStateToCSV();
    void setWriteToOutputModulo(const Real modulo) { m_writeToOutputModulo = modulo; }
    Real getTotalTime() const { return m_totalTime; }
    int getTimeStepCount() const { return m_timeStepCount; }
    void writeStateToVtk();
    void setGeometryMesh(std::shared_ptr<TetrahedralMesh>& geometryMesh) { m_geomUnstructuredGrid = geometryMesh; }
    size_t findNearestParticleToVertex(const Vec3d point);

    void setInletDensity(const double inletDensity) { m_inletDensity = inletDensity; }
    const double getInletDensity() { return m_inletDensity; }
    void setOutletDensity(const double outletDensity) { m_outletDensity = outletDensity; }
    const double getOutletDensity() { return m_outletDensity; }

    void setInletVelocity(const Vec3d inletVelocity) { m_inletVelocity = inletVelocity; }
    const Vec3d getInletVelocity() { return m_inletVelocity; }

    void setOutletVelocity(const Vec3d outletVelocity) { m_outletVelocity = outletVelocity; }

    void setInletRegionXCoord(const double inletRegionXCoord) { m_inletRegionXCoord = inletRegionXCoord; }
    void setOutletRegionXCoord(const double outletRegionXCoord) { m_outletRegionXCoord = outletRegionXCoord; }
    double getInletRegionXCoord() { return m_inletRegionXCoord; }
    double getOutletRegionXCoord() { return m_outletRegionXCoord; }
    double setInletOutletRegionYCoordDivision(const double inletOutletRegionYCoordDivision) { m_inletOutletRegionYCoordDivision = inletOutletRegionYCoordDivision; }


    void printParticleTypes();

    void setInletRadius(const double inletRadius) { m_inletRadius = inletRadius; }
    void setInletCenterPoint(const Vec3d inletCenterPoint) { m_inletCenterPoint = inletCenterPoint; }
    void setBufferXCoord(const double bufferXCoord) { m_bufferXCoord = bufferXCoord; }

    Vec3d computeParabolicInletVelocity(const Vec3d particlePosition);



    std::shared_ptr<TaskNode> getFindParticleNeighborsNode() const { return m_findParticleNeighborsNode; }
    std::shared_ptr<TaskNode> getComputeDensityNode() const { return m_computeDensityNode; }
    std::shared_ptr<TaskNode> getComputePressureNode() const { return m_computePressureAccelNode; }
    std::shared_ptr<TaskNode> getComputeSurfaceTensionNode() const { return m_computeSurfaceTensionNode; }
    std::shared_ptr<TaskNode> getComputeTimeStepSizeNode() const { m_computeTimeStepSizeNode; }
    std::shared_ptr<TaskNode> getSumAccelsNode() const { m_sumAccelsNode; }
    std::shared_ptr<TaskNode> getIntegrateNode() const { return m_integrateNode; }
    std::shared_ptr<TaskNode> getComputeViscosityNode() const { return m_computeViscosityNode; }
    std::shared_ptr<TaskNode> getUpdateVelocityNode() const { return m_updateVelocityNoGravityNode; }
    std::shared_ptr<TaskNode> getMoveParticlesNode() const { return m_moveParticlesNode; }

protected:
    ///
    /// \brief Setup SPH compute graph connectivity
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

private:
    ///
    /// \brief Compute time step size, do nothing if using a fixed time step size for integration
    ///
    void computeTimeStepSize();

    ///
    /// \brief Compute time step size based on CFL condition
    ///
    Real computeCFLTimeStepSize();

    ///
    /// \brief Find the neighbors for each particle
    ///
    void findParticleNeighbors();

    ///
    /// \brief Pre-compute relative positions with neighbor particles
    ///
    void computeNeighborRelativePositions();

    ///
    /// \brief Collect the densities of neighbor particles,
    /// called after all density computation (after density normalization, if applicable)
    ///
    void collectNeighborDensity();

    ///
    /// \brief Compute particle densities
    ///
    void computeDensity();

    ///
    /// \brief Normalize densities, producing smoother density field
    ///
    void normalizeDensity();

    ///
    /// \brief Compute particle acceleration due to pressure
    ///
    void computePressureAcceleration();

    ///
    /// \brief Sum the forces computed in parallel
    ///
    void sumAccels();

    ///
    /// \brief Update particle velocities due to pressure
    ///
    void updateVelocity(const Real timestep);

    void updateVelocityNoGravity(Real timestep);

    ///
    /// \brief Compute viscosity
    ///
    void computeViscosity();

    ///
    /// \brief Compute surface tension and update velocities
    /// Compute surface tension using Akinci et at. 2013 model
    /// (Versatile Surface Tension and Adhesion for SPH Fluids)
    ///
    void computeSurfaceTension();

    ///
    /// \brief Move particles
    ///
    void moveParticles(const Real timestep);

    void computePressureOutlet();


protected:
    std::shared_ptr<TaskNode> m_findParticleNeighborsNode = nullptr;
    std::shared_ptr<TaskNode> m_computeDensityNode        = nullptr;
    std::shared_ptr<TaskNode> m_computePressureAccelNode  = nullptr;
    std::shared_ptr<TaskNode> m_computeSurfaceTensionNode = nullptr;
    std::shared_ptr<TaskNode> m_computeTimeStepSizeNode   = nullptr;
    std::shared_ptr<TaskNode> m_sumAccelsNode = nullptr;
    std::shared_ptr<TaskNode> m_integrateNode = nullptr;
    std::shared_ptr<TaskNode> m_updateVelocityNoGravityNode = nullptr;
    std::shared_ptr<TaskNode> m_computeViscosityNode = nullptr;
    std::shared_ptr<TaskNode> m_moveParticlesNode = nullptr;


    std::shared_ptr<TaskNode> m_normalizeDensityNode = nullptr;
    std::shared_ptr<TaskNode> m_collectNeighborDensityNode = nullptr;


private:
    std::shared_ptr<PointSet> m_pointSetGeometry;
    SPHSimulationState m_simulationState;

    Real m_dt;                                          ///> time step size
    Real m_defaultDt;                                   ///> default time step size

    SPHSimulationKernels m_kernels;                     ///> SPH kernels (must be initialized during model initialization)
    std::shared_ptr<SPHModelConfig> m_modelParameters;  ///> SPH Model parameters (must be set before simulation)
    std::shared_ptr<NeighborSearch> m_neighborSearcher; ///> Neighbor Search (must be initialized during model initialization)

    std::shared_ptr<StdVectorOfVec3d> m_pressureAccels       = nullptr;
    std::shared_ptr<StdVectorOfVec3d> m_surfaceTensionAccels = nullptr;

    StdVectorOfVec3d m_initialVelocities;
    StdVectorOfReal m_initialDensities;

    std::vector<size_t> m_wallPointIndices;
    std::vector<size_t> m_bufferParticleIndices;

    double m_minXCoord;
    double m_maxXCoord;

    double m_inletRegionXCoord;
    double m_outletRegionXCoord;

    double m_inletDensity;
    double m_outletDensity;
    Vec3d m_inletVelocity;
    Vec3d m_outletVelocity;

    double m_totalTime;
    double m_writeToOutputModulo;
    int m_timeStepCount;
    double m_bufferXCoord;

    double m_inletRadius;
    Vec3d m_inletCenterPoint;
    double m_previousTime;
    double m_timeModulo;
    double m_speedOfSound;
    double m_beta;
    double m_inletOutletRegionYCoordDivision;

    std::shared_ptr<TetrahedralMesh> m_geomUnstructuredGrid = nullptr;


};
} // end namespace imstk
