#ifndef SLIME_NODE_H_
#define SLIME_NODE_H_

#include "gloo/SceneNode.hpp"

#include "IntegratorType.hpp"
#include "IntegratorBase.hpp"
#include "PendulumSystem.hpp"
#include "ParticleState.hpp"

#include "gloo/shaders/MyShader.hpp"
#include "gloo/Material.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
    class SlimeNode : public SceneNode {
        public:
        SlimeNode(IntegratorType integrator_type, float integration_step);
        void Update(double delta_time) override;

        private:
        void Init();
        void InitParticle();
        void InitSlime();
        void InitSystem();
        void Advance(float start_time);
        void SetPositions();
        void SetIndices();
        void SetNormals();
        void SetColors();
        // helper
        int IndexOf(int r, int c);

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        PendulumSystem particle_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

        // components
        SceneNode* slime_pointer_;
        std::shared_ptr<MyShader> my_shader_;
        std::shared_ptr<VertexObject> slime_mesh_;
        std::shared_ptr<Material> slime_material_;

        // slime information
        glm::vec3 offset_ = glm::vec3(4.f, 2.f, 0.f);

        int slime_height_ = 20;
        int slime_length_ = 20;
        float scale_ = .01f;
        glm::vec3 first_axis_ = scale_ * glm::normalize(glm::vec3(1.f, 0.f, 0.f));
        glm::vec3 second_axis_ = scale_ * glm::normalize(glm::vec3(.5f, 0.f, .866f));
        glm::vec3 third_axis_ = 10.f * scale_ * glm::normalize(glm::vec3(.5f, 0.1f, .289f));
        float mass_ = .1f;
        float structural_spring_constant_ = 100.f;
        float shear_spring_constant_ = 100.f;
        float flex_spring_constant_ = 100.f;
    };
}

#endif