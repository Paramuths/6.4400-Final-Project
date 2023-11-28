#ifndef SPHERE_SYSTEM_H_
#define SPHERE_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class SphereSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;
        auto spring_force = spring_constant_ * (fixed_point_ - state.positions[0]);
        gradient_state.positions.push_back(state.velocities[0]);
        gradient_state.velocities.push_back(spring_force);
        return gradient_state;
    };

    public:
    void SetFixedPoint (glm::vec3 position) {
        fixed_point_ = position;
    }

    glm::vec3 fixed_point_ = glm::vec3(0.f);
    float spring_constant_ = 1.f;
};
}  // namespace GLOO

#endif
