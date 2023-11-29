#ifndef EXPLODING_SYSTEM_H_
#define EXPLODING_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class ExplodingSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;

        for (int i = 0; i < state.positions.size(); i++) {
            gradient_state.positions.push_back(state.velocities[i]);
            auto drag_force = -drag_constant * state.velocities[i];
            auto acceleration = gravity_ + drag_force;
            gradient_state.velocities.push_back(acceleration);
        }

        return gradient_state;
    };

    public:

    private:
    glm::vec3 gravity_ = glm::vec3(0.f, 0.f, 0.f); // change the gravity here
    float drag_constant = .0f; // change the drag here
};
}  // namespace GLOO

#endif
