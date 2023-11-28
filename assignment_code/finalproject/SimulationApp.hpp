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
                float integration_step);
  void SetupScene() override;

 private:
  float integration_step_;
};
}  // namespace GLOO

#endif
