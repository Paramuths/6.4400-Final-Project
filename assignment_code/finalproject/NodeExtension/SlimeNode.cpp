#include "SlimeNode.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
    SlimeNode::SlimeNode(IntegratorType integrator_type, float integration_step): 
        integration_step_(integration_step) {
        // Instantiate MyShader, VertexObject, Material
        my_shader_ = std::make_shared<MyShader>();
        slime_mesh_ = std::make_shared<VertexObject>();
        slime_material_ = std::make_shared<Material>(glm::vec3(.0f, .0f, .0f),
                                                    glm::vec3(.0f, .0f, .0f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
        integrator_ = IntegratorFactory::CreateIntegrator<ParticleSystemBase, ParticleState>(integrator_type);
        particle_system_ = PendulumSystem();
        Init();
    }

    void SlimeNode::Update(double delta_time) {
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

    void SlimeNode::Init() {
        InitParticle();
        InitSlime();
        InitSystem();
    }

    void SlimeNode::InitParticle() {
        // Clear Stored Information
        particle_state_.positions = {};
        particle_state_.velocities = {}; //= std::vector<glm::vec3>((slime_height_ + 1) * slime_length_ * 4, glm::vec3(0.f, -2.f, 0.f));

        auto normal_speed = glm::vec3(0.f, -2.f, 0.f);
        auto height = float(slime_height_);
        for (int x = 0; x <= slime_height_; x++) {
            auto i = float(x);
            for (int y = 0; y < slime_length_; y++) {
                auto j = float(y);
                particle_state_.positions.push_back(i * third_axis_ + j * first_axis_ + offset_);
                particle_state_.velocities.push_back( (height - i) * normal_speed);
            }
            for (int y = 0; y < slime_length_; y++) {
                auto j = float(y);
                particle_state_.positions.push_back(i * third_axis_ + float(slime_length_) * first_axis_ + j * second_axis_ + offset_);
                particle_state_.velocities.push_back( (height - i) * normal_speed);
            }
            for (int y = slime_length_; y > 0; y--) {
                auto j = float(y);
                particle_state_.positions.push_back(i * third_axis_ + j * first_axis_ + float(slime_length_) * second_axis_ + offset_);
                particle_state_.velocities.push_back( (height - i) * normal_speed);
            }
            for (int y = slime_length_; y > 0; y--) {
                auto j = float(y);
                particle_state_.positions.push_back(i * third_axis_ + j * second_axis_ + offset_);
                particle_state_.velocities.push_back( (height - i) * normal_speed);
            }
        }
    }

    void SlimeNode::InitSlime() {
        auto slime_node = make_unique<SceneNode>();
        slime_node->CreateComponent<ShadingComponent>(my_shader_);
        slime_node->CreateComponent<MaterialComponent>(slime_material_);

        auto positions = make_unique<PositionArray>();
        for (auto position: particle_state_.positions) {
            positions->push_back(position);
        }

        SetPositions();
        SetIndices();
        SetNormals();
        SetColors();
        slime_node->CreateComponent<RenderingComponent>(slime_mesh_).SetDrawMode(DrawMode::Triangles);

        slime_pointer_ = slime_node.get();
        AddChild(std::move(slime_node));
    }

    void SlimeNode::InitSystem() {
        // Mass
        std::vector<float> particle_mass((slime_height_ + 1) * slime_length_ * 4, mass_);
        particle_system_.SetMass(particle_mass);

        // Fixed the top corners
        std::vector<int> fixed_points;
        for (int i = 0; i < 4 * slime_length_; i++) {
            fixed_points.push_back(i + 4 * slime_height_ * slime_length_);
        }
        particle_system_.SetFixedParticle(fixed_points);

        // Spring
        // Structural Spring
        // auto row_rest_length = glm::length(row_length_);
        // auto col_rest_length = glm::length(col_length_);
        std::vector<std::tuple<int, int, float, float>> spring_force;
        // spring_force.push_back(std::make_tuple(0, 1, structural_spring_constant_, scale_));
        // spring_force.push_back(std::make_tuple(0, 2, structural_spring_constant_, scale_));
        // spring_force.push_back(std::make_tuple(0, 3, structural_spring_constant_, scale_));
        // spring_force.push_back(std::make_tuple(1, 2, structural_spring_constant_, scale_));
        // spring_force.push_back(std::make_tuple(1, 3, structural_spring_constant_, scale_));
        // spring_force.push_back(std::make_tuple(2, 3, structural_spring_constant_, scale_));

        auto length_level = 4 * slime_length_;
        for (int i = 0; i < slime_height_; i++) {
            for (int j = 0; j < slime_length_ * 4; j++) {
                spring_force.push_back(std::make_tuple(i * length_level + j, (i + 1) * length_level + j, structural_spring_constant_, 0.001f * scale_));
            }
        }

        // for (int r = 0; r < num_slime_row_ - 1; r++) {
        //     for (int c = 0; c < num_slime_col_ - 1; c++) {
        //         spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r + 1, c), structural_spring_constant_, row_rest_length));
        //         spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r, c + 1), structural_spring_constant_, col_rest_length));
        //     }
        // }

        // for (int r = 0; r < num_slime_row_ - 1; r++) {
        //     spring_force.push_back(std::make_tuple(IndexOf(r, num_slime_col_ - 1), IndexOf(r + 1, num_slime_col_ - 1), structural_spring_constant_, row_rest_length));
        // }

        // for (int c = 0; c < num_slime_col_ - 1; c++) {
        //     spring_force.push_back(std::make_tuple(IndexOf(num_slime_row_ - 1, c), IndexOf(num_slime_row_ - 1, c + 1), structural_spring_constant_, col_rest_length));
        // }
        // // Shear Spring
        // auto diagonal_rest_length = glm::length(row_length_ + col_length_);
        // for (int r = 0; r < num_slime_row_ - 1; r++) {
        //     for (int c = 0; c < num_slime_col_ - 1; c++) {
        //         spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r + 1, c + 1), shear_spring_constant_, diagonal_rest_length));
        //         spring_force.push_back(std::make_tuple(IndexOf(r + 1, c), IndexOf(r, c + 1), shear_spring_constant_, diagonal_rest_length));
        //     }
        // }
        // Flex Spring
        // auto flex_row_rest_length = 2 * row_rest_length;
        // auto flex_col_rest_length = 2 * col_rest_length;
        // for (int r = 0; r < num_slime_row_ - 2; r++) {
        //     for (int c = 0; c < num_slime_col_ - 2; c++) {
        //         spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r + 2, c), flex_spring_constant_, flex_row_rest_length));
        //         spring_force.push_back(std::make_tuple(IndexOf(r, c), IndexOf(r, c + 2), flex_spring_constant_, flex_col_rest_length));
        //     }
        // }

        // for (int r = 0; r < num_slime_row_ - 2; r++) {
        //     spring_force.push_back(std::make_tuple(IndexOf(r, num_slime_col_ - 2), IndexOf(r + 2, num_slime_col_ - 2), flex_spring_constant_, flex_row_rest_length));
        //     spring_force.push_back(std::make_tuple(IndexOf(r, num_slime_col_ - 1), IndexOf(r + 2, num_slime_col_ - 1), flex_spring_constant_, flex_row_rest_length));
        // }

        // for (int c = 0; c < num_slime_col_ - 2; c++) {
        //     spring_force.push_back(std::make_tuple(IndexOf(num_slime_row_ - 2, c), IndexOf(num_slime_row_ - 2, c + 2), flex_spring_constant_, flex_col_rest_length));
        //     spring_force.push_back(std::make_tuple(IndexOf(num_slime_row_ - 1, c), IndexOf(num_slime_row_ - 1, c + 2), flex_spring_constant_, flex_col_rest_length));
        // }

        particle_system_.SetSpringForce(spring_force);
    }

    void SlimeNode::Advance(float start_time) {
        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void SlimeNode::SetPositions() {
        auto positions = make_unique<PositionArray>();
        for (auto position: particle_state_.positions) {
            positions->push_back(position);
        }
        slime_mesh_->UpdatePositions(std::move(positions));
    }

    void SlimeNode::SetIndices() {
        auto indices = make_unique<IndexArray>();
        auto length_level = 4 * slime_length_;
        for (int i = 0; i < slime_height_; i++) {
            for (int j = 0; j < slime_length_ * 4 - 1; j++) {
                indices->push_back(i * length_level + j);
                indices->push_back(i * length_level + j + 1);
                indices->push_back((i + 1) * length_level + j);

                indices->push_back(i * length_level + j + 1);
                indices->push_back((i + 1) * length_level + j);
                indices->push_back((i + 1) * length_level + j + 1);
            }
            indices->push_back(i * length_level + length_level - 1);
            indices->push_back(i * length_level);
            indices->push_back((i + 1) * length_level + length_level - 1);

            indices->push_back(i * length_level);
            indices->push_back((i + 1) * length_level + length_level - 1);
            indices->push_back((i + 1) * length_level );
        }

        slime_mesh_->UpdateIndices(std::move(indices));
    }

    void SlimeNode::SetNormals() {
        std::vector<glm::vec3> unweighted_normals((slime_height_ + 1) * slime_length_ * 4, glm::vec3(0.f));

        auto length_level = float(4 * slime_length_);
        for (int x = 0; x < slime_height_; x++) {
            auto i = float(x);
            for (int y = 0; y < slime_length_ * 4 - 1; y++) {
                auto j = float(y);
                auto vec1 = particle_state_.positions[(i + 1) * length_level + j] - particle_state_.positions[i * length_level + j];
                auto vec2 = particle_state_.positions[i * length_level + j + 1] - particle_state_.positions[i * length_level + j];
                auto normal_surface = glm::cross(vec1, vec2);

                unweighted_normals[i * length_level + j] += normal_surface;
                unweighted_normals[i * length_level + j + 1] += normal_surface;
                unweighted_normals[(i + 1) * length_level + j] += normal_surface;

                vec1 = particle_state_.positions[i * length_level + j + 1] - particle_state_.positions[(i + 1) * length_level + j + 1];
                vec2 = particle_state_.positions[(i + 1) * length_level + j] - particle_state_.positions[(i + 1) * length_level + j + 1];
                normal_surface = glm::cross(vec1, vec2);

                unweighted_normals[i * length_level + j + 1] += normal_surface;
                unweighted_normals[(i + 1) * length_level + j] += normal_surface;
                unweighted_normals[(i + 1) * length_level + j + 1] += normal_surface;
            }

            auto vec1 = particle_state_.positions[(i + 1) * length_level + slime_length_ * 4 - 1] - particle_state_.positions[i * length_level + length_level - 1];
            auto vec2 = particle_state_.positions[i * length_level] - particle_state_.positions[i * length_level + length_level - 1];
            auto normal_surface = glm::cross(vec1, vec2);

            unweighted_normals[i * length_level + length_level - 1] += normal_surface;
            unweighted_normals[i * length_level] += normal_surface;
            unweighted_normals[(i + 1) * length_level + length_level - 1] += normal_surface;

            vec1 = particle_state_.positions[i * length_level] - particle_state_.positions[(i + 1) * length_level ];
            vec2 = particle_state_.positions[(i + 1) * length_level + length_level - 1] - particle_state_.positions[(i + 1) * length_level ];
            normal_surface = glm::cross(vec1, vec2);

            unweighted_normals[i * length_level] += normal_surface;
            unweighted_normals[(i + 1) * length_level + length_level - 1] += normal_surface;
            unweighted_normals[(i + 1) * length_level ] += normal_surface;
        }

        auto normals = make_unique<NormalArray>();
        for (auto unweighted_normal: unweighted_normals) {
            normals->push_back(glm::normalize(unweighted_normal));
        }
        slime_mesh_->UpdateNormals(std::move(normals));
    }

    void SlimeNode::SetColors() {
        std::vector<glm::vec4> color_list = {glm::vec4(.2f, .2f, 0.f, 1.f), 
                                                glm::vec4(.1f, .4f, 0.2f, 1.f),
                                                glm::vec4(.3f, .0f, 1.f, 1.f), 
                                                glm::vec4(.4f, .4f, .4f, 1.f)}; 
        auto colors = make_unique<ColorArray>();
        for (int i = 0; i < slime_height_ + 1; i++) {
            for (int s = 0; s < 4; s++) {
                auto color = color_list[s];
                for (int j = 0; j < slime_length_; j++) {
                    colors->push_back(color);
                }
            }
        }
        slime_mesh_->UpdateColors(std::move(colors));
    }
}