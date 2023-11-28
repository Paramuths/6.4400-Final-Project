#ifndef TRAPEZOIDAL_INTEGRATOR_H_
#define TRAPEZOIDAL_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class TrapezoidalIntegrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    // TODO: Here we are returning the state at time t (which is NOT what we
    // want). Please replace the line below by the state at time t + dt using
    // forward Euler integration.
    auto f0 = system.ComputeTimeDerivative(state, start_time);
    auto f1 = system.ComputeTimeDerivative(state + (dt * f0), start_time + dt);
    auto next_state = state + ((dt/2) * (f0 + f1));
    return next_state;
  }
};
}  // namespace GLOO

#endif
