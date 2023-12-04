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
        void InitSphere();
        void InitSystem();
        void Advance(float start_time);
        void SetPositions();
        void SetNormals();
        // void SetColors();
        void MakeExplosionActive();
        void ResetExplosionActive();

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        ExplodingSystem particle_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

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
    };
}

#endif