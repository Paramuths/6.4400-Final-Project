#ifndef PENDULUM_SYSTEM_H_
#define PENDULUM_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class PendulumSystem : public ParticleSystemBase {
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState gradient_state;
        auto num_particles = state.positions.size();
        auto spring_force_list = CalculateSpringForce(state);

        for (int i = 0; i < num_particles; i++) {
            gradient_state.positions.push_back(state.velocities[i]);
            // Calculate acceleration
            auto gravity_force = particle_mass_[i] * gravity_;
            auto drag_force = -drag_constant * state.velocities[i];
            auto spring_force = spring_force_list[i];
            auto acceleration = (gravity_force + drag_force + spring_force)/particle_mass_[i];
            gradient_state.velocities.push_back(acceleration);
        }

        // Set the gradient of fixed points to zero
        for (auto particle_idx: fixed_particle_) {
            gradient_state.positions[particle_idx] = glm::vec3(0.f);
            gradient_state.velocities[particle_idx] = glm::vec3(0.f);
        }

        return gradient_state;
    };

    public:
    void SetMass(std::vector<float> particle_mass) {
        particle_mass_ = particle_mass;
    }
    
    void UpdateMass(int index, float mass) {
        particle_mass_[index] = mass;
    }

    void AddMass(float mass) {
        particle_mass_.push_back(mass);
    }

    void SetSpringForce(std::vector<std::tuple<int, int, float, float>> spring_force) {
        spring_force_ = spring_force;
    }

    void AddSpringForce(std::tuple<int, int, float, float> single_spring_force) {
        spring_force_.push_back(single_spring_force);
    }

    void SetFixedParticle(std::vector<int> fixed_particle) {
        fixed_particle_ = fixed_particle;
    }

    void AddFixedParticle(int single_fixed_particle) {
        fixed_particle_.push_back(single_fixed_particle);
    }

    private:
    std::vector<glm::vec3> CalculateSpringForce(const ParticleState& state) const {
        std::vector<glm::vec3> spring_force(state.positions.size(), glm::vec3(0.f));

        for (auto single_spring_force: spring_force_) {
            auto particle_i = std::get<0>(single_spring_force);
            auto particle_j = std::get<1>(single_spring_force);
            auto spring_constant = std::get<2>(single_spring_force);
            auto rest_length = std::get<3>(single_spring_force);
            
            auto d = state.positions[particle_i] - state.positions[particle_j];
            auto length_d = glm::length(d);
            auto spring_force_pair = -spring_constant*(length_d - rest_length)*(d/length_d);
            spring_force[particle_i] += spring_force_pair;
            spring_force[particle_j] -= spring_force_pair;
        }

        return spring_force;
    }

    std::vector<float> particle_mass_; // particle_mass_[i] = mass of particle i
    std::vector<std::tuple<int, int, float, float>> spring_force_; // a list of (index i, index j, spring constant, rest length)
    std::vector<int> fixed_particle_; // fixed_particle_[i] = index of particle to fixed;

    glm::vec3 gravity_ = glm::vec3(0.f, -9.8f, 0.f);
    float drag_constant = .2f;
};
}  // namespace GLOO

#endif
