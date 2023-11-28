#include "ClothNode.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
    ClothNode::ClothNode(IntegratorType integrator_type, float integration_step): 
        integration_step_(integration_step) {
        // Instantiate MyShader, VertexObject, Material
        my_shader_ = std::make_shared<MyShader>();
        cloth_mesh_ = std::make_shared<VertexObject>();
        cloth_material_ = std::make_shared<Material>(glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>(integrator_type);
        particle_system_ = ClothSystem();
        Init();
    }

    void ClothNode::Update(double delta_time) {
        int num_steps = (delta_time + carrier_time_step_)/integration_step_;
        carrier_time_step_ = delta_time + carrier_time_step_ - float(num_steps * integration_step_);
        for (int i = 0; i < num_steps; i++) {
            Advance(float(i * integration_step_));
        }
        // Toggle 'R' to reset
        // Toggle 'W' to start/stop wind
        static bool prev_released = true;
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            if (prev_released) {
                InitParticle();
            }
            prev_released = false;
        } else if (InputManager::GetInstance().IsKeyPressed('W')) {
            if (prev_released) {
                particle_system_.ToggleEnableWind();
            }
            prev_released = false;
        } else {
            prev_released = true;
        }
        SetPositions();
        SetNormals();
    }

    int ClothNode::IndexOf(int r, int c) {
        return r * num_cloth_col_ + c;
    }

    void ClothNode::Init() {
        InitParticle();
        InitCloth();
        InitSystem();
    }

    void ClothNode::InitParticle() {
        // Clear Stored Information
        particle_state_.positions = {};
        particle_state_.velocities = {};

        for (int r = 0; r < num_cloth_row_; r++) {
            for (int c = 0; c < num_cloth_col_; c++) {
                particle_state_.positions.push_back(float(r) * row_length_ + float(c) * col_length_ + offset_);
                particle_state_.velocities.push_back(glm::vec3(0));
            }
        }
    }

    void ClothNode::InitCloth() {
        auto cloth_node = make_unique<SceneNode>();
        cloth_node->CreateComponent<ShadingComponent>(my_shader_);
        cloth_node->CreateComponent<MaterialComponent>(cloth_material_);

        auto positions = make_unique<PositionArray>();
        for (auto position: particle_state_.positions) {
            positions->push_back(position);
        }

        SetPositions();
        SetIndices();
        SetNormals();
        SetColors();
        cloth_node->CreateComponent<RenderingComponent>(cloth_mesh_).SetDrawMode(DrawMode::Triangles);

        cloth_pointer_ = cloth_node.get();
        AddChild(std::move(cloth_node));
    }

    void ClothNode::InitSystem() {
        // Mass
        std::vector<float> particle_mass(num_cloth_row_ * num_cloth_col_, mass_);
        particle_system_.SetMass(particle_mass);

        // Fixed the top corners
        particle_system_.SetFixedParticle({0, num_cloth_col_ - 1});

        // Spring
        // Structural Spring
        auto row_rest_length = glm::length(row_length_);
        auto col_rest_length = glm::length(col_length_);
        std::vector<std::tuple<int, int, float, float>> spring_force;
        for (int r = 0; r < num_cloth_row_ - 1; r++) {
            for (int c = 0; c < num_cloth_col_ - 1; c++) {
                spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r + 1, c), structural_spring_constant_, row_rest_length));
                spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r, c + 1), structural_spring_constant_, col_rest_length));
            }
        }

        for (int r = 0; r < num_cloth_row_ - 1; r++) {
            spring_force.push_back(std::make_tuple(IndexOf(r, num_cloth_col_ - 1), IndexOf(r + 1, num_cloth_col_ - 1), structural_spring_constant_, row_rest_length));
        }

        for (int c = 0; c < num_cloth_col_ - 1; c++) {
            spring_force.push_back(std::make_tuple(IndexOf(num_cloth_row_ - 1, c), IndexOf(num_cloth_row_ - 1, c + 1), structural_spring_constant_, col_rest_length));
        }
        // Shear Spring
        auto diagonal_rest_length = glm::length(row_length_ + col_length_);
        for (int r = 0; r < num_cloth_row_ - 1; r++) {
            for (int c = 0; c < num_cloth_col_ - 1; c++) {
                spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r + 1, c + 1), shear_spring_constant_, diagonal_rest_length));
                spring_force.push_back(std::make_tuple(IndexOf(r + 1, c), IndexOf(r, c + 1), shear_spring_constant_, diagonal_rest_length));
            }
        }
        // Flex Spring
        auto flex_row_rest_length = 2 * row_rest_length;
        auto flex_col_rest_length = 2 * col_rest_length;
        for (int r = 0; r < num_cloth_row_ - 2; r++) {
            for (int c = 0; c < num_cloth_col_ - 2; c++) {
                spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r + 2, c), flex_spring_constant_, flex_row_rest_length));
                spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r, c + 2), flex_spring_constant_, flex_col_rest_length));
            }
        }

        for (int r = 0; r < num_cloth_row_ - 2; r++) {
            spring_force.push_back(std::make_tuple(IndexOf(r, num_cloth_col_ - 2), IndexOf(r + 2, num_cloth_col_ - 2), flex_spring_constant_, flex_row_rest_length));
            spring_force.push_back(std::make_tuple(IndexOf(r, num_cloth_col_ - 1), IndexOf(r + 2, num_cloth_col_ - 1), flex_spring_constant_, flex_row_rest_length));
        }

        for (int c = 0; c < num_cloth_col_ - 2; c++) {
            spring_force.push_back(std::make_tuple(IndexOf(num_cloth_row_ - 2, c), IndexOf(num_cloth_row_ - 2, c + 2), flex_spring_constant_, flex_col_rest_length));
            spring_force.push_back(std::make_tuple(IndexOf(num_cloth_row_ - 1, c), IndexOf(num_cloth_row_ - 1, c + 2), flex_spring_constant_, flex_col_rest_length));
        }

        particle_system_.SetSpringForce(spring_force);
    }

    void ClothNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void ClothNode::SetPositions() {
        auto positions = make_unique<PositionArray>();
        for (auto position: particle_state_.positions) {
            positions->push_back(position);
        }
        cloth_mesh_->UpdatePositions(std::move(positions));
    }

    void ClothNode::SetIndices() {
        auto indices = make_unique<IndexArray>();
        for (int r = 0; r < num_cloth_row_ - 1; r++) {
            for (int c = 0; c < num_cloth_col_ - 1; c++) {
                for (auto i: {0, 1}) {
                    indices->push_back(IndexOf(r + i, c + i));
                    indices->push_back(IndexOf(r + 1, c));
                    indices->push_back(IndexOf(r, c + 1));
                }
            }
        }
        cloth_mesh_->UpdateIndices(std::move(indices));
    }

    void ClothNode::SetNormals() {
        std::vector<glm::vec3> unweighted_normals(num_cloth_row_ * num_cloth_col_, glm::vec3(0.f));
        for (int r = 0; r < num_cloth_row_ - 1; r++) {
            for (int c = 0; c < num_cloth_col_ - 1; c++) {
                auto vec1 = particle_state_.positions[IndexOf(r, c + 1)] - particle_state_.positions[IndexOf(r, c)];
                auto vec2 = particle_state_.positions[IndexOf(r + 1, c)] - particle_state_.positions[IndexOf(r, c)];
                auto first_normal_surface = glm::cross(vec1, vec2);
                unweighted_normals[IndexOf(r, c)] += first_normal_surface;
                unweighted_normals[IndexOf(r, c + 1)] += first_normal_surface;
                unweighted_normals[IndexOf(r + 1, c)] += first_normal_surface;

                auto vec3 = particle_state_.positions[IndexOf(r, c + 1)] - particle_state_.positions[IndexOf(r + 1, c + 1)];
                auto vec4 = particle_state_.positions[IndexOf(r + 1, c)] - particle_state_.positions[IndexOf(r + 1, c + 1)];
                auto second_norface = glm::cross(vec4, vec3);
                unweighted_normals[IndexOf(r + 1, c + 1)] += second_norface;
                unweighted_normals[IndexOf(r, c + 1)] += second_norface;
                unweighted_normals[IndexOf(r + 1, c)] += second_norface;
            }
        }

        auto normals = make_unique<NormalArray>();
        for (auto unweighted_normal: unweighted_normals) {
            normals->push_back(glm::normalize(unweighted_normal));
        }
        cloth_mesh_->UpdateNormals(std::move(normals));
    }

    void ClothNode::SetColors() {
        auto colors = make_unique<ColorArray>();
        for (int r = 0; r < num_cloth_row_; r++) {
            for (int c = 0; c < num_cloth_col_; c++) {
                auto color = ((r%2 == 0) && (c%2 == 0))? glm::vec4(1.f, 1.f, 0.f, 1.f): glm::vec4(0.f, 0.f, 0.f, 1.f);
                colors->push_back(color);
            }
        }
        cloth_mesh_->UpdateColors(std::move(colors));
    }
}