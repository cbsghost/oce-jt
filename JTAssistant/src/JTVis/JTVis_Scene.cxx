// JT format reading and visualization tools
// Copyright (C) 2015 OPEN CASCADE SAS
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License, or any later
// version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// Copy of the GNU General Public License is in LICENSE.txt and  
// on <http://www.gnu.org/licenses/>.

#define QT_NO_DEBUG_OUTPUT
#define QT_NO_WARNING_OUTPUT

#include "JTVis_Scene.hxx"
#include "JTVis_ScenegraphTasks.hxx"

#include <iostream>

#pragma warning (push, 0)
#include <QOpenGLContext>
#include <QVector3D>
#include <QRectF>
#include <QStack>
#include <QOpenGLTexture>

#include <QImage>
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QFont>
#include <QPen>
#include <QString>
#include <QCoreApplication>
#pragma warning (pop)

#define _USE_MATH_DEFINES
#include <math.h>
#include <typeinfo>

using namespace Eigen;

// =======================================================================
// function : JTVis_Scene
// purpose  :
// =======================================================================
JTVis_Scene::JTVis_Scene (QObject* parent)
  : QObject (parent),
    myIsInitialized (false),
    myTime (0.0f),
    myShaderProgram (NULL),
    myLinesShaderProgram (NULL),
    myTexQuadShaderProgram (NULL),
    myBgShaderProgram (NULL),
    myIdShaderProgram (NULL),
    myTrihedronShaderProgram (NULL),
    myRotation (0.f, 0.f),
    myStartRotation (AngleAxisf::Identity()),
    myZoom (0.f),
    myPanning (0.f, 0.f),
    myGeometrySource (NULL),
    myMousePos (0, 0),
    myViewport (0, 0),
    myCurrentState (0),
    myUnloadCheckPeriod (500),
    myOldFrameCount (1000),
    mySelectionFbo (NULL),
    myScreenshotFbo (NULL),
    isPerformingSelection (false),
    isPerformingMultipleSelection (false),
    mySelectionBuffer (NULL),
    mySmallPartTreshold (10000),
    isPerformingScreenshot (false),
    myFirstReady (true),
    toResetFBOs (false)
{
  myCamera.reset (new JTVis_TargetedCamera());
}

// =======================================================================
// function : ~JTVis_Scene
// purpose  :
// =======================================================================
JTVis_Scene::~JTVis_Scene()
{
  delete myShaderProgram;
  delete myLinesShaderProgram;
  delete myTexQuadShaderProgram;
  delete myBgShaderProgram;
  delete myIdShaderProgram;
  delete myTrihedronShaderProgram;
  delete mySelectionFbo;
  delete myScreenshotFbo;
  delete [] mySelectionBuffer;
}

// =======================================================================
// function : Initialize
// purpose  :
// =======================================================================
void JTVis_Scene::Initialize()
{
  // Initialize resources
  PrepareShaders();
  PreparePartNodes();

  initializeOpenGLFunctions();

  ResetFbos();

  myScreenQuad.reset (new JTVis_QuadGeometry());
  myScreenQuad->InitializeGeometry (myBgShaderProgram);

  myTrihedron.reset (new JTVis_TrihedronGeometry());
  myTrihedron->InitializeGeometry (myTrihedronShaderProgram);

  myTrihedronLabelQuad.reset (new JTVis_QuadGeometry());
  myTrihedronLabelQuad->InitializeGeometry (myTexQuadShaderProgram, NULL, true);

  myPartAggregator.Initialize (myShaderProgram, 2000000);

  PrepareTextTexture (texAxisX, QSize (16, 16), "x", Qt::red, 10);
  PrepareTextTexture (texAxisY, QSize (16, 16), "y", Qt::green, 10);
  PrepareTextTexture (texAxisZ, QSize (16, 16), "z", Qt::blue, 10);

  myHudRenderer.reset (new JTVis_HudRenderer (QSize (250, 150), myTexQuadShaderProgram));

  // Enable depth testing
  glEnable (GL_DEPTH_TEST);
}

// =======================================================================
// function : HandleCamera
// purpose  : Update the camera position and orientation
// =======================================================================
void JTVis_Scene::HandleCamera (float theDeltaTime)
{
  Q_UNUSED (theDeltaTime)

  if (!myIsInitialized)
    return;

  // rotation
  if (!qFuzzyIsNull (myRotation.x() + myRotation.y()))
  {
    myCamera->SetRotation (myStartRotation);
    myCamera->Rotate (myRotation.y(), myRotation.x());
  }

  float aPanningScale = 1.f;

  if (myCamera->IsOrthographic())
  {
    // zooming
    myCamera->SetScale (myCamera->Scale() * powf (2, myZoom * 1e-3f));

    aPanningScale = myCamera->Scale();
  }
  else
  {
    // zooming
    myCamera->SetDistance (myCamera->Distance() * powf (2, myZoom * 1e-3f));

    aPanningScale = myCamera->Distance() * tanf (myCamera->FieldOfView() * 0.5f / 180.f * static_cast<float> (M_PI)) * 2.f;
  }

  // panning
  myCamera->Translate (myPanning.x() * myCamera->Side() * aPanningScale * myCamera->AspectRatio() + 
                       myPanning.y() * myCamera->Up()   * aPanningScale);

  myZoom = 0.f;
  myRotation = QVector2D (0.f, 0.f);
  myPanning = QVector2D (0.f, 0.f);

  // auto z-fit
  FitZ();
}

// =======================================================================
// function : Update
// purpose  :
// =======================================================================
void JTVis_Scene::Update (float theTime)
{
  const float aDeltaTime = theTime - myTime;
  myTime = theTime;

  if (fabsf (myZoom) + fabsf (myRotation.x()) + fabsf (myRotation.y()) +
      fabsf (myPanning.x()) + fabsf (myPanning.y()) > 1e-5f)
    HandleCamera (aDeltaTime);

  if (!myCameraTransition.IsFinished())
  {
    myCameraTransition.Apply (theTime, *myCamera);
    FitZ();
  }

  if (myGeometrySource.isNull())
    return;

  if (myCurrentState % myUnloadCheckPeriod == 0)
  {
    WalkScenegraph (JTVis_ScenegraphTaskPtr (new JTVis_UnloadOldTask (this, myGeometrySource->SceneGraph()->Tree())));
  }

  ++myCurrentState;

  UpdateLods();

  myStats.SmallPartBufferUsage = myPartAggregator.BufferUsage();

}

// =======================================================================
// function : SelectMesh
// purpose  :
// =======================================================================
void JTVis_Scene::SelectMesh (bool isMultipleSelection)
{
  isPerformingSelection = true;

  isPerformingMultipleSelection = isMultipleSelection;

  emit RequestViewUpdate();
}

// =======================================================================
// function : Render
// purpose  :
// =======================================================================
void JTVis_Scene::Render()
{
  if (myGeometrySource.isNull())
    return;

  if (!myIsInitialized)
  {
    Initialize();
    myIsInitialized = true;
  }

  Matrix4f aViewProjectionMatrix    = myCamera->ProjectionMatrix() * myCamera->ViewMatrix();
  Matrix4f aViewProjectionMatrixInv = aViewProjectionMatrix.inverse();

  Matrix4f aViewMatrixInv = myCamera->ViewMatrix().inverse();

  int aTriangleCounter = 0;

  if (isPerformingScreenshot)
  {
    myScreenshotFbo->bind();

    // Make sure we will not bind another FBO.
    isPerformingSelection = false;
  }

  if (isPerformingSelection)
  {
    mySelectionFbo->bind();

    // Clear id-buffer with -1
    glClearColor (-1.f, 0.f, 0.f, 1.f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  else
  {
    glClear (GL_DEPTH_BUFFER_BIT);

    glDepthMask (GL_FALSE);

    // Draw background
    myBgShaderProgram->bind();
    myScreenQuad->Draw();
    myBgShaderProgram->release();

    glDepthMask (GL_TRUE);
  }

  glLineWidth (1.0f);

  unsigned int aLastVaoUsed = 0xffffff;

  NCollection_Handle<BvhTree> aBVH = myBvhGeometry.BVH();
  JTVis_FrustumIntersection anElementList;

  if (mySettings.IsViewCullingEnabled)
  {
    // traverse and draw
    JTVis_Frustum aCameraFrustum (aViewProjectionMatrixInv);
    aCameraFrustum.UpdatePlanes();

    JTVis_BvhTraverser::Traverse (aCameraFrustum, anElementList, myBvhGeometry);

    Standard_Integer theBeg = 0;
    Standard_Integer theEnd = static_cast<Standard_Integer>(anElementList.Elements.size()) - 1;

    Standard_Integer aLftIdx = 0;
    Standard_Integer aRghIdx = static_cast<Standard_Integer>(anElementList.Elements.size()) - 1;

    // divide node list into 2 groups
    do
    {
      while (aLftIdx < theEnd)
      {
        JTVis_PartBvhObject* anObject = static_cast<JTVis_PartBvhObject*> (
          myBvhGeometry.Objects().ChangeValue (aBVH->BegPrimitive (anElementList.Elements[aLftIdx])).operator->());

        if (anObject->PartNode()->TriangleCount > mySmallPartTreshold)
        {
          break;
        }

        ++aLftIdx;
      }

      while (aRghIdx > theBeg)
      {
        JTVis_PartBvhObject* anObject = static_cast<JTVis_PartBvhObject*> (
          myBvhGeometry.Objects().ChangeValue (aBVH->BegPrimitive (anElementList.Elements[aRghIdx])).operator->());

        if (anObject->PartNode()->TriangleCount <= mySmallPartTreshold)
        {
          break;
        }

        --aRghIdx;
      }

      if (aLftIdx <= aRghIdx)
      {
        if (aLftIdx != aRghIdx)
        {
          const Standard_Integer aLftElem = anElementList.Elements[aLftIdx];
          const Standard_Integer aRghElem = anElementList.Elements[aRghIdx];

          anElementList.Elements[aLftIdx] = aRghElem;
          anElementList.Elements[aRghIdx] = aLftElem;
        }

        ++aLftIdx;
        --aRghIdx;
      }
    }
    while (aLftIdx <= aRghIdx);
  }
  else
  {
    // If view culling is disabled just collect all parts into anElementList
    for (int anIdx = 0; anIdx < myBvhGeometry.BVH()->Length(); ++anIdx)
    {
      if (myBvhGeometry.BVH()->IsOuter (anIdx))
        anElementList.Elements.push_back (anIdx);
    }
  }

  // Bind main shader program
  myShaderProgram->bind();

  static GLfloat aSelectionMaterial[] = { 0.9f, 0.9f, 0.9f, 0.0f,
                                          0.0f, 0.0f, 0.0f, 0.0f,
                                          1.0f, 1.0f, 1.0f, 15.f };

  // Set up selection material
  aSelectionMaterial[4] = static_cast<GLfloat> (mySettings.SelectionColor.redF());
  aSelectionMaterial[5] = static_cast<GLfloat> (mySettings.SelectionColor.greenF());
  aSelectionMaterial[6] = static_cast<GLfloat> (mySettings.SelectionColor.blueF());

  int aColorsLoc = myShaderProgram->uniformLocation ("uColors[0]");

  int aMvpLoc       = myShaderProgram->uniformLocation ("uMvpMatrix");
  int aModelViewLoc = myShaderProgram->uniformLocation ("uModelView");
  int aNormalLoc    = myShaderProgram->uniformLocation ("uNormalMatrix");

  // Replace shader program for selection
  if (isPerformingSelection)
    myIdShaderProgram->bind();

  myStats.VisiblePartCount = 0;
  myStats.SizeCulledTriangles = 0;

  int aPartsNotReady = 0;
  bool isRenderingStarted = false;

  float anInvCameraScale = 1.f / myCamera->Scale();

  for (size_t aNodeIdx = 0; aNodeIdx < anElementList.Elements.size(); ++aNodeIdx)
  {
    int aNode = anElementList.Elements.at (aNodeIdx);
    JTCommon_AABB aCurrentBox (aBVH->MinPoint (aNode), aBVH->MaxPoint (aNode));

    const Standard_Integer anObjectIdx = static_cast<Standard_Integer>(aBVH->BegPrimitive (aNode));

    JTVis_PartBvhObject* anObject = static_cast<JTVis_PartBvhObject*> (
      myBvhGeometry.Objects().ChangeValue (anObjectIdx).operator->());

    JTVis_PartNode* aPartNode = anObject->PartNode();

    aPartNode->SetState (myCurrentState);

    if (aPartNode->MeshNode->RequiresDrawing (myCurrentState))
    {
      if (!aPartNode->IsReady())
      {
        ++aPartsNotReady;
        if (!isPerformingSelection)
        {
          myShaderProgram->release();
          myLinesShaderProgram->bind();

          glUniformMatrix4fv (myLinesShaderProgram->uniformLocation ("uMvpMatrix"), 
            1, false, aViewProjectionMatrix.data());

          myLinesShaderProgram->setUniformValue ("uPartColor", aPartNode->BoxGeometry->Color);

          aPartNode->BoxGeometry->Draw (this);

          myShaderProgram->bind();

          aLastVaoUsed = 0xffffff;
        }

        RequestGeometryForNode (aPartNode);
      }
      else
      {
        isRenderingStarted = true;
        if (mySettings.IsSizeCullingEnabled)
        {
          float aProjectedSize = aCurrentBox.Size().norm() * anInvCameraScale * mySettings.LodQuality;

          float aPixelSize = qMax (1.f / myViewport.x(), 1.f / myViewport.y());

          if (aProjectedSize < aPixelSize * 25.f)
          {
            myStats.SizeCulledTriangles += aPartNode->TriangleCount;
            continue;
          }
        }

        myStats.VisiblePartCount += 1;

        Matrix4f anMvpMatrix = aViewProjectionMatrix * aPartNode->Transform();

        if (!isPerformingSelection)
        {
          Matrix4f aModelViewMatrix = myCamera->ViewMatrix() * aPartNode->Transform();
          Matrix4f aModelViewMatrixInv = aPartNode->TransformInversed() * aViewMatrixInv;

          std::set<JTVis_PartNode*>::iterator aNodeIt = mySelectedParts.find (aPartNode);
          if (aNodeIt != mySelectedParts.end())
          {
            glUniform4fv (aColorsLoc, 3, aSelectionMaterial);
          }
          else
          {
            glUniform4fv (aColorsLoc, 3, aPartNode->Material());
          }

          glUniformMatrix4fv (aMvpLoc,       1, false, anMvpMatrix.data());
          glUniformMatrix4fv (aModelViewLoc, 1, false, aModelViewMatrix.data());
          glUniformMatrix4fv (aNormalLoc,    1, false, aModelViewMatrixInv.data());

          aLastVaoUsed = aPartNode->Geometry()->Draw (this, aLastVaoUsed);
        }
        else
        {
          glUniformMatrix4fv (myIdShaderProgram->uniformLocation ("uMvpMatrix"), 1, false, anMvpMatrix.data());

          float aFloatId = (float) aPartNode->PartNodeId;
          glUniform1f (myIdShaderProgram->uniformLocation ("uObjectId"), aFloatId);

          aPartNode->Geometry()->Draw (this, myIdShaderProgram);

          aLastVaoUsed = 0xffffff;
        }

        aTriangleCounter += aPartNode->TriangleCount;
      }
    }
  }

  if (aLastVaoUsed != 0xffffff)
  {
    static QVertexArrayObjectHelper aHelper (myContext);
    aHelper.glBindVertexArray (0);
  }

  if (isPerformingSelection)
  {
#ifndef QT_OPENGL_ES_2

    mySelectionFbo->release();

    glBindTexture (GL_TEXTURE_2D, mySelectionFbo->texture());
    glGetTexImage (GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, mySelectionBuffer);
    glBindTexture (GL_TEXTURE_2D, 0);

    int aNodeId = static_cast<int> (mySelectionBuffer [myMousePos.y() * myViewport.x() + myMousePos.x()]);
#else
    int aSelectedPartNodeID = -1;

    glReadPixels (myMousePos.x(), myMousePos.y(), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)&aSelectedPartNodeID);
    --aSelectedPartNodeID;

    myIdShaderProgram->release();
    mySelectionFbo->release();
    isPerformingSelection = false;
#endif

#ifndef QT_OPENGL_ES_2
    PerformSelection (aNodeId, isPerformingMultipleSelection);
#else
    PerformSelection (aSelectedPartNodeID, isPerformingMultipleSelection);
#endif
    return;
  }

  if (isPerformingScreenshot)
  {
    myScreenshotFbo->release();
    myScreenshotFbo->toImage().save (myScreenshotPath);
    isPerformingScreenshot = false;
  }

#ifdef DEBUG_KEYS
  myLinesShaderProgram->bind();
  glUniformMatrix4fv (myLinesShaderProgram->uniformLocation ("uMvpMatrix"), 
    1, false, aViewProjectionMatrix.data());

  myLinesShaderProgram->setUniformValue ("uPartColor", QVector4D (1.f, 1.f, 1.f, 1.f));
  foreach (JTVis_GraphicObjectPtr anObj, myHelperObjects)
  {
    anObj->Draw (this);
  }
  myLinesShaderProgram->release();
#endif

  myStats.VisibleTriangleCount = aTriangleCounter;

  if (mySettings.IsTrihedronVisible)
  {
    DrawTrihedron();
  }

  if (mySettings.IsStatsOsdVisible)
  {
    myHudRenderer->SetPosition (QPoint (myViewport.x() - 225, myViewport.y() - 200));
    myHudRenderer->Update (myStats, this);
    myHudRenderer->Draw (this);
  }

  glViewport (0, 0, myViewport.x(), myViewport.y());

  if (isRenderingStarted && aPartsNotReady < 10 && myFirstReady)
  {
    // Loading of late loaded complete
    emit LoadingComplete();
    myFirstReady = false;

    if (mySettings.IsBenchmarkingMode)
    {
      JTCommon_Profiler& aProfiler = JTCommon_Profiler::GetProfiler();
      aProfiler.WriteElapsed ("parts");

      std::cout << "Parts loading time: " << aProfiler.Values()["parts"] << " ms\n" << std::endl;

      std::cout << "Total time: " << aProfiler.Values()["bvh"] +
                                     aProfiler.Values()["loading"] +
                                     aProfiler.Values()["parts"] << " ms\n" << std::endl;
      QCoreApplication::exit();
    }
  }
}

// =======================================================================
// function : DrawTrihedron
// purpose  :
// =======================================================================
void JTVis_Scene::DrawTrihedron()
{
  const float aTrihedronAreaSize = 120.f;
  const float aPixelSize = 1.f / aTrihedronAreaSize;
  glViewport (0, 0,
              static_cast<GLsizei> (aTrihedronAreaSize),
              static_cast<GLsizei> (aTrihedronAreaSize));

  glDisable (GL_DEPTH_TEST);
  glLineWidth (2.0f);
  myTrihedronShaderProgram->bind();

  Affine3f aTransf (myCamera->Rotation());

  Vector4f aTranslation (0.f, -10.f * aPixelSize, 0.f, 0.f);

  glUniformMatrix4fv (myTrihedronShaderProgram->uniformLocation ("uMvpMatrix"), 
    1, false, aTransf.data());

  glUniform4fv (myTrihedronShaderProgram->uniformLocation ("uTranslation"),
    1, aTranslation.data());

  myTrihedron->Draw (this);

  myTrihedronShaderProgram->release();

  glEnable (GL_TEXTURE_2D); 

#ifndef QT_OPENGL_ES_2
  glEnable (GL_ALPHA_TEST);
  glAlphaFunc (GL_GREATER, 0.5f);
#else
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

  myTexQuadShaderProgram->bind();

  Vector4f anXTranslation = aTransf * Vector4f (0.88f, 0.0f, 0.0f, 0.f);
  Vector4f anYTranslation = aTransf * Vector4f (0.0f, 0.88f, 0.0f, 0.f);
  Vector4f aZTranslation  = aTransf * Vector4f (0.0f, 0.0f, 0.88f, 0.f);

  myTexQuadShaderProgram->setUniformValue ("uColorTexture", 0);
  myTexQuadShaderProgram->setUniformValue ("uScale", 16.f * aPixelSize);

  glBindTexture (GL_TEXTURE_2D, myTextures[texAxisX]);
  glUniform4fv (myTexQuadShaderProgram->uniformLocation ("uTranslation"), 1, anXTranslation.data());
  myTrihedronLabelQuad->Draw();

  glBindTexture (GL_TEXTURE_2D, myTextures[texAxisY]);
  glUniform4fv (myTexQuadShaderProgram->uniformLocation ("uTranslation"), 1, anYTranslation.data());
  myTrihedronLabelQuad->Draw();

  glBindTexture (GL_TEXTURE_2D, myTextures[texAxisZ]);
  glUniform4fv (myTexQuadShaderProgram->uniformLocation ("uTranslation"), 1, aZTranslation.data());
  myTrihedronLabelQuad->Draw();

  myTexQuadShaderProgram->release();

#ifndef QT_OPENGL_ES_2
  glDisable (GL_ALPHA_TEST);
#else
  glDisable (GL_BLEND);
#endif

  glDisable (GL_TEXTURE_2D); 
  glBindTexture (GL_TEXTURE_2D, 0);

  glEnable (GL_DEPTH_TEST);
}

// =======================================================================
// function : PerformSelection
// purpose  :
// =======================================================================
void JTVis_Scene::PerformSelection (int theNodeId, bool isMultipleSelection)
{
  if (!isMultipleSelection || theNodeId < 0)
  {
    mySelectedParts.clear();
    emit RequestClearSelection();
  }

  if (theNodeId >= 0 && static_cast<unsigned int> (theNodeId) < myPartNodes.size())
  {
    JTVis_PartNode* aNode = myPartNodes[theNodeId].data();

    std::vector<JTVis_PartNode*> aCollectedNodes;

    if (aNode->RangeNode != 0)
    {
      QStack<JTData_Node*> aStack;

      aStack.push (aNode->RangeNode);
      emit RequestSelection (aNode->RangeNode);

      for (;;)
      {
        if (aStack.isEmpty())
        {
          break;
        }

        JTData_Node* aNode = aStack.pop();

        if (typeid (*aNode) == typeid (JTData_MeshNode))
        {
          JTData_MeshNode* aMesh = static_cast<JTData_MeshNode*> (aNode);

          aCollectedNodes.push_back (myMeshToPartMap[aMesh].data());
        }
        else
        {
          JTData_GroupNode* aGroup = static_cast<JTData_GroupNode*> (aNode);

          for (size_t anIdx = 0; anIdx < aGroup->Children.size(); ++anIdx)
          {
            aStack.push (aGroup->Children.at (anIdx).data());
          }
        }
      }
    }
    else
    {
      aCollectedNodes.push_back (aNode);
      emit RequestSelection (aNode->MeshNode);
    }

    if (isMultipleSelection)
    {
      std::set<JTVis_PartNode*>::iterator anIt = mySelectedParts.find (aNode);
      if (anIt != mySelectedParts.end())
      {
        foreach (JTVis_PartNode* aPartNode, aCollectedNodes)
        {
          mySelectedParts.erase (aPartNode);
        }
      }
      else
      {
        foreach (JTVis_PartNode* aPartNode, aCollectedNodes)
        {
          mySelectedParts.insert (aPartNode);
        }
      }
    }
    else
    {
      foreach (JTVis_PartNode* aPartNode, aCollectedNodes)
      {
        mySelectedParts.insert (aPartNode);
      }
    }
  } 

  isPerformingSelection = false;
  emit RequestViewUpdate();
}

// =======================================================================
// function : RequestGeometryForNode
// purpose  :
// =======================================================================
void JTVis_Scene::RequestGeometryForNode (JTVis_PartNode* theNode)
{
  QMap<JTData_MeshNodeSource*, JTVis_PartGeometryPtr>::iterator anIter = myInstancedMeshes.find (theNode->MeshNode->Source().data());

  if (anIter == myInstancedMeshes.end())
  {
    JTCommon_TriangleDataPtr aData = theNode->MeshNode->Source()->RequestTriangulation (0, this);

    if (!aData.isNull() && aData->Data->Vertices().Count()  != 0)
    {
      JTVis_PartGeometryPtr aNewPart = JTVis_PartGeometryPtr (new JTVis_PartGeometry());

      if (aData->Data->Indices().Count() / 3 > mySmallPartTreshold)
      {
        aNewPart->InitializeGeometry (myShaderProgram, aData);
      }
      else
      {
        aNewPart->InitializeGeometry (this, aData, myPartAggregator);
        if (aNewPart->IsReady() == false)
          aNewPart->InitializeGeometry (myShaderProgram, aData);
      }

      theNode->SetGeometry (aNewPart);
      myInstancedMeshes.insert (theNode->MeshNode->Source().data(), aNewPart);

      theNode->TriangleCount = aNewPart->TriangleCount();
    }
  }
  else
  {
    theNode->SetGeometry (*anIter);
    theNode->TriangleCount = (*anIter)->TriangleCount();
  }
}

// =======================================================================
// function : ResetFbos
// purpose  :
// =======================================================================
void JTVis_Scene::ResetFbos()
{
  {
    delete mySelectionFbo;
    QOpenGLFramebufferObjectFormat anFboFormat;
#ifndef QT_OPENGL_ES_2
    anFboFormat.setInternalTextureFormat (GL_R32F);
#else
    anFboFormat.setInternalTextureFormat (GL_RGBA);
#endif

    anFboFormat.setAttachment (QOpenGLFramebufferObject::Depth);

    mySelectionFbo = new QOpenGLFramebufferObject (myViewport.x(), myViewport.y(), anFboFormat);
    mySelectionFbo->bindDefault(); // solve issue with Qt GL integration on Linux.
  }

  {
    delete myScreenshotFbo;
    QOpenGLFramebufferObjectFormat anFboFormat;
#ifndef QT_OPENGL_ES_2
    anFboFormat.setInternalTextureFormat (GL_RGBA8);

    if (myMaxSamples >= 16)
    {
      anFboFormat.setSamples (16);
    }
    else if (myMaxSamples >= 8)
    {
      anFboFormat.setSamples (8);
    }
    else if (myMaxSamples >= 4)
    {
      anFboFormat.setSamples (4);
    }
#else
    anFboFormat.setInternalTextureFormat (GL_RGBA);
#endif

    anFboFormat.setAttachment (QOpenGLFramebufferObject::Depth);

    myScreenshotFbo = new QOpenGLFramebufferObject (myViewport.x(), myViewport.y(), anFboFormat);
    myScreenshotFbo->bindDefault(); // solve issue with Qt GL integration on Linux.
  }
}

// =======================================================================
// function : Resize
// purpose  :
// =======================================================================
void JTVis_Scene::Resize (int theWidth, int theHeight)
{
  myViewport = QPoint (theWidth, theHeight);

#ifndef QT_OPENGL_ES_2
  ResetFbos();
#else
  toResetFBOs = true;
#endif

  delete[] mySelectionBuffer;
  mySelectionBuffer = new float [theWidth * theHeight];

  // Update the projection matrix
  float anAspect = static_cast<float> (theWidth) / static_cast<float> (theHeight);

  myCamera->SetAspectRatio (anAspect);
}

// =======================================================================
// function : PrepareShaders
// purpose  :
// =======================================================================
void JTVis_Scene::PrepareShaders()
{
  myShaderProgram = new QOpenGLShaderProgram (/*this*/);
  myShaderProgram->addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/shaders/src/JTVis/Shaders/default.vert");
  myShaderProgram->addShaderFromSourceFile (QOpenGLShader::Fragment, ":/shaders/src/JTVis/Shaders/default.frag");
  myShaderProgram->link();

  myLinesShaderProgram = new QOpenGLShaderProgram (/*this*/);
  myLinesShaderProgram->addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/shaders/src/JTVis/Shaders/lineShader.vert");
  myLinesShaderProgram->addShaderFromSourceFile (QOpenGLShader::Fragment, ":/shaders/src/JTVis/Shaders/lineShader.frag");
  myLinesShaderProgram->link();

  myTexQuadShaderProgram = new QOpenGLShaderProgram (/*this*/);
  myTexQuadShaderProgram->addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/shaders/src/JTVis/Shaders/texQuadShader.vert");
  myTexQuadShaderProgram->addShaderFromSourceFile (QOpenGLShader::Fragment, ":/shaders/src/JTVis/Shaders/texQuadShader.frag");
  myTexQuadShaderProgram->link();

  myBgShaderProgram = new QOpenGLShaderProgram (/*this*/);
  myBgShaderProgram->addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/shaders/src/JTVis/Shaders/bgShader.vert");
  myBgShaderProgram->addShaderFromSourceFile (QOpenGLShader::Fragment, ":/shaders/src/JTVis/Shaders/bgShader.frag");
  myBgShaderProgram->link();

  myIdShaderProgram = new QOpenGLShaderProgram (/*this*/);
  myIdShaderProgram->addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/shaders/src/JTVis/Shaders/idShader.vert");
  myIdShaderProgram->addShaderFromSourceFile (QOpenGLShader::Fragment, ":/shaders/src/JTVis/Shaders/idShader.frag");
  myIdShaderProgram->link();

  myTrihedronShaderProgram = new QOpenGLShaderProgram (/*this*/);
  myTrihedronShaderProgram->addShaderFromSourceFile (QOpenGLShader::Vertex,   ":/shaders/src/JTVis/Shaders/trihedronShader.vert");
  myTrihedronShaderProgram->addShaderFromSourceFile (QOpenGLShader::Fragment, ":/shaders/src/JTVis/Shaders/trihedronShader.frag");
  myTrihedronShaderProgram->link();
}

// =======================================================================
// function : PrepareTextTexture
// purpose  :
// =======================================================================
void JTVis_Scene::PrepareTextTexture (const int theTextureId, 
                                      const QSize&   theSize,
                                      const QString& theText,
                                      const QColor&  theColor,
                                      const int      theFontSize)
{
  QRect aRect (0, 0, theSize.width(), theSize.height());
  QImage anImage (aRect.size(), QImage::Format_ARGB32);
  anImage.fill (QColor (0, 0, 0, 0));
  QPainter aPainter;
  aPainter.begin (&anImage);
    aPainter.setPen (QPen (theColor, 0));
    QFont aFont;
    aFont.setPointSize (theFontSize);
    aFont.setFamily ("Courier");
    aPainter.setFont (aFont);
    aRect.setTop (aRect.top());
    aRect.setBottom (aRect.bottom());
    aPainter.drawText (aRect, theText, QTextOption (Qt::AlignCenter));
  aPainter.end();

  GLuint aTexture;
  glGenTextures (1, &aTexture);
  glBindTexture (GL_TEXTURE_2D, aTexture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#ifndef QT_OPENGL_ES_2
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, anImage.width(), anImage.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, anImage.bits());
#else
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, anImage.width(), anImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, anImage.bits());
#endif

  myTextures.insert (theTextureId, aTexture);
}

// =======================================================================
// function : SelectNode
// purpose  :
// =======================================================================
void JTVis_Scene::SelectNode (const JTData_NodePtr& theNode, bool isMultipleSelection)
{
  if (!isMultipleSelection)
    mySelectedParts.clear();
  WalkScenegraph (JTVis_ScenegraphTaskPtr (new JTVis_SelectTask (this, theNode)));

  emit RequestViewUpdate();
}

// =======================================================================
// function : ClearSelection
// purpose  :
// =======================================================================
void JTVis_Scene::ClearSelection()
{
  mySelectedParts.clear();

  emit RequestViewUpdate();
}

// =======================================================================
// function : UpdateLods
// purpose  :
// =======================================================================
void JTVis_Scene::UpdateLods()
{
  if (!myIsInitialized)
    return;

  JTData_SceneGraph* aSceneGraph = myGeometrySource->SceneGraph();

  QStack<JTData_Node*> aStack;

  if (!aSceneGraph || aSceneGraph->Tree().isNull())
  {
    return;
  }

  myVisibleBounds.Clear();

  const float aPixelSize = qMax (1.f / myViewport.x(), 1.f / myViewport.y());
  const float aBaseSize  = aPixelSize * 350.f;

  float aCameraInvScale = 1.f / myCamera->Scale();

  int aTriangleCounter = 0;

  aStack.push (aSceneGraph->Tree().data());

  for (;;)
  {
    if (aStack.isEmpty())
    {
      break;
    }

    JTData_Node* aNode = aStack.pop();

    if (!aNode->IsVisible())
      continue;

    if (typeid (*aNode) == typeid (JTData_MeshNode))
    {
      JTData_MeshNode* aMesh = static_cast<JTData_MeshNode*> (aNode);

      aMesh->SetState (myCurrentState);

      JTVis_PartNodePtr aPartNode = myMeshToPartMap[aMesh];
      if (!aPartNode.isNull())
        myVisibleBounds.Combine (aPartNode->Bounds);

      aTriangleCounter += aMesh->Source()->TriangleCount();
    }
    else if (typeid (*aNode) == typeid (JTData_RangeLODNode))
    {
      JTData_RangeLODNode* aRangeLOD = static_cast<JTData_RangeLODNode*> (aNode);

      Vector4f aLongestDiagonal = aRangeLOD->Box.CornerMax() - aRangeLOD->Box.CornerMin();
      float aDiagonalSize = aLongestDiagonal.norm();

      if (!myCamera->IsOrthographic())
      {
        Vector3f aLodCenter (aRangeLOD->Box.Center().x(), aRangeLOD->Box.Center().y(), aRangeLOD->Box.Center().z());
        aCameraInvScale = (myCamera->EyePosition() - aLodCenter).norm() *
          tanf (myCamera->FieldOfView() * 0.5f / 180.f * static_cast<float> (M_PI)) * 2.f;
        aCameraInvScale = 1.f / aCameraInvScale;
      }

      float aProjectedSize = aDiagonalSize * aCameraInvScale;

      float aSize = aProjectedSize * mySettings.LodQuality;

      int aSelectedLod = 0;
      Standard_Size aLodCount = aRangeLOD->Ranges().size();

      if (aLodCount != 0 && aSize >= aBaseSize * powf (2.f, 1e-2f))
      {
        for (Standard_Size anIdx = 1; anIdx <= aLodCount; ++anIdx)
        {
          if ((size_t )aSelectedLod + 1 >= aRangeLOD->Children.size())
          {
            break;
          }

          ++aSelectedLod;

          if (aSize < (aBaseSize * powf (2.f, (float )anIdx)))
            break;   
        }
      }

      aStack.push (aRangeLOD->Children.at (aRangeLOD->Children.size() - 1 - aSelectedLod).data());
    }
    else
    {
      JTData_GroupNode* aGroup = static_cast<JTData_GroupNode*> (aNode);

      for (size_t anIdx = 0; anIdx < aGroup->Children.size(); ++anIdx)
      {
        aStack.push (aGroup->Children.at (anIdx).data());
      }
    }
  }

  myStats.FullTriangleCount = aTriangleCounter;
}

// =======================================================================
// function : WalkScenegraph
// purpose  :
// =======================================================================
void JTVis_Scene::WalkScenegraph (JTVis_ScenegraphTaskPtr theTask)
{
  JTData_SceneGraph* aSceneGraph = myGeometrySource->SceneGraph();

  QStack<JTVis_ScenegraphTaskPtr> aTaskStack;

  if (!aSceneGraph || aSceneGraph->Tree().isNull())
  {
    return;
  }

  aTaskStack.push (theTask);

  for (;;)
  {
    if (aTaskStack.isEmpty())
    {
      break;
    }

    JTVis_ScenegraphTaskPtr aTask = aTaskStack.pop();

    aTask->Perform  (aTaskStack);
    aTask->Traverse (aTaskStack);
  }
}

// =======================================================================
// function : PreparePartNodes
// purpose  :
// =======================================================================
void JTVis_Scene::PreparePartNodes()
{
  if (myGeometrySource.isNull())
    return;

  JTData_SceneGraph* aSceneGraph = myGeometrySource->SceneGraph();

  myPartNodes.clear();
  myBvhGeometry.Clear();

  WalkScenegraph (JTVis_ScenegraphTaskPtr (new JTVis_PrepareNodeTask(this, aSceneGraph->Tree())));

  JTVis_GenerateCentersTask::Box       = JTCommon_AABB();
  JTVis_GenerateCentersTask::GlobalBox = JTCommon_AABB();
  WalkScenegraph (JTVis_ScenegraphTaskPtr (new JTVis_GenerateCentersTask (this, myGeometrySource->SceneGraph()->Tree())));
  aSceneGraph->GenerateRanges (JTVis_GenerateCentersTask::GlobalBox);

  if (mySettings.IsBenchmarkingMode)
    JTCommon_Profiler::GetProfiler().Start();

  myBvhGeometry.MarkDirty();
  myBvhGeometry.BVH(); // build BVH

  if (mySettings.IsBenchmarkingMode)
  {
    JTCommon_Profiler::GetProfiler().WriteElapsed ("bvh");
    std::cout << "Loading file structure: " << JTCommon_Profiler::GetProfiler().Values()["loading"] << " ms" << std::endl;
    std::cout << "Building BVH: " << JTCommon_Profiler::GetProfiler().Values()["bvh"] << " ms" << std::endl;

    JTCommon_Profiler::GetProfiler().Start();
  }

  NCollection_Handle<BvhTree> aBVH = myBvhGeometry.BVH(); // just a reference
  if (aBVH.IsNull()
   || aBVH->Length() == 0)
  {
    myGlobalBounds = JTCommon_AABB (BVH_Vec4f (-10.0f, -10.0f, -10.0f, 0.0f),
                                    BVH_Vec4f ( 10.0f,  10.0f,  10.0f, 0.0f));
    FitAll (fmFitAll, svIso);
    return;
  }

  BVH_Vec4f aMin = aBVH->MinPoint (0);
  BVH_Vec4f aMax = aBVH->MaxPoint (0);
  myGlobalBounds = JTCommon_AABB (aMin, aMax);

  myVisibleBounds = myGlobalBounds;

  // camera setup
  FitAll (fmFitAll, svIso);

  // Basic structures loaded
  emit LoadingComplete();
}

// =======================================================================
// function : getStandardViewRotation
// purpose  :
// =======================================================================
AngleAxisf getStandardViewRotation (JTVis_StandardView theView)
{
  switch (theView)
  {
  case svTop:
    return AngleAxisf (AngleAxisf ((float) M_PI_2, Vector3f (1.f, 0.f, 0.f)));
  case svBottom:
    return AngleAxisf (AngleAxisf ((float)-M_PI_2, Vector3f (1.f, 0.f, 0.f)));
  case svLeft:
    return AngleAxisf (AngleAxisf ((float) M_PI_2, Vector3f (0.f, 1.f, 0.f)));
  case svRight:
    return AngleAxisf (AngleAxisf ((float)-M_PI_2, Vector3f (0.f, 1.f, 0.f)));
  case svFront:
    return AngleAxisf (AngleAxisf (0.f, Vector3f (0.f, 1.f, 0.f)));
  case svBack:
    return AngleAxisf (AngleAxisf ((float )M_PI, Vector3f (0.f, 1.f, 0.f)));
  case svIso:
    return AngleAxisf (AngleAxisf ((float) M_PI_4 * 0.5f, Vector3f (1.f, 0.f, 0.f)) *
                       AngleAxisf ((float)-M_PI_4,        Vector3f (0.f, 1.f, 0.f)));
  default:
    return AngleAxisf::Identity();
  }
}

// =======================================================================
// function : FitAll
// purpose  :
// =======================================================================
void JTVis_Scene::FitAll (const JTVis_FitMode theFitMode,
                          const JTVis_StandardView theDesiredView)
{
  JTCommon_AABB aBounds;

  if (theFitMode == fmFitAll)
  {
    aBounds = myGlobalBounds;
  }
  else if (theFitMode == fmFitVisible)
  {
    aBounds = myVisibleBounds;
  }
  else if (theFitMode == fmFitSelected)
  {
    foreach (JTVis_PartNode* aNode, mySelectedParts)
    {
      aBounds.Combine (aNode->Bounds);
    }
  }

  if (!aBounds.IsValid())
    return;

  JTVis_TargetedCamera* aFitCamera = myCamera.data();
  JTVis_TargetedCamera* aTempCamera = NULL;
  if (theDesiredView != svDontChange)
  {
    aTempCamera = new JTVis_TargetedCamera(*myCamera);
    aTempCamera->SetRotation (getStandardViewRotation (theDesiredView));
    aFitCamera = aTempCamera;
  }

  Vector4f aSizeScene = aBounds.Size();
  Matrix4f aWorldToView = aFitCamera->ViewMatrix();

  Vector4f aSceneCenter = (aBounds.CornerMin() + aBounds.CornerMax()) * 0.5f;
  aSceneCenter.w() = 1.f;

  JTCommon_AABB aViewSpaceBox;
  for (int aX = 0; aX <= 1; ++aX)
  {
    for (int aY = 0; aY <= 1; ++aY)
    {
      for (int aZ = 0; aZ <= 1; ++aZ)
      {
        Vector4f aCorner = aBounds.CornerMin() +
          Vector4f (aSizeScene.x(), 0.f, 0.f, 0.f) * static_cast<float> (aX) +
          Vector4f (0.f, aSizeScene.y(), 0.f, 0.f) * static_cast<float> (aY) +
          Vector4f (0.f, 0.f, aSizeScene.z(), 0.f) * static_cast<float> (aZ);

        aViewSpaceBox.Add (aWorldToView * aCorner);
      }
    }
  }

  AngleAxisf aNewRotation = aFitCamera->Rotation();
  delete aTempCamera;

  Vector3f   aNewTarget (aSceneCenter.x(), aSceneCenter.y(), aSceneCenter.z());
  float      aNewSize = 1.3f * qMax (aViewSpaceBox.Size().x() / myCamera->AspectRatio(), aViewSpaceBox.Size().y());

  if (theDesiredView != svDontChange)
    aNewRotation = getStandardViewRotation (theDesiredView);

  if (mySettings.IsCameraAnimated)
  {
    myCameraTransition = JTVis_CameraTransition (*myCamera, aNewRotation, aNewTarget, aNewSize, 1.0f, myAnimationCallback);
    myCameraTransition.Start();
  }
  else
  {
    myCamera->SetRotation (aNewRotation);
    myCamera->SetTarget (aNewTarget);
    myCamera->SetScale (aNewSize);

    FitZ();
    emit RequestViewUpdate();
  }
}

// =======================================================================
// function : FitZ
// purpose  :
// =======================================================================
void JTVis_Scene::FitZ()
{
  if (!myGlobalBounds.IsValid())
    return;

  Vector4f aSizeScene = myGlobalBounds.Size();
  Matrix4f aWorldToView = myCamera->ViewMatrix();

  Vector4f aSceneCenter = (myGlobalBounds.CornerMin() + myGlobalBounds.CornerMax()) * 0.5f;
  aSceneCenter.w() = 1.f;

  JTCommon_AABB aViewSpaceBox;
  for (int aX = 0; aX <= 1; ++aX)
  {
    for (int aY = 0; aY <= 1; ++aY)
    {
      for (int aZ = 0; aZ <= 1; ++aZ)
      {
        Vector4f aCorner = myGlobalBounds.CornerMin() +
          Vector4f (aSizeScene.x(), 0.f, 0.f, 0.f) * static_cast<float> (aX) +
          Vector4f (0.f, aSizeScene.y(), 0.f, 0.f) * static_cast<float> (aY) +
          Vector4f (0.f, 0.f, aSizeScene.z(), 0.f) * static_cast<float> (aZ);

        aViewSpaceBox.Add (aWorldToView * aCorner);
      }
    }
  }

  float aZFar  = -aViewSpaceBox.CornerMin().z();
  float aZNear = -aViewSpaceBox.CornerMax().z();

  if (!myCamera->IsOrthographic())
  {
    aZNear = qMax (aZNear, aSizeScene.norm() * 0.01f);
    aZFar = qMax (aZFar, 1.f);
  }

  myCamera->SetZFar  (aZFar);
  myCamera->SetZNear (aZNear);

}

// =======================================================================
// function : SetCameraStandardView
// purpose  :
// =======================================================================
void JTVis_Scene::SetCameraStandardView (JTVis_StandardView theView)
{
  AngleAxisf aRotation = getStandardViewRotation (theView);

  if (mySettings.IsCameraAnimated)
  {
    myCameraTransition = JTVis_CameraTransition (*myCamera, aRotation, myCamera->Target(), myCamera->Scale(), 1.0f, myAnimationCallback);
    myCameraTransition.Start();
  }
  else
  {
    myCamera->SetRotation (aRotation);

    FitZ();
    emit RequestViewUpdate();
  }
}

// =======================================================================
// function : DebugKeyHandler
// purpose  :
// =======================================================================
void JTVis_Scene::DebugKeyHandler (int theKey)
{
#ifdef DEBUG_KEYS
  switch (theKey)
  {
    case Qt::Key_F:
    {
      Matrix4f aViewProjectionMatrix    = myCamera->ProjectionMatrix() * myCamera->ViewMatrix();
      Matrix4f aViewProjectionMatrixInv = aViewProjectionMatrix.inverse();

      JTVis_Frustum aCameraFrustum (aViewProjectionMatrixInv);
      aCameraFrustum.UpdatePlanes();
      JTVis_FrustumGeometryPtr aFrustumGeom (new JTVis_FrustumGeometry (aCameraFrustum));
      aFrustumGeom->InitializeGeometry (myLinesShaderProgram);
      myHelperObjects.clear();
      myHelperObjects.push_back (aFrustumGeom);

      break;
    }
  }
#else
  Q_UNUSED (theKey);
#endif
}
