#ifndef CLOTH_NODE_H_
#define CLOTH_NODE_H_

#include "gloo/SceneNode.hpp"

#include "IntegratorType.hpp"
#include "IntegratorBase.hpp"
#include "ClothSystem.hpp"
#include "ParticleState.hpp"

#include "gloo/shaders/MyShader.hpp"
#include "gloo/Material.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
    class ClothNode : public SceneNode {
        public:
        ClothNode(float integration_step);
        void Update(double delta_time) override;

        private:
        void Init();
        void InitParticle();
        void InitCloth();
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
        ClothSystem particle_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

        // components
        SceneNode* cloth_pointer_;
        std::shared_ptr<MyShader> my_shader_;
        std::shared_ptr<VertexObject> cloth_mesh_;
        std::shared_ptr<Material> cloth_material_;

        // cloth information
        int num_cloth_row_ = 30;
        int num_cloth_col_ = 30;
        glm::vec3 offset_ = glm::vec3(2.f, 2.f, 0.f);
        glm::vec3 row_length_ = glm::vec3(0.f, -.05f, 0.f);
        glm::vec3 col_length_ = glm::vec3(.05f, 0.f, 0.f);
        float mass_ = .03f;
        float structural_spring_constant_ = 200.f;
        float shear_spring_constant_ = 100.f;
        float flex_spring_constant_ = 100.f;
    };
}

#endif