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

#include "gtest/gtest.h"

#include "imstkScene.h"
#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkSpotLight.h"
#include "imstkSceneObject.h"
#include "imstkTaskGraph.h"

using namespace imstk;

TEST(imstkSceneTest, empty_scene_emptiness_checks)
{
    Scene scene("test scene");

    // Do some basic checks for empty scene
    EXPECT_EQ(scene.getName(), "test scene");
    EXPECT_EQ(scene.getSceneObjects().size(), 0);
    EXPECT_EQ(scene.getControllers().size(), 0);
    EXPECT_EQ(scene.getLights().size(), 0);
    EXPECT_EQ(scene.getFPS(), 0.0);
    EXPECT_EQ(scene.getGlobalIBLProbe(), nullptr);

    EXPECT_EQ(scene.getCameras().size(), 2);
    EXPECT_EQ(scene.getCameraName(scene.getActiveCamera()), "default");
}

TEST(imstkSceneTest, empty_scene_default_cameras)
{
    Scene scene("test scene");

    EXPECT_EQ(scene.getCameras().size(), 2);
    EXPECT_EQ(scene.getCameraName(scene.getActiveCamera()), "default");
}

TEST(imstkSceneTest, empty_scene_initialize_advance)
{
    Scene scene("test scene");

    // Calling advance before initialize should do nothing
    EXPECT_NO_THROW({ scene.advance(0.1); });

    // Initialize the first time
    EXPECT_EQ(scene.initialize(), true);
    EXPECT_EQ(scene.initialize(), true); // Repeated call

    // Advance after initialization
    EXPECT_NO_THROW({ scene.advance(0.1); });
}

TEST(imstkSceneTest, empty_scene_bounding_box)
{
    Scene scene("test scene");

    // check bounding box
    Vec3d lowerLeft, upperRight;
    scene.computeBoundingBox(lowerLeft, upperRight, 2.0);
    EXPECT_EQ(lowerLeft, Vec3d(0.0, 0.0, 0.0));
    EXPECT_EQ(upperRight, Vec3d(0.0, 0.0, 0.0));
}

TEST(imstkSceneTest, empty_scene_build_task_graph)
{
    Scene scene("test scene");
    EXPECT_NO_THROW(scene.buildTaskGraph(); );
}

TEST(imstkSceneTest, add_camera)
{
    Scene m_scene("test scene");

    const auto defaultNumCam = m_scene.getCameras().size();

    m_scene.addCamera("default", std::make_shared<Camera>()); // camera by name 'default' already exists
    EXPECT_EQ(m_scene.getCameras().size(), defaultNumCam);
    m_scene.addCamera("debug", std::make_shared<Camera>());   // camera by name 'debug' already exists
    EXPECT_EQ(m_scene.getCameras().size(), defaultNumCam);

    auto cam0 = std::make_shared<Camera>();
    m_scene.addCamera("cam0", cam0);
    EXPECT_EQ(m_scene.getCamera("cam0"), cam0);
    EXPECT_EQ(m_scene.getCameras().size(), defaultNumCam + 1);
}

TEST(imstkSceneTest, remove_camera)
{
    Scene m_scene("test scene");

    const auto defaultNumCam = m_scene.getCameras().size();

    m_scene.addCamera("cam0", std::make_shared<Camera>());

    // test remove camera
    m_scene.removeCamera("default"); // default cannot be removed
    EXPECT_EQ(m_scene.getCameraName(m_scene.getActiveCamera()), "default");
    EXPECT_EQ(m_scene.getCameras().size(), defaultNumCam + 1);

    m_scene.removeCamera("cam0");
    EXPECT_EQ(m_scene.getCamera("cam0"), nullptr);
    EXPECT_EQ(m_scene.getCameras().size(), defaultNumCam);
}

TEST(imstkSceneTest, get_set_active_camera)
{
    Scene m_scene("test scene");

    EXPECT_EQ(m_scene.getCameraName(m_scene.getActiveCamera()), "default");
    auto defaultCam = m_scene.getActiveCamera();

    auto cam0 = std::make_shared<Camera>();
    m_scene.addCamera("cam0", cam0);
    m_scene.setActiveCamera("cam0");
    EXPECT_EQ(m_scene.getActiveCamera(), cam0);

    m_scene.setActiveCamera("default"); // switch back
    EXPECT_EQ(m_scene.getActiveCamera(), defaultCam);
}

TEST(imstkSceneTest, add_remove_lights)
{
    Scene m_scene("sample scene");

    auto light0 = std::make_shared<SpotLight>();
    m_scene.addLight("light0", light0);
    EXPECT_EQ(m_scene.getLight("light0"), light0);
    EXPECT_EQ(m_scene.getLights().size(), 1);

    // Add another light with the same name
    auto light0_2 = std::make_shared<DirectionalLight>();
    m_scene.addLight("light0", light0_2);
    EXPECT_NE(m_scene.getLight("light0"), light0_2);

    // test remove light
    m_scene.removeLight("light0");
    EXPECT_EQ(m_scene.getLight("light0"), nullptr);
    EXPECT_EQ(m_scene.getLights().size(), 0);
}

TEST(imstkSceneTest, add_remove_scene_objects)
{
    Scene m_scene("sample scene");

    // Add a scene object and do some checks
    auto so = std::make_shared<SceneObject>("sceneObject0");
    m_scene.addSceneObject(so);
    EXPECT_EQ(m_scene.getSceneObject("sceneObject0"), so);
    EXPECT_EQ(m_scene.getSceneObjects().size(), 1);

    m_scene.addSceneObject(so);
    EXPECT_EQ(m_scene.getSceneObjects().size(), 1);

    // remove the scene object and do some checks
    m_scene.removeSceneObject(so);
    EXPECT_EQ(m_scene.getSceneObject("sceneObject0"), nullptr);
    EXPECT_EQ(m_scene.getSceneObjects().size(), 0);
}