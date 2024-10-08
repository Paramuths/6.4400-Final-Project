#ifndef BUNNY_NODE_H_
#define BUNNY_NODE_H_

#include "gloo/SceneNode.hpp"
#include "IntegratorBase.hpp"
#include "ExplodingSystem.hpp"
#include "ParticleState.hpp"
#include "gloo/shaders/MyShader.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/Material.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
    class BunnyNode : public SceneNode {
        public:
        BunnyNode(float integration_step);
        void Update(double delta_time) override;

        private:
        void Init();
        void InitParticle();
        void InitBunny();
        void InitTriangle();
        void InitSystem();
        void Advance(float start_time);
        void SetPositions();
        void SetNormals();
        // void SetColors();
        void MakeExplosionActive();
        void ResetExplosionActive();

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        NormalArray particles_initial_normal_;
        ExplodingSystem particle_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;
        float used_time_ = 0.f;

        // components
        SceneNode* bunny_pointer_;
        PositionArray bunny_positions_;
        NormalArray bunny_normals_;
        IndexArray bunny_indices_;

        // std::shared_ptr<MyShader> my_shader_;
        std::shared_ptr<PhongShader> bunny_shader_;
        std::shared_ptr<VertexObject> bunny_mesh_;
        std::shared_ptr<Material> bunny_material_;
        glm::vec3 bunny_scale_;

        std::vector<SceneNode*> triangle_pointers_;
        std::shared_ptr<PhongShader> phong_shader_;
        std::shared_ptr<Material> triangle_material_;

        bool exploding_;
        
        int triangle_scale_ = 1;
        float triangle_multiplier_ = 1.f / float (triangle_scale_);

        // ball
        glm::vec3 ball_start = glm::vec3(-0.67f, 0.2f, 0.0f);
        glm::vec3 ball_velocity = glm::vec3(0.8f,0.0f,0.0f);
        float ball_radius = 0.05f;
        float ball_speed = glm::length(ball_velocity);

        // ball - bunny calculation
        std::pair<bool, std::pair<glm::vec3, glm::vec3>> CheckIntersect(int idx, float time);
        std::pair<glm::vec3, float> CalcClosest(glm::vec3 a, glm::vec3 b, glm::vec3 c);
        std::vector<bool> isSmashed;
        float multiplier_exponent = 20.0f;
    };
}

#endif