#ifndef EXPLODING_SYSTEM_H_
#define EXPLODING_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class ExplodingSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;
        int num_explosive = 2;
        std::vector<bool> get_rekt_(num_explosive, false);
        std::vector<float> time_since_explode_(num_explosive, 0.f);
        for(int i=0; i<num_explosive; i++){
            if(time >= start_explosion_[i] && time < start_explosion_[i] + epsilon_[i]){
                get_rekt_[i] = true;
                time_since_explode_[i] = time - start_explosion_[i];
            }
        }
        for (int i = 0; i < state.positions.size() / 3; i++) {
            for(int j=0; j<3; j++) gradient_state.positions.push_back(state.velocities[3 * i]);
            auto drag_force = -drag_constant * state.velocities[3 * i];
            auto acceleration = gravity_ + drag_force;
            for(int k=0; k<num_explosive; k++){
                if(get_rekt_[k])
                {
                    glm::vec3 avg_explosion = glm::vec3(0.f,0.f,0.f);
                    for(int j=0; j<3; j++){
                        avg_explosion += ExplodingSystem::CalcExplosionAcc(state.positions[3 * i + j], k);
                    }
                    acceleration += avg_explosion * (1.0f/3.0f) * (1.0f - time_since_explode_[k] / epsilon_[k]);
                }
            }

            for(int j=0; j<3; j++) gradient_state.velocities.push_back(acceleration);
        }

        return gradient_state;
    };

    public:
    glm::vec3 CalcExplosionAcc(glm::vec3 vertex_position, int k) const
    {
        glm::vec3 direction = vertex_position - explosion_center_[k];
        float dist = glm::length(direction);
        if(dist == 0.0f) return glm::vec3(0.f,0.f,0.f);
        float explosion_attenuation = explosion_coef_[k][0] * dist * dist + explosion_coef_[k][1] * dist + explosion_coef_[k][2];
        return direction / (dist * explosion_attenuation);
    }

    private:
    glm::vec3 gravity_ = glm::vec3(0.f, 0.0f, 0.f); // change the gravity here
    float drag_constant = 0.1f; // change the drag here
    // float start_explosion = 3.0f; // change time to start the explosion here
    // float epsilon = 0.5f; // duration where force of explosion still exists
    // glm::vec3 explosion_center = glm::vec3(0.0f,0.067f,0.f); // origin of explosion
    // // params for acceleration from explosion, assumed to be roughly inverse qudratic in distance from origin of explosion
    // glm::vec3 explosion_coef = glm::vec3(1.0f, 0.0f, 0.2f);

    //using more than one explosive
    std::vector<float> start_explosion_ = {3.0f, 2.6f};
    std::vector<float> epsilon_ = {0.4f, 0.5f};
    std::vector<glm::vec3> explosion_center_ = {glm::vec3(0.15f,0.067f,0.f), glm::vec3(-0.2f,0.067f,0.f)}; 
    std::vector<glm::vec3> explosion_coef_ = {glm::vec3(0.9f, 0.0f, 0.1f),glm::vec3(1.1f, 0.0f, 0.3f)};

};
}  // namespace GLOO

#endif
