#ifndef PENDULUM_NODE_H_
#define PENDULUM_NODE_H_

#include "gloo/SceneNode.hpp"

#include "IntegratorType.hpp"
#include "IntegratorBase.hpp"
#include "PendulumSystem.hpp"
#include "ParticleState.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/Material.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
    class PendulumNode : public SceneNode {
        public:
        PendulumNode(float integration_step);
        void Update(double delta_time) override;

        private:
        void Advance(float start_time);
        void SetPosition();
        void InitParticle();
        void Init();

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        PendulumSystem particle_system_;
        std::vector<float> particle_mass_;
        std::vector<SceneNode*> particle_pointer_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

        // spring
        std::vector<std::tuple<int, int, float, float>> spring_force_;

        // components
        std::shared_ptr<PhongShader> phong_shader_;
        std::shared_ptr<VertexObject> sphere_mesh_;
        std::shared_ptr<Material> sphere_material_;
        std::shared_ptr<VertexObject> spring_mesh_;
        std::shared_ptr<Material> spring_material_;
    };
}

#endif