#include "SimulationApp.hpp"

#include "glm/gtx/string_cast.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/Material.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

#include "BunnyNode.hpp"

#include <fstream>

namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             float integration_step)
    : Application(app_name, window_size),
      integration_step_(integration_step) {
}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 5.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.2f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 2.0f, 4.f));
  root.AddChild(std::move(point_light_node));

  // Create Bunny Node
  auto bunny_node = make_unique<BunnyNode>(integration_step_);
  BunnyNode* bunny_pointer = bunny_node.get();
  root.AddChild(std::move(bunny_node));
  bunny_pointer->GetTransform().SetRotation(glm::quat(1.f, 0.f, 0.f, 0.f));

  // Create Explosion Center
  auto expl_center = std::shared_ptr<VertexObject>(PrimitiveFactory::CreateSphere(0.05f, 20, 20));
  auto shader_ = std::make_shared<PhongShader>();
  glm::vec3 ctrl_color(0.f,1.f,0.f);
  auto expl_material = std::make_shared<Material>(ctrl_color, ctrl_color, glm::vec3(0.4f), 20.0f);

  auto expl_node = make_unique<SceneNode>();
  expl_node->CreateComponent<ShadingComponent>(shader_);
  expl_node->CreateComponent<RenderingComponent>(expl_center);
  expl_node->CreateComponent<MaterialComponent>(expl_material);
  expl_node->GetTransform().SetPosition(glm::vec3(0.f,0.201f,0.f));
  root.AddChild(std::move(expl_node));
}
}  // namespace GLOO
