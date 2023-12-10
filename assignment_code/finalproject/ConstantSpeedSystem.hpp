#ifndef CONSTANT_SPEED_SYSTEM_H_
#define CONSTANT_SPEED_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class ConstantSpeedSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;
        for (auto velocity: state.velocities) {
            gradient_state.positions.push_back(velocity);
            gradient_state.velocities.push_back(drag_constant * velocity);
        }
        return gradient_state;
    };

    private:
    float drag_constant = 0.f;//-0.05f; // change the drag here
};
}  // namespace GLOO

#endif
