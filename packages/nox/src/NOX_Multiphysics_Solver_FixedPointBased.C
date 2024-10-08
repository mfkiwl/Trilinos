// @HEADER
// *****************************************************************************
//            NOX: An Object-Oriented Nonlinear Solver Package
//
// Copyright 2002 NTESS and the NOX contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include "NOX_Multiphysics_Solver_FixedPointBased.H"    // class definition
#include "NOX_Abstract_Vector.H"
#include "NOX_Abstract_Group.H"
#include "NOX_Common.H"
#include "NOX_Utils.H"
#include "NOX_GlobalData.H"
#include "NOX_LineSearch_Generic.H"
#include "NOX_LineSearch_Factory.H"
#include "NOX_Direction_Generic.H"
#include "NOX_Direction_Factory.H"
#include "NOX_Observer.hpp"
#include "NOX_Solver_SolverUtils.H"

NOX::Multiphysics::Solver::FixedPointBased::
FixedPointBased(const Teuchos::RCP<std::vector<Teuchos::RCP<NOX::Solver::Generic> > >& solvers,
        const Teuchos::RCP<NOX::Multiphysics::DataExchange::Interface>& i,
        const Teuchos::RCP<NOX::StatusTest::Generic>& t,
        const Teuchos::RCP<Teuchos::ParameterList>& p) :
  solveType( JACOBI ),
  solversVecPtr(solvers),
  dataExInterface(i),
  globalDataPtr(Teuchos::rcp(new NOX::GlobalData(p))),
  utilsPtr(globalDataPtr->getUtils()),
  solnPtr( Teuchos::rcp(new Group(solvers, t, p)) ),
  testPtr(t),
  paramsPtr(p)
{
  NOX::Solver::validateSolverOptionsSublist(paramsPtr->sublist("Solver Options"));
  observer = NOX::Solver::parseObserver(p->sublist("Solver Options"));
  init();
}


// Protected
void
NOX::Multiphysics::Solver::FixedPointBased::init()
{
  // Initialize
  nIter = 0;
  status = NOX::StatusTest::Unconverged;

  // Get the checktype
  //   Python interface can't create enumerated types in a python
  //   generated teuchos parameter list, so we need to convert int
  //   values to enum if they exist parameter list.
  if (Teuchos::isParameterType<int>(*paramsPtr, "Status Test Check Type")) {
    checkType = static_cast<NOX::StatusTest::CheckType>
      (paramsPtr->sublist("Solver Options").get("Status Test Check Type",
        int(0)));
  }
  else {
    checkType = static_cast<NOX::StatusTest::CheckType>
      (paramsPtr->sublist("Solver Options").get("Status Test Check Type",
                        NOX::StatusTest::Minimal));
  }

  // Get the type of fixed-point solve
  std::string solveTypeName =  paramsPtr->sublist("Solver Options").get( "Fixed Point Iteration Type", "Seidel" );
  if( "Jacobi" == solveTypeName )
    solveType = JACOBI;
  else if( "Seidel" == solveTypeName )
    solveType = SEIDEL;
  else
  {
    utilsPtr->out() << "NOX::Multiphysics::Solver::FixedPointBased::step - "
                    << "Invalid Solver Method " << solveTypeName << std::endl;
    throw std::runtime_error("NOX Error");
  }

  // Print out parameters
  if (utilsPtr->isPrintType(NOX::Utils::Parameters))
  {
    utilsPtr->out() << "\n" << NOX::Utils::fill(72) << "\n";
    utilsPtr->out() << "\n-- Parameters Passed to Fixed-Point Coupling Solver --\n\n";
    paramsPtr->print(utilsPtr->out(),5);
  }

}

bool
NOX::Multiphysics::Solver::FixedPointBased::reset(
      const Teuchos::RCP<std::vector<Teuchos::RCP<NOX::Solver::Generic> > >& solvers,
      const Teuchos::RCP<NOX::Multiphysics::DataExchange::Interface>& /* i */,
      const Teuchos::RCP<NOX::StatusTest::Generic>& t,
      const Teuchos::RCP<Teuchos::ParameterList>& p)
{
  solversVecPtr = solvers;
  globalDataPtr = Teuchos::rcp(new NOX::GlobalData(p));
  solnPtr = Teuchos::rcp( new NOX::Multiphysics::Group(solvers, t, p) );
  testPtr = t;
  paramsPtr = p;
  utilsPtr = globalDataPtr->getUtils();
  NOX::Solver::validateSolverOptionsSublist(paramsPtr->sublist("Solver Options"));
  observer = NOX::Solver::parseObserver(p->sublist("Solver Options"));

  init();

  return false;
}

void
NOX::Multiphysics::Solver::FixedPointBased::reset(
      const NOX::Abstract::Vector& /* initialGuess */,
      const Teuchos::RCP<NOX::StatusTest::Generic>& /* t */)
{
  std::string msg = "Error - NOX::Multiphysics::Solver::FixedPointBased::reset() - this reset method is not valid for a Multiphysics Solver!";
  TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, msg);
}

void
NOX::Multiphysics::Solver::FixedPointBased::reset(
      const NOX::Abstract::Vector& /* initialGuess */)
{
  std::string msg = "Error - NOX::Multiphysics::Solver::FixedPointBased::reset() - this reset method is not valid for a Multiphysics Solver!";
  TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, msg);
}

void
NOX::Multiphysics::Solver::FixedPointBased::reset()
{
  std::string msg = "Error - NOX::Multiphysics::Solver::FixedPointBased::reset() - this reset method is not valid for a Multiphysics Solver!";
  TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, msg);
}

NOX::Multiphysics::Solver::FixedPointBased::~FixedPointBased()
{

}


NOX::StatusTest::StatusType
NOX::Multiphysics::Solver::FixedPointBased::getStatus() const
{
  return status;
}

NOX::StatusTest::StatusType
NOX::Multiphysics::Solver::FixedPointBased::step()
{
  observer->runPreIterate(*this);

  // On the first step, do some initializations
  if (nIter == 0)
  {
    // Compute F of initital guess
    dataExInterface->exchangeAllData();
    NOX::Abstract::Group::ReturnType rtype = solnPtr->computeF();
    if (rtype != NOX::Abstract::Group::Ok)
    {
      utilsPtr->out() << "NOX::Multiphysics::Solver::FixedPointBased::step - "
              << "Unable to compute F" << std::endl;
      throw std::runtime_error("NOX Error");
    }

    // Test the initial guess
    status = testPtr->checkStatus(*this, checkType);
    if ((status == NOX::StatusTest::Converged) && (utilsPtr->isPrintType(NOX::Utils::Warning)))
        utilsPtr->out() << "Warning: NOX::Multiphysics::Solver::FixedPointBased::step() - "
                        << "The solution passed into the solver (either "
                        << "through constructor or reset method) "
                        << "is already converged!  The solver wil not "
                        << "attempt to solve this system since status is "
                        << "flagged as converged." << std::endl;

    printUpdate();
  }

  // First check status
  if (status != NOX::StatusTest::Unconverged)
  {
    observer->runPostIterate(*this);
    return status;
  }

  // Copy pointers into temporary references
  NOX::Abstract::Group& soln = *solnPtr;
  NOX::StatusTest::Generic& test = *testPtr;

  NOX::StatusTest::StatusType status2 = NOX::StatusTest::Unconverged;

  std::vector<Teuchos::RCP<NOX::Solver::Generic> >::iterator iter = (*solversVecPtr).begin();
  std::vector<Teuchos::RCP<NOX::Solver::Generic> >::iterator iter_end = (*solversVecPtr).end();

  for( int i = 0; iter_end != iter; ++iter, ++i )
  {
    status2 = NOX::StatusTest::Unconverged;

    // Conditionally bring all data needed from other problems to the current one
    if( SEIDEL == solveType )
      dataExInterface->exchangeDataTo(i);

    // Reset the problem's group
    const_cast<NOX::Abstract::Group&>((*iter)->getSolutionGroup()).setX((*iter)->getSolutionGroup().getX());

    const Teuchos::RCP<NOX::Abstract::Group> sameGrp =
        Teuchos::rcp( const_cast<NOX::Abstract::Group*>(&(*iter)->getSolutionGroup()), false );

    (*iter)->reset( sameGrp->getX() );

    status2 = (*iter)->solve();

    // Check return status
  }

  // Compute F for new current solution.
  NOX::Abstract::Group::ReturnType rtype = soln.computeF();
  if (rtype != NOX::Abstract::Group::Ok)
  {
    utilsPtr->out() << "NOX::Multiphysics::Solver::FixedPointBased::step - unable to compute F" << std::endl;
    status2 = NOX::StatusTest::Failed;
    observer->runPostIterate(*this);
    return status2;
  }

  // Update iteration count.
  nIter ++;

  // Bring all problems up-to-date - including forced residual evaluation
  dataExInterface->exchangeAllData();
  for( iter = (*solversVecPtr).begin(); iter_end != iter; ++iter )
    // Reset the problem's group
    const_cast<NOX::Abstract::Group&>((*iter)->getSolutionGroup()).setX((*iter)->getSolutionGroup().getX());
  rtype = solnPtr->computeF();
  if (rtype != NOX::Abstract::Group::Ok)
  {
    utilsPtr->out() << "NOX::Multiphysics::Solver::FixedPointBased::step - "
                    << "Unable to compute F" << std::endl;
    throw std::runtime_error("NOX Error");
  }

  // Evaluate the current status.
  status2 = test.checkStatus(*this, checkType);

  observer->runPostIterate(*this);

  // Return status.
  return status2;
}

NOX::StatusTest::StatusType
NOX::Multiphysics::Solver::FixedPointBased::solve()
{
  observer->runPreSolve(*this);

  // Iterate until converged or failed
  while (status == NOX::StatusTest::Unconverged)
  {
    status = step();
    printUpdate();
  }

  Teuchos::ParameterList& outputParams = paramsPtr->sublist("Output");
  outputParams.set("Nonlinear Iterations", nIter);
  outputParams.set("2-Norm of Residual", solnPtr->getNormF());

  observer->runPostSolve(*this);

  return status;
}

const NOX::Abstract::Group &
NOX::Multiphysics::Solver::FixedPointBased::getSolutionGroup() const
{
  return *solnPtr;
}

const NOX::Abstract::Group&
NOX::Multiphysics::Solver::FixedPointBased::getPreviousSolutionGroup() const
{
  utilsPtr->out() << "NOX::Multiphysics::Solver::FixedPointBased::getPreviousSolutionGroup - "
                  << "Old group not available.  This method is not currently supported." << std::endl;
  throw std::runtime_error("NOX Error");
}

Teuchos::RCP< const NOX::Abstract::Group>
NOX::Multiphysics::Solver::FixedPointBased::getPreviousSolutionGroupPtr() const
{
  utilsPtr->out() << "NOX::Multiphysics::Solver::FixedPointBased::getPreviousSolutionGroupPtr - "
                  << "Old group not available.  This method is not currently supported." << std::endl;
  throw std::runtime_error("NOX Error");
}

int NOX::Multiphysics::Solver::FixedPointBased::getNumIterations() const
{
  return nIter;
}

const Teuchos::ParameterList&
NOX::Multiphysics::Solver::FixedPointBased::getList() const
{
  return *paramsPtr;
}

Teuchos::RCP<const NOX::SolverStats>
NOX::Multiphysics::Solver::FixedPointBased::getSolverStatistics() const
{
  return globalDataPtr->getSolverStatistics();
}

// protected
void NOX::Multiphysics::Solver::FixedPointBased::printUpdate()
{
  double normSoln = 0;
  //double normStep = 0;

  // Print the status test parameters at each iteration if requested
  if ((status == NOX::StatusTest::Unconverged) &&
      (utilsPtr->isPrintType(NOX::Utils::OuterIterationStatusTest)))
  {
    utilsPtr->out() << NOX::Utils::fill(72) << "\n";
    utilsPtr->out() << "-- Status Test Results --\n";
    testPtr->print(utilsPtr->out());
    utilsPtr->out() << NOX::Utils::fill(72) << "\n";
  }

  // All processes participate in the computation of these norms...
  if (utilsPtr->isPrintType(NOX::Utils::OuterIteration))
    normSoln = solnPtr->getNormF();

  // ...But only the print process actually prints the result.
  if (utilsPtr->isPrintType(NOX::Utils::OuterIteration))
  {
    utilsPtr->out() << "\n" << NOX::Utils::fill(72) << "\n";
    utilsPtr->out() << "-- Fixed-point Solver Step " << nIter << " -- \n";
    utilsPtr->out() << "Fixed-point ||F|| = " << utilsPtr->sciformat(normSoln);
    if (status == NOX::StatusTest::Converged)
      utilsPtr->out() << " (Converged!)";
    if (status == NOX::StatusTest::Failed)
      utilsPtr->out() << " (Failed!)";
    utilsPtr->out() << "\n" << NOX::Utils::fill(72) << "\n" << std::endl;
  }

  // Print the final parameter values of the status test
  if ((status != NOX::StatusTest::Unconverged) &&
      (utilsPtr->isPrintType(NOX::Utils::OuterIteration)))
  {
    utilsPtr->out() << NOX::Utils::fill(72) << "\n";
    utilsPtr->out() << "-- Final Status Test Results --\n";
    testPtr->print(utilsPtr->out());
    utilsPtr->out() << NOX::Utils::fill(72) << "\n";
  }
}
