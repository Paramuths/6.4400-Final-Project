#ifndef PARTICLE_NODE_H_
#define PARTICLE_NODE_H_

#include "gloo/SceneNode.hpp"

#include "IntegratorType.hpp"
#include "IntegratorBase.hpp"
#include "SimpleParticleSystem.hpp"
#include "ParticleState.hpp"

namespace GLOO {
    class ParticleNode : public SceneNode {
        public:
        ParticleNode(float integration_step);
        void Update(double delta_time) override;

        private:
        void Advance(float start_time);
        void SetPosition();
        void InitParticle();

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        SimpleParticleSystem particle_system_;
        float integration_step_;

        float carrier_time_step_ = 0.f;

        SceneNode* sphere_node_;
    };
}

#endif