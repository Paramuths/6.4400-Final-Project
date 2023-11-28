#ifndef BUNNY_NODE_H_
#define BUNNY_NODE_H_

#include "gloo/SceneNode.hpp"

#include "IntegratorType.hpp"
#include "IntegratorBase.hpp"
#include "PendulumSystem.hpp"
#include "SimpleParticleSystem.hpp"
#include "ParticleState.hpp"

#include "gloo/shaders/MyShader.hpp"
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
        void InitSystem();
        void Advance(float start_time);
        void SetPositions();
        void SetNormals();
        void SetColors();

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        SimpleParticleSystem particle_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

        // components
        SceneNode* bunny_pointer_;
        PositionArray bunny_positions_;
        std::shared_ptr<MyShader> my_shader_;
        std::shared_ptr<VertexObject> bunny_mesh_;
        std::shared_ptr<Material> bunny_material_;
    };
}

#endif