/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkVTKAssemblyRenderDelegate.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"
#include "imstkColorFunction.h"
#include "imstkAssembly.h"
#include "imstkLogger.h"

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkColorTransferFunction.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

namespace imstk
{
VTKAssemblyRenderDelegate::VTKAssemblyRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKRenderDelegate(visualModel)
{
	m_geometry = std::static_pointer_cast<Assembly>(visualModel->getGeometry());
	LOG(INFO) << "VTKAssemblyRenderDelegate: " << 00000;

    // Setup mapper
    {
        vtkNew<vtkAssembly> assembly;
			vtkNew<vtkPolyDataMapper> mapper;
			mapper->SetInputData(m_geometry->polyData[1]);
			vtkNew<vtkActor> actor;
			actor->SetMapper(mapper);
			assembly->AddPart(actor);
        //actor->SetUserTransform(m_transform);
        //m_mapper = mapper;
        m_actor  = actor;
    }

    update();
    updateRenderProperties();
}

void
VTKAssemblyRenderDelegate::updateRenderProperties()
{
    std::shared_ptr<RenderMaterial>    material      = m_visualModel->getRenderMaterial();
    vtkSmartPointer<vtkProperty>       actorProperty = vtkActor::SafeDownCast(m_actor)->GetProperty();

    // Display mode
    switch (material->getDisplayMode())
    {
    case RenderMaterial::DisplayMode::Wireframe:
        actorProperty->SetRepresentationToWireframe();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::Points:
        actorProperty->SetRepresentationToPoints();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::WireframeSurface:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(true);
        break;
    case RenderMaterial::DisplayMode::Surface:
    default:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(false);
        actorProperty->SetVertexVisibility(false);
        break;
    }

    m_actor->SetVisibility(m_visualModel->isVisible() ? 1 : 0);
}
}