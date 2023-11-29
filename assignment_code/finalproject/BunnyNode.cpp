#include "BunnyNode.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include <fstream>

namespace GLOO {
    BunnyNode::BunnyNode(float integration_step): 
        integration_step_(integration_step) {
        Init();
    }

    void BunnyNode::Init() {
        InitBunny();
        InitParticle();
        InitSphere();
        InitSystem();
    }

    void BunnyNode::InitBunny() {
        my_shader_ = std::make_shared<MyShader>();
        bunny_material_ = std::make_shared<Material>(glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        std::string bunny_path = GetAssetDir() + "bunny_1k.obj";
        bunny_mesh_ = MeshLoader::Import(bunny_path).vertex_obj;
        bunny_positions_ = bunny_mesh_->GetPositions();
        SetNormals();
        bunny_normals_ = bunny_mesh_->GetNormals();
        SetColors();
        bunny_scale_ = glm::vec3(3.f);

        auto bunny_node = make_unique<SceneNode>();
        bunny_node->CreateComponent<ShadingComponent>(my_shader_);
        bunny_node->CreateComponent<MaterialComponent>(bunny_material_);
        bunny_node->CreateComponent<RenderingComponent>(bunny_mesh_);
        bunny_node->GetTransform().SetScale(bunny_scale_);
        bunny_pointer_ = bunny_node.get();
        AddChild(std::move(bunny_node));
    }

    void BunnyNode::InitParticle() {
        particle_state_.positions = {};
        particle_state_.velocities = {};

        for (auto position: bunny_positions_) {
            particle_state_.positions.push_back(position);
        }
        for (auto normal: bunny_normals_) {
            particle_state_.velocities.push_back(normal * 0.05f);
        }
    }

    void BunnyNode::InitSphere() {
        phong_shader_ = std::make_shared<PhongShader>();
        sphere_material_ = std::make_shared<Material>(glm::vec3(0.f, 1.f, 1.f),
                                                    glm::vec3(0.f, 1.f, 0.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        sphere_mesh_ = PrimitiveFactory::CreateSphere(0.01f, 20, 20);

        for (auto position: particle_state_.positions) {
            auto sphere_node = make_unique<SceneNode>();
            sphere_node->CreateComponent<ShadingComponent>(phong_shader_);
            sphere_node->CreateComponent<MaterialComponent>(sphere_material_);
            sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
            // sphere_node->SetActive(false);

            auto sphere_parent_node = make_unique<SceneNode>();
            sphere_parent_node->GetTransform().SetPosition(position * bunny_scale_);
            sphere_parent_node->AddChild(std::move(sphere_node));

            sphere_parent_pointers_.push_back(sphere_parent_node.get());
            AddChild(std::move(sphere_parent_node));
        }
    }

    void BunnyNode::InitSystem() {
        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>();
        particle_system_ = ExplodingSystem();
    }

    void BunnyNode::Update(double delta_time) {
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
        SetPositions();
        SetNormals();
    }

    void BunnyNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void BunnyNode::SetPositions() {
        auto positions = make_unique<PositionArray>();
        for (int i = 0; i < particle_state_.positions.size(); i++) {
            positions->push_back(particle_state_.positions[i]);
            sphere_parent_pointers_[i]->GetTransform().SetPosition(particle_state_.positions[i] * bunny_scale_);
        }
        // bunny_mesh_->UpdatePositions(std::move(positions));
    }

    void BunnyNode::SetNormals() {
        auto positions = bunny_mesh_->GetPositions();
        auto indices = bunny_mesh_->GetIndices();

        std::vector<std::vector<glm::vec3>> normal_around_vertices;
        for (int i = 0; i < positions.size(); i++) {
            normal_around_vertices.push_back(std::vector<glm::vec3>());
        }

        for (int i = 0; i < indices.size(); i += 3) {
            auto vertex1 = positions[indices[i]];
            auto vertex2 = positions[indices[i + 1]];
            auto vertex3 = positions[indices[i + 2]];
            auto cross_product = glm::cross(vertex3 - vertex2, vertex1 - vertex2);
            
            normal_around_vertices[indices[i]].push_back(cross_product);
            normal_around_vertices[indices[i + 1]].push_back(cross_product);
            normal_around_vertices[indices[i + 2]].push_back(cross_product);
        }

        auto normals = make_unique<NormalArray>();
        for (int i = 0; i < positions.size(); i++) {
            glm::vec3 normal(0.f, 0.f, 0.f);
            auto normal_around_vertex = normal_around_vertices[i];
            for (int j = 0; j < normal_around_vertex.size(); j++) {
            normal += normal_around_vertex[j];
            }
            normals->push_back(glm::normalize(normal));
        }
        bunny_mesh_->UpdateNormals(std::move(normals));
    }

    void BunnyNode::SetColors() {
        auto colors = make_unique<ColorArray>();
        for (int i = 0; i < bunny_positions_.size(); i++) {
            glm::vec3 color(0.f, 0.f, 0.f);
            colors->push_back(glm::vec4(color[0], color[1], color[2], 0.f));
        }
        bunny_mesh_->UpdateColors(std::move(colors));
    }
}