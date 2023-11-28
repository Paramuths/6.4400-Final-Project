#ifndef GLOO_MY_SHADER_H_
#define GLOO_MY_SHADER_H_

#include "ShaderProgram.hpp"

namespace GLOO {
class MyShader : public ShaderProgram {
 public:
  MyShader();
  void SetTargetNode(const SceneNode& node,
                     const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;
  void SetLightSource(const LightComponent& componentt) const override;


 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
