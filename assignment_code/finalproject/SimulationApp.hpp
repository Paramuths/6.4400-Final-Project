#ifndef SIMULATION_APP_H_
#define SIMULATION_APP_H_

#include "gloo/Application.hpp"

#include "IntegratorType.hpp"
#include "ParticleSystemBase.hpp"
#include "ParticleState.hpp"

namespace GLOO {
class SimulationApp : public Application {
 public:
  SimulationApp(const std::string& app_name,
                glm::ivec2 window_size,
                IntegratorType integrator_type,
                float integration_step,
                int spring_particle_index);
  void SetupScene() override;

 private:
  IntegratorType integrator_type_;
  float integration_step_;
  int spring_particle_index_;
};
}  // namespace GLOO

#endif
