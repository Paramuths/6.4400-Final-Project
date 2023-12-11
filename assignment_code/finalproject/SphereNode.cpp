#include "SphereNode.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include <fstream>

namespace GLOO {
    SphereNode::SphereNode(float integration_step): 
        integration_step_(integration_step) {
        Init();
    }

    void SphereNode::Init() {
        InitSphere();
        InitParticle();
        InitSystem();
        SetPositions();
    }

    void SphereNode::InitSphere() {
        // my_shader_ = std::make_shared<MyShader>();
        phong_shader_ = std::make_shared<PhongShader>();
        sphere_material_ = std::make_shared<Material>(glm::vec3(0.f, 1.f, 0.f),
                                                    glm::vec3(0.f, 1.f, 0.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        sphere_mesh_ = std::shared_ptr<VertexObject>(PrimitiveFactory::CreateSphere(0.05f, 20, 20));

        auto sphere_node = make_unique<SceneNode>();
        sphere_node->CreateComponent<ShadingComponent>(phong_shader_);
        sphere_node->CreateComponent<MaterialComponent>(sphere_material_);
        sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
        sphere_pointer_ = sphere_node.get();
        AddChild(std::move(sphere_node));
    }

    void SphereNode::InitParticle() {
        auto initial_position = glm::vec3(-0.67f, 0.2f, 0.0f);
        particle_state_.positions = {initial_position};
        particle_state_.velocities = {glm::vec3(0.8f, 0.f, 0.f)};

        start_ = false;
    }

    void SphereNode::InitSystem() {
        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>();
        particle_system_ = ConstantSpeedSystem();
    }

    void SphereNode::Update(double delta_time) {
        double slow_factor = 1;
        double delta_time_ = delta_time / slow_factor;
        if (start_) {
            int num_steps = (delta_time_ + carrier_time_step_)/integration_step_;
            carrier_time_step_ = delta_time_ + carrier_time_step_ - float(num_steps * integration_step_);
            for (int i = 0; i < num_steps; i++) {
                Advance(float(i * integration_step_));
            }
            SetPositions();
        }

        // Toggle 'R' to reset
        static bool prev_released = true;
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            if (prev_released) {
                if (start_) {
                    InitParticle();
                    SetPositions();
                }
            }
            prev_released = false;
        // Toggle 'E' to explode
        } else if (InputManager::GetInstance().IsKeyPressed('E')) {
            if (prev_released) {
                start_ = true;
            }
            prev_released = false;
        } else {
            prev_released = true;
        }
    }

    void SphereNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void SphereNode::SetPositions() {
        GetTransform().SetPosition(particle_state_.positions[0]);
    }
}