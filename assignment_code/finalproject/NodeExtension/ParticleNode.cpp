#include "ParticleNode.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
    ParticleNode::ParticleNode(IntegratorType integrator_type, float integration_step): 
        integration_step_(integration_step) {
        GetTransform().SetPosition(glm::vec3(-2.5f, 0.f, 0.f));
        // Instantiate PhongShader, VertexObject, Material
        std::shared_ptr<PhongShader> phong_shader = std::make_shared<PhongShader>();
        std::shared_ptr<VertexObject> sphere_mesh = PrimitiveFactory::CreateSphere(0.1f, 50, 50);
        std::shared_ptr<Material> sphere_material = std::make_shared<Material>(glm::vec3(0.f, 1.f, 1.f),
                                                                                glm::vec3(0.f, 1.f, 1.f),
                                                                                glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);

        // Create ShadingComponent, RenderingComponent, MaterialComponent
        auto sphere_node = make_unique<SceneNode>();
        sphere_node->CreateComponent<ShadingComponent>(phong_shader);
        sphere_node->CreateComponent<RenderingComponent>(sphere_mesh);
        sphere_node->CreateComponent<MaterialComponent>(sphere_material);
        sphere_node_ = sphere_node.get();
        AddChild(std::move(sphere_node));

        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>(integrator_type);
        particle_system_ = SimpleParticleSystem();
        InitParticle();
        SetPosition();
    }

    void ParticleNode::InitParticle() {
        particle_state_.positions = {glm::vec3(0.5f, .5f, 0.f)};
        particle_state_.velocities = {glm::vec3(0.f)};
    }

    void ParticleNode::Update(double delta_time) {
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

    void ParticleNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void ParticleNode::SetPosition() {
        sphere_node_->GetTransform().SetPosition(particle_state_.positions[0]);
    }

}