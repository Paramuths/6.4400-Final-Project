#ifndef COLLISION_NODE_H_
#define COLLISION_NODE_H_

#include "gloo/SceneNode.hpp"

#include "IntegratorType.hpp"
#include "IntegratorBase.hpp"
#include "PendulumSystem.hpp"
#include "SphereSystem.hpp"
#include "ParticleState.hpp"

#include "gloo/shaders/MyShader.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/Material.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
    class CollisionNode : public SceneNode {
        public:
        CollisionNode(IntegratorType integrator_type, float integration_step);
        void Update(double delta_time) override;

        private:
        void Init();
        void InitParticle();
        void InitCollision();
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
        ParticleState sphere_state_;

        PendulumSystem particle_system_;
        SphereSystem sphere_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

        // components
        SceneNode* cloth_pointer_;
        std::shared_ptr<MyShader> my_shader_;
        std::shared_ptr<VertexObject> cloth_mesh_;
        std::shared_ptr<Material> cloth_material_;

        SceneNode* sphere_pointer_;
        std::shared_ptr<PhongShader> phong_shader_;
        std::shared_ptr<VertexObject> sphere_mesh_;
        std::shared_ptr<Material> sphere_material_;
        float sphere_radius_ = 0.4f;

        // cloth information
        int num_cloth_row_ = 20;
        int num_cloth_col_ = 20;
        glm::vec3 offset_ = glm::vec3(0.2f, 1.2f, 0.f);
        glm::vec3 row_length_ = glm::vec3(0.f, -.05f, 0.f);
        glm::vec3 col_length_ = glm::vec3(.05f, 0.f, 0.f);
        float mass_ = .03f;
        float structural_spring_constant_ = 200.f;
        float shear_spring_constant_ = 100.f;
        float flex_spring_constant_ = 100.f;
    };
}

#endif