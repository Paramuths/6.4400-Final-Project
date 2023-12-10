#ifndef SPHERE_NODE_H_
#define SPHERE_NODE_H_

#include "gloo/SceneNode.hpp"
#include "IntegratorBase.hpp"
#include "ConstantSpeedSystem.hpp"
#include "ParticleState.hpp"
#include "gloo/shaders/MyShader.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/Material.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
    class SphereNode : public SceneNode {
        public:
        SphereNode(float integration_step);
        void Update(double delta_time) override;

        private:
        void Init();
        void InitParticle();
        void InitSphere();
        void InitSystem();
        void Advance(float start_time);
        void SetPositions();

        std::unique_ptr<IntegratorBase<ParticleSystemBase, ParticleState>> integrator_;
        ParticleState particle_state_;
        ConstantSpeedSystem particle_system_;

        // step
        float integration_step_;
        float carrier_time_step_ = 0.f;

        SceneNode* sphere_pointer_;
        std::shared_ptr<PhongShader> phong_shader_;
        std::shared_ptr<Material> sphere_material_;
        std::shared_ptr<VertexObject> sphere_mesh_;

        bool start_;
    };
}

#endif