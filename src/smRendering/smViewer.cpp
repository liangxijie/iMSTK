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

#include "smCore/smConfig.h"
#include "smCore/smSDK.h"
#include "smCore/smKeySFMLInterface.h"
#include "smUtilities/smIOStream.h"
#include "smRendering/smGLRenderer.h"
#include "smRendering/smViewer.h"
#include "smShader/smShader.h"
#include "smShader/smMetalShader.h"
#include "smShader/smSceneTextureShader.h"
#include "smUtilities/smDataStructures.h"
#include "smRendering/smVBO.h"
#include "smRendering/smVAO.h"
#include "smExternal/tree.hh"

#ifdef SIMMEDTK_OPERATINGSYSTEM_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glxew.h>
#include <GL/glxext.h>
#include <GL/glx.h>
#endif

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
#endif

void smViewer::setVSync(bool sync)
{
    this->sfmlWindow->setVerticalSyncEnabled(sync);
}

smRenderOperation::smRenderOperation()
{
    fbo = NULL;
    scene = NULL;
    fboName = "";
}

smViewer::smViewer()
{
    type = SIMMEDTK_SMVIEWER;
    viewerRenderDetail = SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;
    shadowMatrix.setIdentity();
    shadowMatrix(0, 1) = 0;
    shadowMatrix(1, 1) = 0.0;
    shadowMatrix(2, 1) = 0;
    shadowMatrix(3, 1) = 0;

    defaultAmbientColor.setValue(0.1, 0.1, 0.1, 1.0);
    defaultDiffuseColor.setValue(0.8, 0.8, 0.8, 1.0);
    defaultSpecularColor.setValue(0.9, 0.9, 0.9, 1.0);

    this->log = NULL;
    consoleDisplay = false;
    lights = new smIndiceArray<smLight*>(SIMMEDTK_VIEWER_MAXLIGHTS);
    windowOutput = new smOpenGLWindowStream();
    lightDrawScale = 50;
    enableCameraMotion = false;

    unlimitedFPSEnabled = false;
    unlimitedFPSVariableChanged = 1;
    screenResolutionWidth = 1680;
    screenResolutionHeight = 1050;

    offsetAngle_Direction = 0;
    offsetAngle_UpDirection = 0;
    offsetAngle_rightDirection = 0;

    cameraRadius = 1.0;
    prevState_collided = false;
    checkCameraCollision = false;
}

///affects the framebuffer size and depth buffer size
void smViewer::setScreenResolution(smInt p_width, smInt p_height)
{
    this->screenResolutionHeight = p_height;
    this->screenResolutionWidth = p_width;
}

smInt smViewer::addLight(smLight *p_light)
{
    smInt index = lights->add(p_light);
    lights->getByRef(index)->renderUsage = GL_LIGHT0 + index;
    lights->getByRef(index)->activate(true);
    return index;
}

smBool smViewer::setLight(smInt p_lightId, smLight *p_light)
{
    smInt index = lights->replace(p_lightId, p_light);

    if (index > 0)
    {
        lights->getByRef(p_lightId)->renderUsage = GL_LIGHT0 + p_lightId;
        return true;
    }
    return false;
}

void smViewer::refreshLights()
{
    smIndiceArrayIter<smLight*> iter(lights);

    for (smInt i = iter.begin(); i < iter.end(); iter++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, iter[i]->spotCutOffAngle);
        glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
    }
}

smBool smViewer::updateLight(smInt p_lightId, smLight *p_light)
{

    p_light->updateDirection();
    return lights->replace(p_lightId, p_light);
}

void smViewer::setLightPos(smInt p_lightId, smLightPos p_pos)
{

    smLight *temp;
    temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->updateDirection();
}

void smViewer::setLightPos(smInt p_lightId,
                           smLightPos p_pos,
                           smVec3f p_direction)
{

    smLight *temp;
    temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->direction = p_direction;
    temp->updateDirection();
}

void smViewer::setUnlimitedFPS(smBool p_enableFPS)
{

    unlimitedFPSEnabled = p_enableFPS;
    unlimitedFPSVariableChanged++;
}

void smViewer::initGLCaps()
{
    //use multiple fragment samples in computing the final color of a pixel
    glEnable(GL_MULTISAMPLE);
    //do depth comparisons and update the depth buffer
    glEnable(GL_DEPTH_TEST);
    //cull polygons based on their winding in window coordinates
    glEnable(GL_CULL_FACE);
    //DEPRECIATED AS OF v3.3 have one or more material parameters
    // track the current color
    glDisable(GL_COLOR_MATERIAL);
    //DEPRECIATED AS OF v3.3 If enabled and no vertex shader is active,
    // use the current lighting parameters to compute the vertex color or index
    glEnable(GL_LIGHTING);
    //DEPRECIATED AS OF v3.3 If enabled and no vertex shader is active,
    // normal vectors are normalized to unit length after transformation and
    // before lighting
    glEnable(GL_NORMALIZE);

    //Fill the face of the polygon for all front and back facing polygons
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    glFrontFace(GL_CCW);
    //DEPRECIATED AS OF v3.3 Smooth shading
    glShadeModel(GL_SMOOTH);
    //DEPRECIATED AS OF v3.3 Specifies the specular component of a material
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 50);
}

void smViewer::initLights()
{
    smIndiceArrayIter<smLight*> iter(lights);
    // Create light components
    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, (smGLFloat)iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, (smGLFloat)iter[i]->spotCutOffAngle);
        glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
    }
}

void smViewer::initObjects(smDrawParam p_param)
{
    for (smInt i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->getType() != SIMMEDTK_SMSHADER)
        {
            objectList[i]->initDraw(p_param);
        }
        else
        {
            continue;
        }
    }
}

void smViewer::initResources(smDrawParam p_param)
{
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_SOFTSHADOWS)
    {
        fbo = new smFrameBuffer();
        fbo->setDim(2048, 2048);
        smTextureManager::createDepthTexture("depth", 2048, 2048);
        backfbo = new smFrameBuffer();
        backfbo->setDim(1024, 1024);
        smTextureManager::createColorTexture("backmap", 1024, 1024);
        smTextureManager::createDepthTexture("backmapdepth", 1024, 1024);
        backfbo->renderDepthBuff = true;
        backfbo->renderColorBuff = true;
    }

    smTextureManager::initGLTextures();
    smShader::initGLShaders(p_param);
    smVAO::initVAOs(p_param);

    initFboListItems();

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_SOFTSHADOWS)
    {
        fbo->setDim(2048, 2048);
        fbo->attachDepthTexture(smTextureManager::getTexture("depth"));

        std::cout << "Checking the status of framebuffer for shadow" << "\n";
        fbo->checkStatus();

        backfbo->setDim(1024, 1024);
        backfbo->attachColorTexture(smTextureManager::getTexture("backmap"), 0);
        backfbo->attachDepthTexture(smTextureManager::getTexture("backmapdepth"));
        std::cout << "Checking the status of framebuffer for dualparaboloid backmap" << "\n";
        backfbo->checkStatus();

        smTextureManager::disableTexture("depth");
        smTextureManager::disableTexture("backmap");
    }
}

void smViewer::initScenes(smDrawParam p_param)
{
    smClassType objectType;
    smStaticSceneObject *staticSceneObject;
    smSceneObject *sceneObject;
    smScene *scene;
    smScene::smSceneIterator sceneIter;

    //traverse all the scene and the objects in the scene
    for (smInt i = 0; i < sceneList.size(); i++)
    {
        scene = sceneList[i];
        scene->registerForScene(this);
        sceneIter.setScene(scene, this);

        for (smInt j = sceneIter.start(); j < sceneIter.end(); j++)
        {
            //sceneObject=scene->sceneObjects[j];
            sceneObject = sceneIter[j];

            //initialize the custom Render if there is any
            if (sceneObject->customRender != NULL && sceneObject->getType() != SIMMEDTK_SMSHADER)
            {
                sceneObject->customRender->initDraw(p_param);
            }

            sceneObject->initDraw(p_param);

            if (sceneObject->renderDetail.renderType & SIMMEDTK_RENDER_VBO && viewerRenderDetail & SIMMEDTK_VIEWERRENDER_VBO_ENABLED)
            {
                objectType = sceneObject->getType();

                switch (objectType)
                {
                case SIMMEDTK_SMSTATICSCENEOBJECT:
                    staticSceneObject = (smStaticSceneObject*)sceneObject;
                    break;
                }
            }//scene object is added in the vbo object.
        }//object traverse
    }//scene traverse
}

void smViewer::initCamera()
{
    //Generate the Projection and View Matricies
    camera.genProjMat();
    camera.genViewMat();
}

void smViewer::initGLContext()
{

    // Init OpenGL context
    sfmlContext = std::unique_ptr<sf::Context>(new sf::Context);
    sfmlWindow = std::unique_ptr<sf::Window>(new sf::Window);
    // Init the rest of window system
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
    {
        this->sfmlWindow->create(sf::VideoMode(this->width(), this->height()),
                            windowTitle, sf::Style::Fullscreen);
    }
    else
    {
        this->sfmlWindow->create(sf::VideoMode(this->width(), this->height()),
                            windowTitle, (sf::Style::Titlebar | sf::Style::Close));
    }

    // Init GLEW
    GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong.
         * Most likely an OpenGL context is not created yet */
        std::cout << "Error:" << glewGetErrorString(err) << "\n";
        assert(false);
    }
}

///initialization of the viewer module
void smViewer::init()
{
    static smDrawParam param;

    if (isInitialized)
    {
        return;
    }

    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;

    this->initGLContext();
    this->initGLCaps();
    this->initLights();
    this->initObjects(param);
    this->initResources(param);
    this->initScenes(param);

    isInitialized = true;
}

void smViewer::destroyGLContext()
{
    //nothing to do
}

///draw the surface mesh triangles based on the rendering type
///problem is here
//void smViewer::drawSurfaceMeshTriangles(smSurfaceMesh *p_surfaceMesh,smRenderDetail *renderDetail)
void smViewer::drawSurfaceMeshTriangles(smMesh *p_surfaceMesh, smRenderDetail *renderDetail)
{

    static smVec3f origin(0, 0, 0);
    static smVec3f xAxis(1, 0, 0);
    static smVec3f yAxis(0, 1, 0);
    static smVec3f zAxis(0, 0, 1);

    if (renderDetail->renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glPointSize(renderDetail->pointSize);
    glLineWidth(renderDetail->lineSize);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (smGLReal*)renderDetail->colorDiffuse.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (smGLReal*)renderDetail->colorSpecular.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (smGLReal*)renderDetail->colorAmbient.toGLColor());
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_surfaceMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::activateTexture(p_surfaceMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    glVertexPointer(3, smGLRealType, 0, p_surfaceMesh->vertices.data());

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glTexCoordPointer(2, smGLRealType, 0, p_surfaceMesh->texCoord);
        }
    }

    glNormalPointer(smGLRealType, 0, p_surfaceMesh->vertNormals);

    if (renderDetail->renderType & SIMMEDTK_RENDER_FACES)
    {
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
    }

    if ((renderDetail->renderType & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_WIREFRAME || this->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_WIREFRAMEALL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(3.0, 2.0);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_LOCALAXIS)
    {
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_SHADOWS)
    {
        glMatrixMode(GL_MATRIX_MODE);
        glPushMatrix();
        glDisable(GL_LIGHTING);
        glMultMatrixf(shadowMatrix.data());
        glColor4fv((smGLFloat*)&renderDetail->shadowColor);
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_HIGHLIGHTVERTICES)
    {
        glDisable(GL_LIGHTING);
        glColor3fv((smGLReal*)&renderDetail->highLightColor);
        glDrawArrays(GL_POINTS, 0, p_surfaceMesh->nbrVertices);
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_surfaceMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::disableTexture(p_surfaceMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

///vertex buffer implementation. It is not implemented yet. It will improve the performance drastically
void smViewer::drawSurfaceMeshTrianglesVBO(smSurfaceMesh *p_surfaceMesh, smRenderDetail *renderDetail, smInt p_objectId, smVBOType p_VBOType)
{

    static smFloat shadowMatrixGL[16];
    static smVec3f origin(0, 0, 0);
    static smVec3f xAxis(1, 0, 0);
    static smVec3f yAxis(0, 1, 0);
    static smVec3f zAxis(0, 0, 1);

    if (renderDetail->renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glPointSize(renderDetail->pointSize);
    glLineWidth(renderDetail->lineSize);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (smGLReal*)&renderDetail->colorDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (smGLReal*)&renderDetail->colorSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (smGLReal*)&renderDetail->colorAmbient);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_FACES)
    {
        //placeholder
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

void smViewer::drawSMDeformableObject(smPBDSurfaceSceneObject *p_smPhsyObject)
{

    if (p_smPhsyObject->renderDetail.renderType & SIMMEDTK_RENDER_VBO)
    {
        //placeholder
    }
    else
    {
        drawSurfaceMeshTriangles(p_smPhsyObject->mesh, &p_smPhsyObject->renderDetail);
    }

    drawNormals(p_smPhsyObject->mesh);
}

void smViewer::drawNormals(smMesh *p_mesh)
{

    glDisable(GL_LIGHTING);
    glColor3fv((smGLFloat*)&smColor::colorBlue);
    smVec3f baryCenter;
    smGLFloat* tmp = NULL;
    glBegin(GL_LINES);

    for (smInt i = 0; i < p_mesh->nbrVertices; i++)
    {
        glVertex3fv((smGLFloat*) & (p_mesh->vertices[i]));
        smVec3f vector = p_mesh->vertices[i] + p_mesh->vertNormals[i] * 5;
        glVertex3fv(vector.data());
    }

    for (smInt i = 0; i < p_mesh->nbrTriangles; i++)
    {
        baryCenter = p_mesh->vertices[p_mesh->triangles[i].vert[0]] + p_mesh->vertices[p_mesh->triangles[i].vert[1]] + p_mesh->vertices[p_mesh->triangles[i].vert[2]] ;
        baryCenter = baryCenter / 3.0;
        glVertex3fv((smGLFloat*) & (baryCenter));
        smVec3f vector = baryCenter + p_mesh->triNormals[i] * 5;
        glVertex3fv(vector.data());
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

///draw the static objects
void smViewer::drawSMStaticObject(smStaticSceneObject *p_smPhsyObject)
{

    if (p_smPhsyObject->renderDetail.renderType & SIMMEDTK_RENDER_VBO)
    {
        //
    }
    else
    {
        drawSurfaceMeshTriangles(p_smPhsyObject->mesh, &p_smPhsyObject->renderDetail);
    }
}

void smViewer::enableLights()
{

    static smIndiceArrayIter<smLight*> iter(lights);
    smFloat dir[4];
    static smLightPos defaultPos(0, 0, 0);

    glEnable(GL_LIGHTING);

    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        if (iter[i]->isEnabled())
        {
            glEnable(iter[i]->renderUsage);
        }
        else
        {
            glDisable(iter[i]->renderUsage);
        }

        glLightf(iter[i]->renderUsage, GL_CONSTANT_ATTENUATION, iter[i]->attn_constant);
        glLightf(iter[i]->renderUsage, GL_LINEAR_ATTENUATION, iter[i]->attn_linear);
        glLightf(iter[i]->renderUsage, GL_QUADRATIC_ATTENUATION, iter[i]->attn_quadratic);

        if (iter[i]->lightLocationType == SIMMEDTK_LIGHTPOS_EYE)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
            glPopMatrix();
        }
        else
        {
            glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        }

        if (iter[i]->lightType == SIMMEDTK_LIGHT_SPOTLIGHT)
        {
            glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
        }

        glGetLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, dir);
    }
}

void setTextureMatrix()
{

    static double modelView[16];
    static double projection[16];

    // This is matrix transform every coordinate x,y,z
    // x = x* 0.5 + 0.5
    // y = y* 0.5 + 0.5
    // z = z* 0.5 + 0.5
    // Moving from unit cube [-1,1] to [0,1]
    const GLdouble bias[16] =
    {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    };

    // Grab modelview and transformation matrices
    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glActiveTextureARB(GL_TEXTURE7);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glLoadMatrixd(bias);

    // concatating all matrice into one.
    glMultMatrixd(projection);
    glMultMatrixd(modelView);

    // Go back to normal matrix mode
    glMatrixMode(GL_MODELVIEW);
}

void smViewer::renderTextureOnView()
{

    glPushAttrib(GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_LIGHTING_BIT);
    glUseProgramObjectARB(0);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glActiveTextureARB(GL_TEXTURE0);
    smTextureManager::activateTexture("depth");
    glEnable(GL_TEXTURE_2D);
    glTranslated(0, 0, -1);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2d(1, 0);
    glVertex3f(1, 0, 0);
    glTexCoord2d(1, 1);
    glVertex3f(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3f(0, 1.0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void smViewer::addFBO(const smString &p_fboName,
                      smTexture *p_colorTex,
                      smTexture *p_depthTex,
                      smUInt p_width, smUInt p_height)
{
    smFboListItem item;

    item.fboName = p_fboName;
    item.width = p_width;
    item.height = p_height;
    if (p_colorTex)
    {
        item.colorTex = p_colorTex;
    }
    if (p_depthTex)
    {
        item.depthTex = p_depthTex;
    }

    this->fboListItems.push_back(item);
}

void smViewer::initFboListItems()
{
    for (int i = 0; i < this->fboListItems.size(); i++)
    {
        smFboListItem *item = &fboListItems[i];
        item->fbo = new smFrameBuffer();
        item->fbo->setDim(item->width, item->height);
        if (item->colorTex)
        {
            item->fbo->attachColorTexture(item->colorTex, 0);
        }
        if (item->depthTex)
        {
            item->fbo->attachDepthTexture(item->depthTex);
        }
        for (int j = 0; j < renderOperations.size(); j++)
        {
            if (renderOperations[j].fboName == item->fboName)
            {
                renderOperations[j].fbo = item->fbo;
            }
        }
    }
}

void smViewer::destroyFboListItems()
{
    for (int i = 0; i < this->fboListItems.size(); i++)
    {
        if (fboListItems[i].fbo)
        {
            delete (fboListItems[i].fbo);
            fboListItems[i].fbo = NULL;
        }
    }
}

void smViewer::renderSceneList(smDrawParam p_param)
{
    smScene::smSceneIterator sceneIter;

    //this routine is for rendering. if you implement different objects add rendering accordingly. Viewer knows to draw
    //only current objects and their derived classes
    for (smInt sceneIndex = 0; sceneIndex < sceneList.size(); sceneIndex++)
    {
        smGLRenderer::renderScene(sceneList[sceneIndex], p_param);
    }
}

void smViewer::processViewerOptions()
{
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FADEBACKGROUND)
    {
        smGLUtils::fadeBackgroundDraw();
    }
}

void smViewer::processRenderOperation(const smRenderOperation &p_rop, smDrawParam p_param)
{
    switch (p_rop.target)
    {
    case SMRENDERTARGET_SCREEN:
        renderToScreen(p_rop, p_param);
        break;
    case SMRENDERTARGET_FBO:
        renderToFBO(p_rop, p_param);
        break;
    default:
        assert(0);
    }
}

void smViewer::renderToFBO(const smRenderOperation &p_rop, smDrawParam p_param)
{
    assert(p_rop.fbo);
    //Enable FBO for rendering
    p_rop.fbo->enable();
    //Setup Viewport & Clear buffers
    glViewport(0, 0, p_rop.fbo->getWidth(), p_rop.fbo->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //Enable lights
    enableLights();
    processViewerOptions();
    //Render Scene
     smGLRenderer::renderScene(p_rop.scene, p_param);
    //Disable FBO
    p_rop.fbo->disable();
}

void smViewer::renderToScreen(const smRenderOperation &p_rop, smDrawParam p_param)
{
    //Setup Viewport & Clear buffers
    glViewport(0, 0, this->width(), this->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //Enable lights
    enableLights();
    processViewerOptions();
    //Render Scene
    smGLRenderer::renderScene(p_rop.scene, p_param);
}

void smViewer::registerScene(smScene *p_scene,
                             smRenderTargetType p_target,
                             const smString &p_fboName)
{
    smRenderOperation rop;

    //sanity checks
    assert(p_scene);
    if (p_target == SMRENDERTARGET_FBO)
    {
        assert(p_fboName != "");
    }

    rop.target = p_target;
    rop.scene = p_scene;

    rop.fboName = p_fboName;

    p_scene->registerForScene(this);
    renderOperations.push_back(rop);
}

void smViewer::drawWithShadows(smDrawParam &p_param)
{
    smLight *light = NULL;

    for (smInt i = 0; i < lights->size(); i++)
    {
        light = lights->getByRef(i);

        if (light->castShadow)
        {
            break;
        }
    }

    renderStage = SMRENDERSTAGE_SHADOWPASS;

    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(light->shadorAngle, light->shadowRatio, light->shadowNearView, light->shadowFarView);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    light->upVector[0] = camera.up.x;
    light->upVector[1] = camera.up.y;
    light->upVector[2] = camera.up.z;
    light->direction[0] = camera.fp.x;
    light->direction[1] = camera.fp.y;
    light->direction[2] = camera.fp.z;
    light->updateDirection();
    gluLookAt(light->lightPos.pos[0], light->lightPos.pos[1], light->lightPos.pos[2],
              camera.fp.x, camera.fp.y, camera.fp.z,
              camera.up.x, camera.up.y, camera.up.z);
    gluLookAt(camera.pos.x, camera.pos.y, camera.pos.z,
              0.0, 0.0, 0.0,
              camera.up.x, camera.up.y, camera.up.z);

    fbo->enable();
    smTextureManager::activateTexture("depth");
    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT); // for GL_DRAW_BUFFER and GL_READ_BUFFER
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glClearColor(0, 0, 0, 1);
    glDepthRange(0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glViewport(0, 0, fbo->getWidth(), fbo->getHeight());
    renderSceneList(p_param);
    glPopAttrib();
    fbo->disable();
    setTextureMatrix();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}


inline void smViewer::setToDefaults()
{

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (smGLReal*)defaultDiffuseColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (smGLReal*)defaultSpecularColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (smGLReal*)defaultAmbientColor.toGLColor());
    glColor4fv(defaultDiffuseColor.toGLColor());
}

inline void smViewer::adjustFPS()
{

    static smInt _unlimitedFPSVariableChanged = 0;
    smInt unlimitedFPSVariableChangedCurrent;
    unlimitedFPSVariableChangedCurrent = unlimitedFPSVariableChanged;

    if (_unlimitedFPSVariableChanged < unlimitedFPSVariableChangedCurrent)
    {
        _unlimitedFPSVariableChanged = unlimitedFPSVariableChangedCurrent;

        if (unlimitedFPSEnabled)
        {
            setVSync(false);
        }
        else
        {
            setVSync(true);
        }
    }
}

smBool  smViewer::checkCameraCollisionWithScene()
{

    static bool collided = false;
    static bool prev_collided = false;
    static smVec3f  lastCollidedHatpicPos(deviceCameraPos[0], deviceCameraPos[1], deviceCameraPos[2]);
    static smVec3f  proxy_hapticPos(deviceCameraPos[0], deviceCameraPos[1], deviceCameraPos[2]);
    static float radiusEffective = 0;
    static smVec3f  last_collisionNormal;
    static smVec3f prevPosition(deviceCameraPos[0], deviceCameraPos[1], deviceCameraPos[2]);
    static smFloat radiusMotion = 0;

    smVec3f collisionNormal(0, 0, 0);
    collided = false;
    radiusEffective = 0;
    smVec3f hPos(deviceCameraPos[0], deviceCameraPos[1], deviceCameraPos[2]);
    radiusMotion = (hPos - prevPosition).norm();
    prevPosition = hPos;

    for (int i = 0; i < collisionMeshes.size(); i++)
    {
        float distance;
        smMesh*mesh = collisionMeshes[i];
        int nbrVert = mesh->nbrVertices;

        for (int j = 0; j < nbrVert; j++)
        {
            distance = (mesh->vertices[j]-hPos).norm();

            if (prev_collided)
            {
                smVec3f  posVector = hPos - lastCollidedHatpicPos;
                float distance = posVector.norm();
                posVector.normalize();

                if (posVector.dot(last_collisionNormal) < 0)
                {
                    if ((distance - cameraRadius) > 0)
                    {
                        radiusEffective = distance - cameraRadius;
                    }

                    if (radiusEffective < 0)
                    {
                        radiusEffective = 0;
                    }

                    collided = true;
                    break;
                }
            }
            else
            {
                radiusEffective = 0;
            }

            if (distance < cameraRadius + radiusEffective + radiusMotion)
            {
                collisionNormal = collisionNormal + mesh->vertNormals[j];

                if (!prev_collided)
                {
                    lastCollidedHatpicPos = hPos;
                    last_collisionNormal = collisionNormal;
                }

                collided = true;
                prev_collided = true;
            }
        }
    }

    if (collided)
    {
        proxy_hapticPos = lastCollidedHatpicPos;
        last_collisionNormal.normalize();
    }
    else
    {
        proxy_hapticPos = hPos;
        prev_collided = false;
    }

    return collided;
}

void smViewer::addCollisionCheckMeshes(smMesh *p_mesh)
{

    collisionMeshes.push_back(p_mesh);
}


///main drawing routine for Rendering of all objects in the scene
void smViewer::draw()
{

    static smDrawParam param;

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;

    beginModule();

    adjustFPS();

    param.projMatrix = camera.getProjMatRef();
    param.viewMatrix = camera.getViewMatRef();

    for (smInt i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->drawOrder == SIMMEDTK_DRAW_BEFOREOBJECTS);

        objectList[i]->draw(param);
    }

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_SOFTSHADOWS)
    {
        drawWithShadows(param);
    }
    else
    {
        for (int i = 0; i < renderOperations.size(); i++)
        {
            processRenderOperation(renderOperations[i], param);
        }
    }

    for (smInt i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->drawOrder == SIMMEDTK_DRAW_AFTEROBJECTS);

        objectList[i]->draw(param);
    }

    endModule();
}

///called by the module before each frame starts
void smViewer::beginFrame()
{
    if (terminateExecution == true)
    {
        terminationCompleted = true;
    }

    this->sfmlWindow->setActive(true); //activates opengl context
}

///called by the module after each frame ends
void smViewer::endFrame()
{
    this->sfmlWindow->display(); //swaps buffers
}

void smViewer::processSFMLEvents(const sf::Event& p_event)
{
    smEvent *event = nullptr;
    smSDK *sdk = smSDK::getInstance();
    assert(sdk);
    smViewer *viewer = sdk->getViewerInstance();
    assert(viewer);

    event = new smEvent();
    assert(event);
    event->senderId = this->getModuleId();
    event->senderType = SIMMEDTK_SENDERTYPE_MODULE;
    event->data = nullptr;

    switch(p_event.type)
    {
    case sf::Event::Closed:
        event->eventType = SIMMEDTK_EVENTTYPE_TERMINATE;
        break;
    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased:
        event->eventType = SIMMEDTK_EVENTTYPE_KEYBOARD;
        if (sf::Event::KeyPressed == p_event.type)
        {
            event->data =
                new smKeyboardEventData(SFMLKeyToSmKey(p_event.key.code), true,
                                        p_event.key.shift, p_event.key.control,
                                        p_event.key.alt, p_event.key.system);
        }
        else if (sf::Event::KeyReleased == p_event.type)
        {
            event->data =
                new smKeyboardEventData(SFMLKeyToSmKey(p_event.key.code), false,
                                        p_event.key.shift, p_event.key.control,
                                        p_event.key.alt, p_event.key.system);
        }
        else
        {
            delete event;
            return;
        }
        assert(event->data);
        break;
    case sf::Event::MouseButtonPressed:
    case sf::Event::MouseButtonReleased:
    {
        smMouseButton button;
        if (sf::Mouse::Left == p_event.mouseButton.button)
            button = smMouseButton::Left;
        else if (sf::Mouse::Right == p_event.mouseButton.button)
            button = smMouseButton::Right;
        else if (sf::Mouse::Middle == p_event.mouseButton.button)
            button = smMouseButton::Middle;
        else
            button = smMouseButton::Unknown;

        if (sf::Event::MouseButtonPressed == p_event.type)
        {
            event->eventType = SIMMEDTK_EVENTTYPE_MOUSE_BUTTON;
            event->data =
                new smMouseButtonEventData(button, true, p_event.mouseButton.x,
                                           p_event.mouseButton.y);
        }
        else if (sf::Event::MouseButtonReleased == p_event.type)
        {
            event->eventType = SIMMEDTK_EVENTTYPE_MOUSE_BUTTON;
            event->data =
                new smMouseButtonEventData(button, false, p_event.mouseButton.x,
                                           p_event.mouseButton.y);
        }
        assert(event->data);
        break;
    }
    case sf::Event::MouseMoved:
        event->eventType = SIMMEDTK_EVENTTYPE_MOUSE_MOVE;
        event->data =
            new smMouseMoveEventData(p_event.mouseMove.x, p_event.mouseMove.y);
        assert(event->data);
        break;
    default:
        delete event;
        return;
    }

    sdk->getEventDispatcher()->sendEventAndDelete(event);
}

void smViewer::addObject(smCoreClass *object)
{

    smSDK::addRef(object);
    objectList.push_back(object);
}

void smViewer::handleEvent(smEvent *p_event)
{
    smLight *light;
    smVec3d lightDir;
    smVec3d lightUp;
    smVec3d transverseDir;

    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_CAMERA_UPDATE:
    {
        smCameraEventData *cameraData =
            (smCameraEventData *)p_event->data;
        deviceCameraPos = cameraData->pos;
        deviceCameraDir = cameraData->direction;
        deviceCameraUpDir = cameraData->upDirection;
        break;
    }
    case SIMMEDTK_EVENTTYPE_LIGHTPOS_UPDATE:
    {
        smLightMotionEventData *lightPosData =
            (smLightMotionEventData*)p_event->data;

        if (lights->size() < lightPosData->lightIndex)
        {
            light = lights->getByRef(lightPosData->lightIndex);
            light->lightPos.pos = lightPosData->pos;
            light->direction = lightPosData->direction;
        }
        break;
    }
    default:
        break;
    }
}

void smViewer::addText(smString p_tag)
{

    windowOutput->addText(p_tag, smString(""));
}

void smViewer::updateText(smString p_tag, smString p_string)
{

    windowOutput->updateText(p_tag, p_string);
}
void smViewer::updateText(smInt p_handle, smString p_string)
{

    windowOutput->updateText(p_handle, p_string);
}

void smViewer::setWindowTitle(const smString &str)
{
    windowTitle = str;
}

void smViewer::exec()
{
    // Init the viewer
    this->init();

    while (!terminateExecution)
    {
        sf::Event event;
        this->draw();
        while (this->sfmlWindow->pollEvent(event))
        {
            this->processSFMLEvents(event);
        }
    }

    cleanUp();
}

void smViewer::cleanUp()
{
    destroyFboListItems();
    destroyGLContext();

    //Must be set when all cleanup is done
    terminationCompleted = true;
}

smInt smViewer::height(void)
{
    return screenResolutionHeight;
}

smInt smViewer::width(void)
{
    return screenResolutionWidth;
}

smFloat smViewer::aspectRatio(void)
{
    return screenResolutionHeight / screenResolutionWidth;
}
