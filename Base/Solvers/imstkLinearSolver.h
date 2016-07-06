// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef imstkLinearSolver_h
#define imstkLinearSolver_h

// imstk includes
#include "imstkLinearSystem.h"
#include "imstkMath.h"

#include "g3log/g3log.hpp"

namespace imstk
{

///
/// \brief Base class for linear solvers
///
template<typename SystemMatrixType>
class LinearSolver
{
public:
    using MatrixType = SystemMatrixType;
    using LinearSystemType = LinearSystem < MatrixType > ;

    enum class Type
    {
        ConjugateGradient,
        LUFactorization,
        GaussSeidel,
        SuccessiveOverRelaxation,
        Jacobi,
        GMRES,
        none
    };

public:
    ///
    /// \brief Default constructor/destructor
    ///
    LinearSolver() : m_linearSystem(nullptr){};
    virtual ~LinearSolver(){};

    ///
    /// \brief Main solve routine
    ///
    virtual void solve(Vectord& x) = 0;

    ///
    /// \brief Set/get the system. Replaces/Returns the stored linear system of equations.
    ///
    virtual void setSystem(std::shared_ptr<LinearSystemType> newSystem)
    {
        m_linearSystem.reset();
        m_linearSystem = newSystem;
    }

    std::shared_ptr<LinearSystemType> getSystem() const
    {
        return m_linearSystem;
    }

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance)
    {
        m_tolerance = tolerance;
    }

    ///
    /// \brief Get solver tolerance
    ///
    double getTolerance() const
    {
        return m_tolerance;
    }

    ///
    /// \brief Print solver information.
    ///
    virtual void print() const
    {
        LOG(INFO) << "Solver type (broad): Linear";
    };

    ///
    /// \brief Returns true if the solver is iterative
    ///
    virtual bool isIterative() const = 0;

protected:
    Type m_Type;                                        ///> Type of the scene object
    double m_tolerance = MACHINE_PRECISION;             ///> default tolerance
    std::shared_ptr<LinearSystemType> m_linearSystem;   /// Linear system of equations
};

}
#endif // imstkLinearSolver_h
