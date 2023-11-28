#ifndef SIMPLE_PARTICLE_SYSTEM_H_
#define SIMPLE_PARTICLE_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class SimpleParticleSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;
        for (auto individual_state: state.positions) {
            gradient_state.positions.push_back(glm::vec3(-individual_state[1], individual_state[0], 0));
            gradient_state.velocities.push_back(glm::vec3(0.f));
        }
        return gradient_state;
    };
};
}  // namespace GLOO

#endif
