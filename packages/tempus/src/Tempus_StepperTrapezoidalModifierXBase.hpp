//@HEADER
// *****************************************************************************
//          Tempus: Time Integration and Sensitivity Analysis Package
//
// Copyright 2017 NTESS and the Tempus contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
//@HEADER

#ifndef Tempus_StepperTrapezoidalModifierXBase_hpp
#define Tempus_StepperTrapezoidalModifierXBase_hpp

#include "Tempus_config.hpp"
#include "Tempus_SolutionHistory.hpp"
#include "Tempus_StepperTrapezoidalAppAction.hpp"

namespace Tempus {

/** \brief Base ModifierX for StepperTrapezoidal.
 *
 *  This class provides a means to modify just the solution values
 *  (i.e., \f$x\f$ and \f$dot{x}\f$), and nothing else, but time and
 *  timestep are also provided.
 *
 *  Users deriving from this class can access and change the solution
 *  during the timestep (e.g., limiting the solution for monoticity).
 *  It is expected that the user knows what changes are allowable without
 *  affecting the Stepper correctness, performance, accuracy and stability
 *  (i.e., USER BEWARE!!).
 *
 *  The locations of the StepperTrapezoidalModifierXBase::MODIFIER_TYPE
 *  which correspond to the AppAction calls
 *  (StepperTrapezoidalAppAction::ACTION_LOCATION) are shown in the
 *  algorithm documentation of the StepperTrapezoidal.
 */
template <class Scalar>
class StepperTrapezoidalModifierXBase
  : virtual public Tempus::StepperTrapezoidalAppAction<Scalar> {
 private:
  /* \brief Adaptor execute function
   *
   *  This is an adaptor function to bridge between the AppAction
   *  interface and the ModifierX interface.  It is meant to be private
   *  and non-virtual as deriving from this class should only need to
   *  implement the modify function.
   *
   *  For the ModifierX interface, this adaptor maps the
   *  StepperTrapezoidalAppAction::ACTION_LOCATION to the
   *  StepperTrapezoidalModifierX::MODIFIERX_TYPE, and only pass the solution
   *  (\f$x\f$ and/or \f$\dot{x}\f$ and other parameters to the modify
   *  function.
   */
  void execute(
      Teuchos::RCP<SolutionHistory<Scalar> > sh,
      Teuchos::RCP<StepperTrapezoidal<Scalar> > stepper,
      const typename StepperTrapezoidalAppAction<Scalar>::ACTION_LOCATION
          actLoc)
  {
    using Teuchos::RCP;

    MODIFIER_TYPE modType                    = X_BEGIN_STEP;
    RCP<SolutionState<Scalar> > workingState = sh->getWorkingState();
    const Scalar time                        = workingState->getTime();
    const Scalar dt                          = workingState->getTimeStep();
    RCP<Thyra::VectorBase<Scalar> > x;

    switch (actLoc) {
      case StepperTrapezoidalAppAction<Scalar>::BEGIN_STEP: {
        modType = X_BEGIN_STEP;
        x       = workingState->getX();
        break;
      }
      case StepperTrapezoidalAppAction<Scalar>::BEFORE_SOLVE: {
        modType = X_BEFORE_SOLVE;
        x       = workingState->getX();
        break;
      }
      case StepperTrapezoidalAppAction<Scalar>::AFTER_SOLVE: {
        modType = X_AFTER_SOLVE;
        x       = workingState->getX();
        break;
      }
      case StepperTrapezoidalAppAction<Scalar>::END_STEP: {
        modType = XDOT_END_STEP;
        if (workingState->getXDot() != Teuchos::null)
          x = workingState->getXDot();
        else
          x = stepper->getStepperXDot();
        break;
      }
      default:
        TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error,
                                   "Error - unknown action location.\n");
    }

    this->modify(x, time, dt, modType);
  }

 public:
  /// Indicates the location of application action (see algorithm).
  enum MODIFIER_TYPE {
    X_BEGIN_STEP,    ///< Modify \f$x\f$ at the beginning of the step.
    X_BEFORE_SOLVE,  ///< Modify \f$x\f$ before the implicit solve.
    X_AFTER_SOLVE,   ///< Modify \f$x\f$ after the implicit solve.
    XDOT_END_STEP    ///< Modify \f$\dot{x}\f$ at the end of the step.
  };

  /// Modify solution based on the MODIFIER_TYPE.
  virtual void modify(Teuchos::RCP<Thyra::VectorBase<Scalar> > /* x */,
                      const Scalar /* time */, const Scalar /* dt */,
                      const MODIFIER_TYPE modType) = 0;
};

}  // namespace Tempus

#endif  // Tempus_StepperTrapezoidalModifierXBase_hpp
