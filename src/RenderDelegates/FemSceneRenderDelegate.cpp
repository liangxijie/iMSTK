#include "Simulators/FemSceneObject.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Mesh/VolumeMesh.h"

class smFemSceneRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
};

void smFemSceneRenderDelegate::draw() const
{
  smVolumeMesh* v_mesh = this->getSourceGeometryAs<smFemSceneObject>()->v_mesh;
  RenderDelegate::Ptr delegate = v_mesh->getRenderDelegate();
  if (delegate)
    delegate->draw();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_fem_scene_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,smFemSceneRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
