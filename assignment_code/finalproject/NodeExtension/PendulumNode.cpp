#include "PendulumNode.hpp"

#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
    PendulumNode::PendulumNode(float integration_step): 
        integration_step_(integration_step) {
        // Instantiate PhongShader, VertexObject, Material
        phong_shader_ = std::make_shared<PhongShader>();
        sphere_mesh_ = PrimitiveFactory::CreateSphere(0.2f, 50, 50);
        sphere_material_ = std::make_shared<Material>(glm::vec3(1.f, .3f, .3f),
                                                    glm::vec3(1.f, .3f, .3f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        spring_mesh_ = PrimitiveFactory::CreateCylinder(0.05f, 1.f, 50);
        spring_material_ = std::make_shared<Material>(glm::vec3(.3f, .6f, .3f),
                                                    glm::vec3(.3f, .6f, .3f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);

        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>();
        particle_system_ = PendulumSystem();
        Init();
    }

    void PendulumNode::Update(double delta_time) {
        int num_steps = (delta_time + carrier_time_step_)/integration_step_;
        carrier_time_step_ = delta_time + carrier_time_step_ - float(num_steps * integration_step_);
        for (int i = 0; i < num_steps; i++) {
            Advance(float(i * integration_step_));
        }
        // Toggle 'R' to reset
        static bool prev_released = true;
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            if (prev_released) {
            InitParticle();
            }
            prev_released = false;
        } else {
            prev_released = true;
        }
        SetPosition();
    }

    void PendulumNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void PendulumNode::SetPosition() {
        for (int i = 0; i < particle_pointer_.size(); i++) {
            particle_pointer_[i]->GetTransform().SetPosition(particle_state_.positions[i]);
        }
    }

    void PendulumNode::InitParticle() {
        particle_state_.positions = {};
        particle_state_.velocities = {};
        particle_mass_ = {};
        // First particle
        particle_state_.positions.push_back(glm::vec3(-2.5f, 0.f, 0.f));
        particle_state_.velocities.push_back(glm::vec3(1.f, 1.f, 1.f));
        particle_mass_.push_back(1.f);

        // Second particle
        particle_state_.positions.push_back(glm::vec3(-.5f, 0.f, 0.f));
        particle_state_.velocities.push_back(glm::vec3(0.f, -2.f, 0.f));
        particle_mass_.push_back(3.f);
        
        // Third particle
        particle_state_.positions.push_back(glm::vec3(.0f, 3.f, 0.f));
        particle_state_.velocities.push_back(glm::vec3(1.f, 0.f, -1.f));
        particle_mass_.push_back(5.f);
        
        // Fourth particle
        particle_state_.positions.push_back(glm::vec3(-4.f, 0.f, 0.f));
        particle_state_.velocities.push_back(glm::vec3(-1.f, 3.f, 0.f));
        particle_mass_.push_back(2.f);
    }

    void PendulumNode::Init() {
        InitParticle();

        for (auto position: particle_state_.positions) {
            auto sphere_node = make_unique<SceneNode>();
            sphere_node->CreateComponent<ShadingComponent>(phong_shader_);
            sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
            sphere_node->CreateComponent<MaterialComponent>(sphere_material_);
            sphere_node->GetTransform().SetPosition(position);

            particle_pointer_.push_back(sphere_node.get());
            AddChild(std::move(sphere_node));
        }

        particle_system_.SetMass(particle_mass_);
        particle_system_.AddFixedParticle(0); // Fixed the first particle

        spring_force_ = {std::make_tuple(0, 1, 30.f, 1.f), // Set spring force between first and second particle
                        std::make_tuple(1, 2, 30.f, 2.f), // Set spring force between second and third particle
                        std::make_tuple(2, 3, 30.f, 3.f), // Set spring force between third and fourth particle
                        };
        particle_system_.SetSpringForce(spring_force_); 
    }
}