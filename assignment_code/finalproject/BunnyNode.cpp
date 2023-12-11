#include "BunnyNode.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include <fstream>
#include <cmath>

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
        bunny_scale_ = glm::vec3(1.f);

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
            auto pos1 = bunny_positions_[bunny_indices_[i]];
            auto pos2 = bunny_positions_[bunny_indices_[i + 1]];
            auto pos3 = bunny_positions_[bunny_indices_[i + 2]];

            auto nor1 = bunny_normals_[bunny_indices_[i]];
            auto nor2 = bunny_normals_[bunny_indices_[i + 1]];
            auto nor3 = bunny_normals_[bunny_indices_[i + 2]];

            for (int i1 = 0; i1 < triangle_scale_; i1++) {
                for (int i2 = 0; i2 < triangle_scale_ - i1; i2++) {
                    float f1 = float(i1);
                    float f2 = float(i2);
                    float f3 = float(triangle_scale_) - f1 - f2;

                    auto pos = triangle_multiplier_ * (f1 * pos1 + f2 * pos2 + f3 * pos3);
                    auto nor = triangle_multiplier_ * (f1 * nor1 + f2 * nor2 + f3 * nor3);

                    particle_state_.positions.push_back(pos);
                    particle_state_.positions.push_back(pos + triangle_multiplier_ * (pos1 - pos3));
                    particle_state_.positions.push_back(pos + triangle_multiplier_ * (pos2 - pos3));

                    particles_initial_normal_.push_back(glm::normalize(nor));
                    particles_initial_normal_.push_back(glm::normalize(nor + triangle_multiplier_ * (nor1 - nor3)));
                    particles_initial_normal_.push_back(glm::normalize(nor + triangle_multiplier_ * (nor2 - nor3)));

                    particle_state_.velocities.push_back(glm::vec3(0.f,0.f,0.f));
                    particle_state_.velocities.push_back(glm::vec3(0.f,0.f,0.f));
                    particle_state_.velocities.push_back(glm::vec3(0.f,0.f,0.f));
                    if (f3 > 1.1f) {
                        particle_state_.positions.push_back(pos + triangle_multiplier_ * (pos1 + pos2 - 2.f * pos3));
                        particle_state_.positions.push_back(pos + triangle_multiplier_ * (pos1 - pos3));
                        particle_state_.positions.push_back(pos + triangle_multiplier_ * (pos2 - pos3));

                        particles_initial_normal_.push_back(glm::normalize(nor + triangle_multiplier_ * (nor1 + nor2 - 2.f * nor3)));
                        particles_initial_normal_.push_back(glm::normalize(nor + triangle_multiplier_ * (nor1 - nor3)));
                        particles_initial_normal_.push_back(glm::normalize(nor + triangle_multiplier_ * (nor2 - nor3)));

                        particle_state_.velocities.push_back(glm::vec3(0.f,0.f,0.f));
                        particle_state_.velocities.push_back(glm::vec3(0.f,0.f,0.f));
                        particle_state_.velocities.push_back(glm::vec3(0.f,0.f,0.f));
                    } 
                }
            }
        }
        for(int i=0; i< particle_state_.positions.size(); i+=3)
        {
            isSmashed.push_back(false);
        }
        exploding_ = false;
    }

    void BunnyNode::InitTriangle() {
        phong_shader_ = std::make_shared<PhongShader>();
        triangle_material_ = std::make_shared<Material>(glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(1.f, 1.f, 1.f),
                                                    glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);

        for (int i = 0; i < particles_initial_normal_.size(); i += 3) {
            auto triangle_node = make_unique<SceneNode>();
            triangle_node->CreateComponent<ShadingComponent>(phong_shader_);
            triangle_node->CreateComponent<MaterialComponent>(triangle_material_);

            auto triangle_mesh = std::make_shared<VertexObject>();

            auto positions = make_unique<PositionArray>();
            positions->push_back(particle_state_.positions[i]);
            positions->push_back(particle_state_.positions[i + 1]);
            positions->push_back(particle_state_.positions[i + 2]);

            auto normals = make_unique<NormalArray>();
            normals->push_back(particles_initial_normal_[i]);
            normals->push_back(particles_initial_normal_[i + 1]);
            normals->push_back(particles_initial_normal_[i + 2]);

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
        double slow_factor = 1;
        double delta_time_=delta_time / slow_factor;
        if (exploding_) {
            int num_steps = (delta_time_ + carrier_time_step_)/integration_step_;
            carrier_time_step_ = delta_time_ + carrier_time_step_ - float(num_steps * integration_step_);
            for (int i = 0; i < num_steps; i++) {
                Advance(used_time_ + float(i * integration_step_));
            }
            used_time_ += float(num_steps * integration_step_);
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
        for (int i = 0; i < particle_state_.positions.size(); i+=3) {
            if(isSmashed[i/3]) continue;
            std::pair<bool, std::pair<glm::vec3, glm::vec3>> result = CheckIntersect(i, start_time);
            if(result.first && glm::length(result.second.second) > 0.001){
                isSmashed[i/3] = true;
                particle_state_.velocities[i] += ball_velocity;
                particle_state_.velocities[i+1] += ball_velocity;
                particle_state_.velocities[i+2] += ball_velocity;
                glm::vec3 face_normal = glm::normalize(result.second.second);
                float multiplier = pow(abs(glm::dot(face_normal, ball_velocity / ball_speed)), multiplier_exponent) * ball_speed;
                particle_system_.AddBomb(start_time, result.second.first, multiplier);
                // std::cout << start_time << std::endl;
                  // Create Explosion Center
                // auto expl_center = std::shared_ptr<VertexObject>(PrimitiveFactory::CreateSphere(0.01f, 20, 20));
                // auto shader_ = std::make_shared<PhongShader>();
                // glm::vec3 ctrl_color(1.f,1.f,0.f);
                // auto expl_material = std::make_shared<Material>(ctrl_color, ctrl_color, glm::vec3(0.4f), 20.0f);

                // auto expl_node = make_unique<SceneNode>();
                // expl_node->CreateComponent<ShadingComponent>(shader_);
                // expl_node->CreateComponent<RenderingComponent>(expl_center);
                // expl_node->CreateComponent<MaterialComponent>(expl_material);
                // expl_node->GetTransform().SetPosition(result.second.first);
                // AddChild(std::move(expl_node));
            }
            particle_state_.positions[i] += result.second.second;
            particle_state_.positions[i+1] += result.second.second;
            particle_state_.positions[i+2] += result.second.second;
        }

        auto next_state = integrator_->Integrate(particle_system_, particle_state_, start_time, integration_step_);
        particle_state_ = next_state;
    }

    void BunnyNode::SetPositions() {
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
        used_time_ = 0.0f;
    }

    void BunnyNode::ResetExplosionActive() {
        bunny_pointer_->SetActive(true);
        particle_system_.ClearBomb();
        for(int i=0;i<isSmashed.size();i++) isSmashed[i] = false;
        for (auto triangle_pointer: triangle_pointers_) {
            triangle_pointer->SetActive(false);
        }
    }

    // retunr {sphere intersect the triangle with vertices p1,p2,p3 or not {point of contact, displacement of the triangle to avoid overlap}}
    std::pair<bool, std::pair<glm::vec3, glm::vec3>> BunnyNode::CheckIntersect(int idx, float time) {
        glm::vec3 p1 = particle_state_.positions[idx];
        glm::vec3 p2 = particle_state_.positions[idx+1];
        glm::vec3 p3 = particle_state_.positions[idx+2];
        glm::vec3 o = ball_start + time * ball_velocity;

        glm::vec3 v = o - p3;
        glm::vec3 v1 = p1-p3;
        glm::vec3 v2 = p2-p3;

        float q11 = glm::dot(v1, v1);
        float q12 = glm::dot(v1, v2);
        float q22 = glm::dot(v2, v2);
        glm::mat2 Q =glm::mat2(q11, q12, q12, q22);
        glm::vec2 coefs = glm::inverse(Q) * glm::vec2(glm::dot(v, v1), glm::dot(v, v2));
        float r1 = coefs[0];
        float r2 = coefs[1];
        glm::vec3 plane_vec = r1 * v1 + r2 * v2;
        glm::vec3 perp_vec = v - plane_vec;
        float perp_length = glm::length(perp_vec);

        if (perp_length > ball_radius) return {false, {glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,0.f,0.f)}};

        if(r1 >= 0 && r2 >= 0 && r1+r2 <= 1) return {true, {p3+plane_vec, (1 - ball_radius / perp_length)  * perp_vec}};

        // calc closest point to sphere if any (must be on edge)
        auto temp_data = CalcClosest(p1, p2, o);
        float min_dist = temp_data.second;
        glm::vec3 hit_position = temp_data.first;
        temp_data = CalcClosest(p2,p3,o);
        if(temp_data.second < min_dist){
            min_dist = temp_data.second;
            hit_position = temp_data.first;
        }
        temp_data = CalcClosest(p3,p1,o);
        if(temp_data.second < min_dist){
            min_dist = temp_data.second;
            hit_position = temp_data.first;
        }
        if(min_dist > ball_radius) return {false, {glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,0.f,0.f)}};
        float last_perp_dist = pow(ball_radius * ball_radius - glm::dot(plane_vec, plane_vec), 0.5);
        return {true, {hit_position, (1 - last_perp_dist / perp_length) * perp_vec}};
    }

    std::pair<glm::vec3, float> BunnyNode::CalcClosest(glm::vec3 a, glm::vec3 b, glm::vec3 c){
        glm::vec3 ab = b-a;
        float l = glm::length(ab);
        ab = (1.0f/ l) * ab;
        float d = glm::dot(c, ab);
        if (d<0) d=0;
        if(d>l) d=l;
        glm::vec3 landing = a + d * ab;
        return {landing, glm::length(c - landing)};
    }
}