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
        InitTriangle();
        InitSystem();
    }

    void BunnyNode::InitBunny() {
        // my_shader_ = std::make_shared<MyShader>();
        bunny_shader_ = std::make_shared<PhongShader>();
        bunny_material_ = std::make_shared<Material>(glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        std::string bunny_path = GetAssetDir() + "bunny_1k.obj";
        bunny_mesh_ = MeshLoader::Import(bunny_path).vertex_obj;
        bunny_positions_ = bunny_mesh_->GetPositions();
        bunny_indices_ = bunny_mesh_->GetIndices();
        SetNormals();
        bunny_normals_ = bunny_mesh_->GetNormals();
        bunny_scale_ = glm::vec3(3.f);

        auto bunny_node = make_unique<SceneNode>();
        bunny_node->CreateComponent<ShadingComponent>(bunny_shader_);
        bunny_node->CreateComponent<MaterialComponent>(bunny_material_);
        bunny_node->CreateComponent<RenderingComponent>(bunny_mesh_);
        bunny_node->GetTransform().SetScale(bunny_scale_);
        bunny_pointer_ = bunny_node.get();
        AddChild(std::move(bunny_node));
    }

    void BunnyNode::InitParticle() {
        particle_state_.positions = {};
        particle_state_.velocities = {};

        for (int i = 0; i < bunny_indices_.size(); i += 3) {
            particle_state_.positions.push_back(bunny_positions_[bunny_indices_[i]]);
            particle_state_.positions.push_back(bunny_positions_[bunny_indices_[i + 1]]);
            particle_state_.positions.push_back(bunny_positions_[bunny_indices_[i + 2]]);

            auto normal = glm::normalize(glm::cross(bunny_positions_[bunny_indices_[i + 1]] - bunny_positions_[bunny_indices_[i]],
                                                    bunny_positions_[bunny_indices_[i + 2]] - bunny_positions_[bunny_indices_[i]]));
            particle_state_.velocities.push_back(normal * 0.05f);
            particle_state_.velocities.push_back(normal * 0.05f);
            particle_state_.velocities.push_back(normal * 0.05f);
        }

        exploding_ = false;
    }

    void BunnyNode::InitTriangle() {
        phong_shader_ = std::make_shared<PhongShader>();
        triangle_material_ = std::make_shared<Material>(glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        // sphere_mesh_ = PrimitiveFactory::CreateSphere(0.01f, 10, 10);

        for (int i = 0; i < bunny_indices_.size(); i += 3) {
            auto triangle_node = make_unique<SceneNode>();
            triangle_node->CreateComponent<ShadingComponent>(phong_shader_);
            triangle_node->CreateComponent<MaterialComponent>(triangle_material_);

            auto triangle_mesh = std::make_shared<VertexObject>();

            auto positions = make_unique<PositionArray>();
            positions->push_back(bunny_positions_[bunny_indices_[i]]);
            positions->push_back(bunny_positions_[bunny_indices_[i + 1]]);
            positions->push_back(bunny_positions_[bunny_indices_[i + 2]]);

            auto normals = make_unique<NormalArray>();
            normals->push_back(bunny_normals_[bunny_indices_[i]]);
            normals->push_back(bunny_normals_[bunny_indices_[i + 1]]);
            normals->push_back(bunny_normals_[bunny_indices_[i + 2]]);

            auto indices = make_unique<IndexArray>();
            indices->push_back(0);
            indices->push_back(1);
            indices->push_back(2);

            triangle_mesh->UpdatePositions(std::move(positions));
            triangle_mesh->UpdateNormals(std::move(normals));
            triangle_mesh->UpdateIndices(std::move(indices));

            triangle_node->CreateComponent<RenderingComponent>(triangle_mesh).SetDrawMode(DrawMode::Triangles);
            triangle_node->GetTransform().SetScale(bunny_scale_);
            triangle_node->SetActive(false);

            triangle_pointers_.push_back(triangle_node.get());
            AddChild(std::move(triangle_node));
        }
    }

    void BunnyNode::InitSystem() {
        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>();
        particle_system_ = ExplodingSystem();
    }

    void BunnyNode::Update(double delta_time) {
        if (exploding_) {
            int num_steps = (delta_time + carrier_time_step_)/integration_step_;
            carrier_time_step_ = delta_time + carrier_time_step_ - float(num_steps * integration_step_);
            for (int i = 0; i < num_steps; i++) {
                Advance(float(i * integration_step_));
            }
            SetPositions();
        }

        // Toggle 'R' to reset
        static bool prev_released = true;
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            if (prev_released) {
                if (exploding_) {
                    InitParticle();
                    SetPositions();
                    SetNormals();
                    ResetExplosionActive();
                }
            }
            prev_released = false;
        // Toggle 'E' to explode
        } else if (InputManager::GetInstance().IsKeyPressed('E')) {
            if (prev_released) {
                exploding_ = true;
                MakeExplosionActive();
            }
            prev_released = false;
        } else {
            prev_released = true;
        }
    }

    void BunnyNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void BunnyNode::SetPositions() {
        #include <iostream>
        // std::cout << triangle_pointers_[0]. << std::endl;
        for (int i = 0; i < particle_state_.positions.size()/3; i++) {
            auto positions = make_unique<PositionArray>();
            positions->push_back(particle_state_.positions[3 * i]);
            positions->push_back(particle_state_.positions[3 * i + 1]);
            positions->push_back(particle_state_.positions[3 * i + 2]);
            triangle_pointers_[i]->GetComponentPtr<RenderingComponent>()->GetVertexObjectPtr()->UpdatePositions(std::move(positions));
        }
    }

    void BunnyNode::SetNormals() {
        std::vector<std::vector<glm::vec3>> normal_around_vertices;
        for (int i = 0; i < bunny_positions_.size(); i++) {
            normal_around_vertices.push_back(std::vector<glm::vec3>());
        }

        for (int i = 0; i < bunny_indices_.size(); i += 3) {
            auto vertex1 = bunny_positions_[bunny_indices_[i]];
            auto vertex2 = bunny_positions_[bunny_indices_[i + 1]];
            auto vertex3 = bunny_positions_[bunny_indices_[i + 2]];
            auto cross_product = glm::cross(vertex3 - vertex2, vertex1 - vertex2);
            
            normal_around_vertices[bunny_indices_[i]].push_back(cross_product);
            normal_around_vertices[bunny_indices_[i + 1]].push_back(cross_product);
            normal_around_vertices[bunny_indices_[i + 2]].push_back(cross_product);
        }

        auto normals = make_unique<NormalArray>();
        for (int i = 0; i < bunny_positions_.size(); i++) {
            glm::vec3 normal(0.f, 0.f, 0.f);
            auto normal_around_vertex = normal_around_vertices[i];
            for (int j = 0; j < normal_around_vertex.size(); j++) {
            normal += normal_around_vertex[j];
            }
            normals->push_back(glm::normalize(normal));
        }
        bunny_mesh_->UpdateNormals(std::move(normals));
    }

    // void BunnyNode::SetColors() {
    //     auto colors = make_unique<ColorArray>();
    //     for (int i = 0; i < bunny_positions_.size(); i++) {
    //         glm::vec3 color(0.f, 0.f, 0.f);
    //         colors->push_back(glm::vec4(color[0], color[1], color[2], 0.f));
    //     }
    //     bunny_mesh_->UpdateColors(std::move(colors));
    // }

    void BunnyNode::MakeExplosionActive() {
        bunny_pointer_->SetActive(false);
        for (auto triangle_pointer: triangle_pointers_) {
            triangle_pointer->SetActive(true);
        }
    }

    void BunnyNode::ResetExplosionActive() {
        bunny_pointer_->SetActive(true);
        for (auto triangle_pointer: triangle_pointers_) {
            triangle_pointer->SetActive(false);
        }
    }
}