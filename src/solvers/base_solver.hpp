// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#ifndef BASE_SOLVER
#define BASE_SOLVER

#include <map>
#include <memory>
#include <optional>

#include "common/serac_types.hpp"
#include "mfem.hpp"

namespace serac {

/**
 * Wraps a (currently iterative) system solver and handles the configuration of linear
 * or nonlinear solvers
 */
class SystemSolver {
 public:
  // TODO: Eliminate this once a dependency injection approach is used for the solvers
  SystemSolver() = default;
  /**
   * Constructs a new solver wrapper
   * @param[in] comm The MPI communicator object
   * @param[in] lin_params The parameters for the linear solver
   * @param[in] nonlin_params The optional parameters for the optional nonlinear solver
   * @see serac::LinearSolverParameters
   * @see serac::NonlinearSolverParameters
   */
  SystemSolver(MPI_Comm comm, const LinearSolverParameters& lin_params,
               const std::optional<NonlinearSolverParameters>& nonlin_params = std::nullopt);

  /**
   * Sets a preconditioner for the underlying linear solver object
   * @param[in] prec The preconditioner, of which the object takes ownership
   * @note The preconditioner must be moved into the call
   * @code(.cpp)
   * solver.setPreconditioner(std::move(prec));
   * @endcode
   */
  void setPreconditioner(std::unique_ptr<mfem::Solver>&& prec)
  {
    prec_ = std::move(prec);
    iter_lin_solver_->SetPreconditioner(*prec_);
  }

  /**
   * Returns the underlying solver object
   * @return The underlying nonlinear solver, if one was configured
   * when the object was constructed, otherwise, the underlying linear solver
   */
  mfem::IterativeSolver& solver() { return (nonlin_solver_) ? **nonlin_solver_ : *iter_lin_solver_; }

 private:
  std::unique_ptr<mfem::IterativeSolver>                iter_lin_solver_;
  std::optional<std::unique_ptr<mfem::IterativeSolver>> nonlin_solver_;
  std::unique_ptr<mfem::Solver>                         prec_;
};

/// This is the abstract base class for a generic forward solver
class BaseSolver {
 protected:
  /// The MPI communicator
  MPI_Comm comm_;

  /// List of finite element data structures
  std::vector<std::shared_ptr<serac::FiniteElementState>> state_;

  /// Block vector storage of the true state
  std::unique_ptr<mfem::BlockVector> block_;

  /// Essential BC markers
  std::vector<serac::BoundaryCondition> ess_bdr_;

  /// Natural BC markers
  std::vector<serac::BoundaryCondition> nat_bdr_;

  /// Type of state variable output
  serac::OutputType output_type_;

  /// Time integration method
  serac::TimestepMethod timestepper_;

  /// MFEM ode solver object
  std::unique_ptr<mfem::ODESolver> ode_solver_;

  /// Root output name
  std::string root_name_;

  /// Current time
  double time_;

  /// Current cycle
  int cycle_;

  /// MPI rank
  int mpi_rank_;

  /// MPI size
  int mpi_size_;

  /// Order of basis functions
  int order_;

  /// VisIt data collection pointer
  std::unique_ptr<mfem::VisItDataCollection> visit_dc_;

  /// State variable initialization indicator
  std::vector<bool> gf_initialized_;

  SystemSolver solver_;

 public:
  /// Empty constructor
  BaseSolver(MPI_Comm comm);

  /// Constructor that creates n entries in state_ of order p
  BaseSolver(MPI_Comm comm, int n, int p);

  /// Set the essential boundary conditions from a list of boundary markers and
  /// a coefficient
  virtual void setEssentialBCs(const std::set<int>& ess_bdr, serac::BoundaryCondition::Coef ess_bdr_coef,
                               const mfem::ParFiniteElementSpace& fes, const int component = -1);

  /// Set a list of true degrees of freedom from a coefficient
  virtual void setTrueDofs(const mfem::Array<int>& true_dofs, serac::BoundaryCondition::Coef ess_bdr_coef,
                           const int component = -1);

  /// Set the natural boundary conditions from a list of boundary markers and a
  /// coefficient
  virtual void setNaturalBCs(const std::set<int>& nat_bdr, serac::BoundaryCondition::Coef nat_bdr_coef,
                             const int component = -1);

  /// Set the state variables from a coefficient
  virtual void setState(const std::vector<serac::BoundaryCondition::Coef>& state_coef);

  /// Set the state variables from an existing grid function
  virtual void setState(const std::vector<std::shared_ptr<serac::FiniteElementState>>& state);

  /// Get the list of state variable grid functions
  virtual std::vector<std::shared_ptr<serac::FiniteElementState>> getState() const;

  /// Set the time integration method
  virtual void setTimestepper(const serac::TimestepMethod timestepper);

  /// Set the current time
  virtual void setTime(const double time);

  /// Get the current time
  virtual double time() const;

  /// Get the current cycle
  virtual int cycle() const;

  /// Complete the setup and allocate the necessary data structures
  virtual void completeSetup() = 0;

  /// Advance the state variables according to the chosen time integrator
  virtual void advanceTimestep(double& dt) = 0;

  /// Initialize the state variable output
  virtual void initializeOutput(const serac::OutputType output_type, const std::string& root_name);

  /// output the state variables
  virtual void outputState() const;

  /// Destructor
  virtual ~BaseSolver() = default;
};

}  // namespace serac

#endif
