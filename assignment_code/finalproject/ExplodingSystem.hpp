#ifndef EXPLODING_SYSTEM_H_
#define EXPLODING_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class ExplodingSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;
        bool get_rekt = false;
        float time_since_explode = 0.f;
        if(time >= start_explosion_ && time < start_explosion_ + epsilon){
            get_rekt = true;
            time_since_explode = time - start_explosion_;
        }
        for (int i = 0; i < state.positions.size() / 3; i++) {
            for(int j=0; j<3; j++) gradient_state.positions.push_back(state.velocities[3 * i]);
            auto drag_force = -drag_constant * state.velocities[3 * i];
            auto acceleration = gravity_ + drag_force;
            if(get_rekt)
            {
                glm::vec3 avg_explosion = glm::vec3(0.f,0.f,0.f);
                for(int j=0; j<3; j++){
                    avg_explosion += ExplodingSystem::CalcExplosionAcc(state.positions[3 * i + j]);
                }
                acceleration += avg_explosion * (1.0f/3.0f) * (1.0f - time_since_explode / epsilon);
            }
            for(int j=0; j<3; j++) gradient_state.velocities.push_back(acceleration);
        }

        return gradient_state;
    };

    public:
    glm::vec3 CalcExplosionAcc(glm::vec3 vertex_position) const
    {
        glm::vec3 direction = vertex_position - explosion_center;
        float dist = glm::length(direction);
        if(dist == 0.0f) return glm::vec3(0.f,0.f,0.f);
        float explosion_attenuation = explosion_coef[0] * dist * dist + explosion_coef[1] * dist + explosion_coef[2];
        return direction / (dist * explosion_attenuation);
    }

    private:
    glm::vec3 gravity_ = glm::vec3(0.f, 0.0f, 0.f); // change the gravity here
    float drag_constant = 0.1f; // change the drag here
    float start_explosion_ = 3.0f; // change time to start the explosion here
    float epsilon = 0.5f; // duration where force of explosion still exists
    glm::vec3 explosion_center = glm::vec3(0.0f,0.067f,0.f); // origin of explosion
    // params for acceleration from explosion, assumed to be roughly inverse qudratic in distance from origin of explosion
    glm::vec3 explosion_coef = glm::vec3(1.0f, 0.0f, 0.2f);
};
}  // namespace GLOO

#endif
