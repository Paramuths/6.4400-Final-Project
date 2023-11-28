#ifndef INTEGRATOR_FACTORY_H_
#define INTEGRATOR_FACTORY_H_

#include "IntegratorBase.hpp"

#include <stdexcept>

#include "gloo/utils.hpp"
#include "IntegratorType.hpp"
#include "RungeKutta4Integrator.hpp"

namespace GLOO {
class IntegratorFactory {
 public:
  template <class TSystem, class TState>
  static std::unique_ptr<IntegratorBase<TSystem, TState>> CreateIntegrator() {
    return make_unique<RungeKutta4Integrator<TSystem, TState>>();
  }
};
}  // namespace GLOO

#endif
