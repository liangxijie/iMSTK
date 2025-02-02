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

#include "imstkVTKViewer.h"
#include "imstkLogger.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKScreenCaptureUtility.h"
#include "imstkVTKTextStatusManager.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTextActor.h>
#include <vtkOpenGLRenderWindow.h>

#include <vtkCamera.h>

#ifdef WIN32
#include <vtkWin32HardwareWindow.h>
#include <vtkWin32RenderWindowInteractor.h>
#else
#ifdef iMSTK_USE_VTK_OSMESA
#include <vtkGenericRenderWindowInteractor.h>
#else
#include <vtkXRenderWindowInteractor.h>
#endif
#endif

namespace imstk
{
VTKViewer::VTKViewer(std::string name) : AbstractVTKViewer(name),
    m_lastFpsUpdate(std::chrono::high_resolution_clock::now()),
    m_textStatusManager(std::make_shared<VTKTextStatusManager>()),
    m_lastFps(60.0)
{
    // Create the interactor style
    m_vtkInteractorStyle = vtkSmartPointer<VTKInteractorStyle>::New();

    // Create the interactor
#ifdef WIN32
    auto iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
#else
#ifdef iMSTK_USE_VTK_OSMESA
    auto iren = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
#else
    vtkSmartPointer<vtkXRenderWindowInteractor> iren = vtkSmartPointer<vtkXRenderWindowInteractor>::New();
    iren->SetInteractorStyle(m_vtkInteractorStyle.get());
#endif
#endif
    iren->SetInteractorStyle(m_vtkInteractorStyle);

    // Create the RenderWindow
    m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_vtkRenderWindow->SetInteractor(iren);
    m_vtkRenderWindow->SetSize(m_config->m_renderWinWidth, m_config->m_renderWinHeight);

    // Screen capture
    m_screenCapturer = std::make_shared<VTKScreenCaptureUtility>(m_vtkRenderWindow);

    // Setup text status
    m_textStatusManager->setWindowSize(this);
}

void
VTKViewer::setActiveScene(std::shared_ptr<Scene> scene)
{
    // If already current scene
    if (scene == m_activeScene)
    {
        LOG(WARNING) << scene->getName() << " already is the viewer current scene.";
        return;
    }

    // If the current scene has a renderer, remove it
    if (m_activeScene != nullptr)
    {
        auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();
        if (m_vtkRenderWindow->HasRenderer(vtkRenderer))
        {
            m_vtkRenderWindow->RemoveRenderer(vtkRenderer);
        }
    }

    // Update current scene
    m_activeScene = scene;

    // Create renderer if it doesn't exist
    if (!m_rendererMap.count(m_activeScene))
    {
        m_rendererMap[m_activeScene] = std::make_shared<VTKRenderer>(m_activeScene, false);
    }

    // Cast to VTK renderer
    auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(vtkRenderer);

    // Move text actors from old to new renderer
    if (m_vtkInteractorStyle->GetCurrentRenderer() != NULL)
    {
        // Remove from old renderer
        for (int i = 0; i < static_cast<int>(VTKTextStatusManager::StatusType::NumStatusTypes); i++)
        {
            m_vtkInteractorStyle->GetCurrentRenderer()->RemoveActor2D(m_textStatusManager->getTextActor(i));
        }
    }
    m_vtkInteractorStyle->SetCurrentRenderer(vtkRenderer);
    // Add to new renderer
    for (int i = 0; i < static_cast<int>(VTKTextStatusManager::StatusType::NumStatusTypes); i++)
    {
        vtkRenderer->AddActor2D(m_textStatusManager->getTextActor(i));
    }

    // Set name to renderWindow
    m_vtkRenderWindow->SetWindowName(m_activeScene->getName().data());

    // Update the camera
    std::shared_ptr<VTKRenderer> ren = std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer());
    if (ren != nullptr)
    {
        ren->updateCamera();
    }
}

void
VTKViewer::setDebugAxesLength(double x, double y, double z)
{
    auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer());
    if (vtkRenderer != nullptr)
    {
        vtkRenderer->setAxesLength(x, y, z);
    }
}

void
VTKViewer::setInfoLevel(const int level)
{
    Viewer::setInfoLevel(level);

    // Level 0 show no info
    if (level == 0)
    {
        getTextStatusManager()->setStatusVisibility(VTKTextStatusManager::StatusType::FPS, false);
        getActiveScene()->setEnableTaskTiming(false);
        std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer())->setTimeTableVisibility(false);
    }
    // Level 1, show fps only
    else if (level == 1)
    {
        getTextStatusManager()->setStatusVisibility(VTKTextStatusManager::StatusType::FPS, true);
        getActiveScene()->setEnableTaskTiming(false);
        std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer())->setTimeTableVisibility(false);
    }
    // Level 2 show fps and timing graph
    else if (level == 2)
    {
        getTextStatusManager()->setStatusVisibility(VTKTextStatusManager::StatusType::FPS, true);
        getActiveScene()->setEnableTaskTiming(true);
        std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer())->setTimeTableVisibility(true);
    }
}

void
VTKViewer::setRenderingMode(const Renderer::Mode mode)
{
    if (!m_activeScene)
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Switch the renderer to the mode
    this->getActiveRenderer()->setMode(mode, false);

    updateModule();

    if (m_config->m_hideCurzor)
    {
        m_vtkRenderWindow->HideCursor();
    }

    if (m_config->m_hideBorder)
    {
        m_vtkRenderWindow->BordersOff();
    }

    if (m_config->m_fullScreen)
    {
        m_vtkRenderWindow->FullScreenOn();
    }
}

bool
VTKViewer::initModule()
{
    if (!AbstractVTKViewer::initModule())
    {
        return false;
    }

    m_vtkRenderWindow->SetWindowName(m_config->m_windowName.c_str());
    if (m_vtkRenderWindow->GetInteractor()->HasObserver(vtkCommand::StartEvent))
    {
        m_vtkRenderWindow->GetInteractor()->InvokeEvent(vtkCommand::StartEvent, nullptr);
    }

    m_vtkRenderWindow->GetInteractor()->Initialize();
    setUseVsync(m_useVsync);
    m_vtkRenderWindow->Render();

    return true;
}

std::shared_ptr<VTKScreenCaptureUtility>
VTKViewer::getScreenCaptureUtility() const
{
    return std::static_pointer_cast<VTKScreenCaptureUtility>(m_screenCapturer);
}

std::shared_ptr<KeyboardDeviceClient>
VTKViewer::getKeyboardDevice() const
{
    return VTKInteractorStyle::SafeDownCast(m_vtkInteractorStyle)->getKeyboardDeviceClient();
}

std::shared_ptr<MouseDeviceClient>
VTKViewer::getMouseDevice() const
{
    return VTKInteractorStyle::SafeDownCast(m_vtkInteractorStyle)->getMouseDeviceClient();
}

void
VTKViewer::updateModule()
{
    std::shared_ptr<VTKRenderer> ren = std::dynamic_pointer_cast<VTKRenderer>(getActiveRenderer());
    if (ren == nullptr)
    {
        return;
    }

    // Update Camera
    ren->updateCamera();

    // Call visual update on every scene object
    getActiveScene()->updateVisuals();
    // Update all the rendering delegates
    ren->updateRenderDelegates();

    // Automatically determine near and far planes (not used atm)
    //ren->getVtkRenderer()->ResetCameraClippingRange();

    // If fps status is on, measure it
    if (getTextStatusManager()->getStatusVisibility(VTKTextStatusManager::StatusType::FPS))
    {
        // Update framerate value display
        auto   now       = std::chrono::high_resolution_clock::now();
        double visualFPS = 1e6 / static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - m_pre).count());
        visualFPS = 0.1 * visualFPS + 0.9 * m_lastFps;
        m_lastFps = visualFPS;

        const int t = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFpsUpdate).count());
        if (t > 150) // wait 150ms before updating displayed value
        {
            const double physicalFPS = getActiveScene()->getFPS();
            m_textStatusManager->setFPS(visualFPS, physicalFPS);
            m_lastFpsUpdate = now;

            // Update the timing table
            getActiveScene()->lockComputeTimes();
            ren->setTimeTable(getActiveScene()->getTaskComputeTimes());
            getActiveScene()->unlockComputeTimes();
        }
        m_pre = now;
    }

    // Render
    m_vtkRenderWindow->Render();
}
}
