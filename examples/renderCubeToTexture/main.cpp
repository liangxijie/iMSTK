// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "../common/wasdCameraController.h"
#include "../common/KeyPressSDKShutdown.h"

#include <memory>

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smCollision/smMeshCollisionModel.h"

int main()
{
    std::shared_ptr<smSDK> sdk;
    std::shared_ptr<smViewer> viewer;
    std::shared_ptr<smScene> scene1, scene2;
    std::shared_ptr<smLight> light1, light2;
    std::shared_ptr<smCamera> sceneCamera1, sceneCamera2;
    std::shared_ptr<smStaticSceneObject> cube, square;
    std::shared_ptr<smtk::Examples::Common::wasdCameraController> camCtl;
    std::shared_ptr<smtk::Examples::Common::KeyPressSDKShutdown> keyShutdown;

    //Create an instance of the SimMedTK framework/SDK
    sdk = smSDK::getInstance();

    //Create the scenes
    scene1 = sdk->createScene(); //internal scene containing cube
    scene2 = sdk->createScene(); //external scene containing square with scene1 mapped to it

    //Create a viewer to see the scene through
    viewer = std::make_shared<smViewer>();
    sdk->addViewer(viewer);

    //Create the camera controller
    camCtl = std::make_shared<smtk::Examples::Common::wasdCameraController>();
    keyShutdown = std::make_shared<smtk::Examples::Common::KeyPressSDKShutdown>();

    //Initialize the texture manager
    smTextureManager::init(sdk->getErrorLog());

    //Load in the texture for the cube model
    smTextureManager::loadTexture("textures/cube.png", "cubetex");

    std::shared_ptr<smMeshCollisionModel> cubeModel = std::make_shared<smMeshCollisionModel>();
    cubeModel->loadTriangleMesh("models/cube.obj", SM_FILETYPE_OBJ);
    cubeModel->getMesh()->assignTexture("cubetex");
    cubeModel->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    //setup scene1
    cube = std::make_shared<smStaticSceneObject>();
    cube->setModel(cubeModel);

    //Add the cube to the scene to be rendered
    scene1->addSceneObject(cube);

    //setup scene2
    //Create a color and depth texture for the FBO
    smTextureManager::createColorTexture("colorTex1", 64, 64);
    smTextureManager::createDepthTexture("depthTex1", 64, 64);

    std::shared_ptr<smMeshCollisionModel> squareModel = std::make_shared<smMeshCollisionModel>();
    squareModel->loadTriangleMesh("models/square.obj", SM_FILETYPE_OBJ);
    squareModel->getMesh()->assignTexture("colorTex1");
    squareModel->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    square = std::make_shared<smStaticSceneObject>();
    square->setModel(squareModel);

    //Setup an FBO for rendering in the viewer.
    //Add the FBO and textures to the viewer
    viewer->addFBO("fbo1",
                  smTextureManager::getTexture("colorTex1"),
                  smTextureManager::getTexture("depthTex1"),
                  64, 64);

    //Add the square to the scene
    scene2->addSceneObject(square);

    //Register the scene with the viewer, and setup render target
    viewer->registerScene(scene1, SMRENDERTARGET_FBO, "fbo1");
    viewer->registerScene(scene2, SMRENDERTARGET_SCREEN, "");

    //Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TO TEXTURE TEST");

    //Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    // Setup Scene lighting
    light1 = smLight::getDefaultLighting();
    assert(light1);
    scene1->addLight(light1);

    light2 = smLight::getDefaultLighting();
    assert(light2);
    light2->lightPos.setPosition(smVec3d(0.0, 0.0, 5.0));
    scene2->addLight(light2);

    // Camera setup
    sceneCamera1 = smCamera::getDefaultCamera();
    assert(sceneCamera1);
    sceneCamera1->setCameraPos(3, 3, 5);
    sceneCamera1->setCameraFocus(0, 0, -1);
    sceneCamera1->genProjMat();
    sceneCamera1->genViewMat();
    scene1->addCamera(sceneCamera1);
    camCtl->setCamera(sceneCamera1);

    sceneCamera2 = smCamera::getDefaultCamera();
    assert(sceneCamera2);
    sceneCamera2->setCameraPos(0, 0, 5);
    sceneCamera2->setCameraFocus(0, 0, -1);
    sceneCamera2->genProjMat();
    sceneCamera2->genViewMat();
    scene2->addCamera(sceneCamera2);

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(smtk::Event::EventType::Keyboard, camCtl);
    viewer->attachEvent(smtk::Event::EventType::Keyboard, keyShutdown);

    //run the framework
    sdk->run();

    //cleanup
    sdk->releaseScene(scene1);
    sdk->releaseScene(scene2);

    return 0;
}
