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

#ifndef JTVIS_SCENE_H
#define JTVIS_SCENE_H

#pragma warning (push, 0)
#include <QMatrix4x4>
#include <QStringList>
#include <QtGui/QVector2D>

#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>

#ifndef QT_OPENGL_ES_2
  #include <QtGui/QOpenGLFunctions_2_0>
  typedef QOpenGLFunctions_2_0 OpenGLFunctions;
#else
  #include <QtGui/QOpenGLFunctions_ES2>
  typedef QOpenGLFunctions_ES2 OpenGLFunctions;
#endif
#pragma warning (pop)

#pragma warning (push, 0)
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#pragma warning (pop)

#include <BVH/BVH_Geometry.hxx>
#include <BVH/BVH_Tree.hxx>

#include "JTVis_HudRenderer.hxx"
#include "JTVis_GraphicObject.hxx"
#include "JTVis_PartNode.hxx"
#include "JTVis_AABBGeometry.hxx"
#include "JTVis_Frustum.hxx"
#include "JTVis_QuadGeometry.hxx"
#include "JTVis_TrihedronGeometry.hxx"
#include <JTData/JTData_GeometrySource.hxx>
#include <JTCommon_Utils.hxx>

#include "JTVis_BvhGeometryWrapper.hxx"

#include "JTVis_TargetedCamera.hxx"
#include "JTVis_CameraTransition.hxx"

#include <set>

//! Statistical data of visualization process.
struct JTVis_Stats
{
  JTVis_Stats (int theVisibleTriangleCount = 0, 
               int theFullTriangleCount    = 0,
               int theSizeCulledTriangles  = 0,
               int thePartCount            = 0,
               int theVisiblePartCount     = 0,
               int theSmallPartBufferUsage = 0)
  : VisibleTriangleCount (theVisibleTriangleCount),
    FullTriangleCount    (theFullTriangleCount),
    SizeCulledTriangles  (theSizeCulledTriangles),
    PartCount            (thePartCount),
    VisiblePartCount     (theVisiblePartCount),
    SmallPartBufferUsage (theSmallPartBufferUsage)
  {}

  int VisibleTriangleCount; //!< Count of visible triangles.
  int FullTriangleCount;    //!< Full triangle count for current LOD configuration.
  int SizeCulledTriangles;  //!< Count of size culled triangles.

  int PartCount;            //!< Full part count in scene.
  int VisiblePartCount;     //!< Count of visible parts.

  int SmallPartBufferUsage; //!< Utilization of scene SmallPartBuffer.
};

//! Visualization settings.
struct JTVis_Settings
{
  JTVis_Settings (bool  theViewCullingEnabled = true,
                  bool  theSizeCullingEnabled = true,
                  bool  theTrihedronVisible   = false,
                  bool  theStatsOsdVisible    = true,
                  bool  theBenchmarkingMode    = false,
                  float theLodQuality         = 1.f,
                  bool  theCameraAnimated     = true,
                  QColor theSelectionColor    = QColor (0, 255, 255))
  : IsViewCullingEnabled (theViewCullingEnabled),
    IsSizeCullingEnabled (theSizeCullingEnabled),
    IsStatsOsdVisible    (theStatsOsdVisible),
    IsTrihedronVisible   (theTrihedronVisible),
    IsBenchmarkingMode   (theBenchmarkingMode),
    LodQuality           (theLodQuality),
    IsCameraAnimated     (theCameraAnimated),
    SelectionColor       (theSelectionColor)
  {}

  bool IsViewCullingEnabled; //!< Indicates when viewer will perform view area culling.
  bool IsSizeCullingEnabled; //!< Indicates when viewer will perform size culling.

  bool IsStatsOsdVisible;    //!< Indicates when statistics OSD is visible.
  bool IsTrihedronVisible;   //!< Indicates when trihedron is need to be rendered.

  bool IsBenchmarkingMode;   //!< Indicates when JTViewer is ran in benchmark mode.

  float LodQuality;          //!< Scales LOD settings to adjust quality of visualization.

  bool IsCameraAnimated;     //!< Indicates when camera is need to be animated.

  QColor SelectionColor;     //!< Color of selected objects.

};

//! Helper object to load OpenGL VAO functions.
class QVertexArrayObjectHelper
{
public:

  QVertexArrayObjectHelper (QOpenGLContext *context)
  {
    Q_ASSERT (context);
#if !defined (QT_OPENGL_ES_2)
    GenVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenVertexArrays"));
    DeleteVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteVertexArrays"));
    BindVertexArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glBindVertexArray"));
#else
    GenVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenVertexArraysOES"));
    DeleteVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteVertexArraysOES"));
    BindVertexArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glBindVertexArrayOES"));
#endif
  }

  inline void glGenVertexArrays(GLsizei n, GLuint *arrays)
  {
    GenVertexArrays(n, arrays);
  }

  inline void glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
  {
    DeleteVertexArrays(n, arrays);
  }

  inline void glBindVertexArray(GLuint array)
  {
    BindVertexArray(array);
  }

private:

  // Function signatures are equivalent between desktop core, ARB and ES 2 extensions
  void (QOPENGLF_APIENTRYP GenVertexArrays)(GLsizei n, GLuint *arrays);
  void (QOPENGLF_APIENTRYP DeleteVertexArrays)(GLsizei n, const GLuint *arrays);
  void (QOPENGLF_APIENTRYP BindVertexArray)(GLuint array);
};

typedef BVH_Tree<float, 4> BvhTree;

class JTVis_ScenegraphTask;

typedef QSharedPointer<JTVis_ScenegraphTask> JTVis_ScenegraphTaskPtr;

typedef QSharedPointer<JTCommon_Callback<JTData_Node*> > JTVis_SelectionCallbackPtr;

typedef QSharedPointer<JTCommon_Callback<void*> > JTVis_ClearSelectionCallbackPtr;

typedef QSharedPointer<JTCommon_Callback<bool> > JTVis_NeedToUpdateCallbackPtr;

typedef QSharedPointer<JTCommon_Callback<bool> > JTVis_AnimationModeCallbackPtr;

//! Enum for trihedron label textures.
enum JTVis_Textures
{
  texAxisX, texAxisY, texAxisZ
};

//! Enum for fit modes, applied in FitAll().
enum JTVis_FitMode
{
  fmFitAll, fmFitVisible, fmFitSelected
};

//! Enum for standard views applied in FitAll() and SetCameraStandardView().
enum JTVis_StandardView
{
  svTop, svBottom, svLeft, svRight, svFront, svBack, svIso, svDontChange
};

//! Main scene class responsible for visualization of JT file contents. 
class JTVis_Scene: public QObject, protected OpenGLFunctions
{
  Q_OBJECT

public:

  friend class JTVis_ScenegraphTask;

  //! Creates graphic scene representation.
  JTVis_Scene (QObject* parent = 0);

  //! Frees resources which handled manually.
  virtual ~JTVis_Scene();

  //! Initializes scene.
  void Initialize();

  //! Updates scene.
  void Update (float theTime);

  //! Draws visible objects.
  void Render();

  //! Handles window resize.
  void Resize (int theWidth, int theHeight);

  //! Sets OpenGl context.
  void SetContext (QOpenGLContext* context) { myContext = context; }

  //! Returns OpenGl context.
  QOpenGLContext* Context() const { return myContext; }

  //! Remembers camera's start rotation.
  void StartRotation() { myStartRotation = myCamera->Rotation(); }

  //! Rotates camera.
  void Rotate (float theX, float theY) { myRotation = QVector2D (theX, theY); }

  //! Zoom camera.
  void Zoom (float theDelta) { myZoom =+ theDelta; } 

  //! Pan camera.
  void Pan (float theX, float theY) { myPanning += QVector2D (theX, theY); }

  //! Sets geometry source object for scene.
  void SetGeometrySource (JTData_GeometrySourcePtr theGeomSrc) { myGeometrySource = theGeomSrc; }

  //! Returns geometry source object of scene.
  JTData_GeometrySourcePtr GeometrySource() { return myGeometrySource; }

  //! Sets mouse position.
  void SetMousePosition (QPoint thePoint) { myMousePos = thePoint; }

  //! Fits scene contents on the screen.
  void FitAll (const JTVis_FitMode theFitMode = fmFitVisible,
               const JTVis_StandardView theDesiredView = svDontChange);

  //! Fits scene contents in z-range.
  void FitZ();

  //! Sets camera projection mode.
  void SetCameraMode (JTVis_CameraMode theMode) { myCamera->SetCameraMode (theMode); FitZ(); }

  //! Sets one of standard views for camera.
  void SetCameraStandardView (JTVis_StandardView theView);

  //! Returns reference to settings structure.
  JTVis_Settings& ChangeSettings() { return mySettings; }

  //! Returns reference to settings structure.
  const JTVis_Settings& Settings() const { return mySettings; }

  //! Returns current rendering statistics.
  const JTVis_Stats& RenderStats() const { return myStats; }

  //! Tells scene to select node and its subtree if present.
  void SelectNode (const JTData_NodePtr& theNode, bool isMultipleSelection = false);

  //! Tries to select mesh under current mouse position.
  void SelectMesh (bool isMultipleSelection = false);

  //! Clears selection.
  void ClearSelection();

  //! Sets external callback to request animation mode.
  void SetAnimationModeCallback (const JTVis_AnimationModeCallbackPtr& theCallback) { myAnimationCallback = theCallback; }

  //! Takes screen shot from current view and saves image in theScreenshotPath.
  void MakeScreenshot (QString theScreenshotPath)
  {
    myScreenshotPath = theScreenshotPath;
    isPerformingScreenshot = true;
  }

  //! Sets max samples value.
  void SetMaxSamples (int theSamples) { myMaxSamples = theSamples; }

  void DebugKeyHandler (int theKey);

public slots:

  //! Updates view.
  void ForceUpdate() { emit RequestViewUpdate(); }

signals:

  //! Request scene redraw.
  void RequestViewUpdate();

  //! Request animation mode.
  void RequestAnimationMode (bool isEnabled);

  //! Request clear selection.
  void RequestClearSelection();

  //! Request selection of specified node.
  void RequestSelection (JTData_Node* theNode);

  //! Indicates then scene loaded and ready to visualize.
  void LoadingComplete();

private:

  //! Traverses scenegraph applying given task.
  void WalkScenegraph (JTVis_ScenegraphTaskPtr theTask);

  //! Traverses scenegraph updating LOD states of nodes.
  void UpdateLods();

  //! Loads shaders.
  void PrepareShaders();

  //! Retrieves part nodes from scenegraph. Calls PreparePartNode recursively.
  void PreparePartNodes();

  //! Handles camera control operations.
  void HandleCamera (float theDeltaTime);

  //! Requests geometry extraction from data source.
  //! If data is ready, loads triangulation to GPU.
  void RequestGeometryForNode (JTVis_PartNode* theNode);

  //! Recreates FBOs for current viewport.
  void ResetFbos();

  //! Marks PartNode this id "theNodeId" and its alternate LODs as selected. 
  void PerformSelection (int theNodeId, bool isMultipleSelection);

  //! Draws text into OpenGL texture.
  void PrepareTextTexture (const int theTextureId, 
                           const QSize&   theSize,
                           const QString& theText,
                           const QColor&  theColor,
                           const int      theFontSize);

  //! Draws trihedron in orthographic projection.
  void DrawTrihedron();

private:

  bool myIsInitialized; //!< Indicates when scene already initialized.

  QOpenGLContext* myContext; //!< OpenGL context.

  JTVis_TargetedCameraPtr myCamera; //!< Current camera object.

  float myTime; //!< Time helper object.

  QOpenGLShaderProgram* myShaderProgram;          //!< Main shader program, implements Phong shading.
  QOpenGLShaderProgram* myLinesShaderProgram;     //!< Shader program for drawing lines.
  QOpenGLShaderProgram* myTexQuadShaderProgram;   //!< Shader program for drawing textured quad.
  QOpenGLShaderProgram* myBgShaderProgram;        //!< Background shader program.
  QOpenGLShaderProgram* myIdShaderProgram;        //!< Shader program that draws object ids into texture.
                                                  //!< Used for selection.
  QOpenGLShaderProgram* myTrihedronShaderProgram; //!< Shader program for drawing trihedron.

  std::vector<JTVis_PartNodePtr> myPartNodes;                //!< Main storage for PartNodes.
  QMap<JTData_MeshNode*, JTVis_PartNodePtr> myMeshToPartMap; //!< Map to access PartNode with given MeshNode.

  QVector2D myRotation;              //!< Stored current rotation of camera.
  Eigen::AngleAxisf myStartRotation; //!< Stored start rotation of camera.

  float myZoom;         //!< Stored zoom of camera, to apply on next frame.
  QVector2D myPanning;  //!< Stored panning of camera, to apply on next frame.

  JTData_GeometrySourcePtr myGeometrySource; //!< Reference to geometry source object.

  BVH_Geometry<float, 4> myBvhGeometry; //!< Acceleration structure. Contains BVH-tree used for view culling.

  QVector<JTVis_GraphicObjectPtr> myHelperObjects; //! Helper objects container. For debug.

  QPoint myMousePos; //!< Current mouse position.
  QPoint myViewport; //!< Current viewport dimensions.

  long long int myCurrentState; //!< Current state value. Implemented as frame counter.

  JTCommon_AABB myGlobalBounds;  //!< Global bounds of scene in world coordinates.
  JTCommon_AABB myVisibleBounds; //!< Bounds of visible scene objects in world coordinates.

  JTVis_Stats myStats; //!< Statistical data of visualization process.

  QMap<JTData_MeshNodeSource*, JTVis_PartGeometryPtr> myInstancedMeshes; //!< Map to determine whenever PartGeometry may be instanced.

  unsigned int myUnloadCheckPeriod; //!< Frame count between unload checks.
  unsigned int myOldFrameCount;     //!< Nodes with state outdated for myOldFrameCount frames treated as old.

  std::set<JTVis_PartNode*> mySelectedParts; //!< Set of selected nodes.

  QOpenGLFramebufferObject* mySelectionFbo;  //!< OpenGL Frame buffer object.
  QOpenGLFramebufferObject* myScreenshotFbo; //!< OpenGL Frame buffer object.

  JTVis_QuadGeometryPtr myScreenQuad; //!< Geometry object for quad covering entire screen.

  bool isPerformingSelection;         //!< Indicates when mesh selection is queried.
  bool isPerformingMultipleSelection; //!< Multiple selection mode.

  float *mySelectionBuffer;   //!< Buffer for mapping id texture into CPU memory.
                              //!< Managed in Resize() method.

  JTVis_PartGeometryAggregator myPartAggregator; //!< Part geometry aggregator object.

  Standard_Integer mySmallPartTreshold; //!< Threshold to consider part as "small".
                                        //!< Used to determine possibility of myPartAggregator utilizing 
                                        //!< for given PartGeometry.

  JTVis_SelectionCallbackPtr mySelectionCallback; //!< External callback to handle selections.

  JTVis_ClearSelectionCallbackPtr myClearSelectionCallback; //!< External callback to handle selections.

  JTVis_NeedToUpdateCallbackPtr myUpdateCallback; //!< External callback to request scene redraw.

  JTVis_AnimationModeCallbackPtr myAnimationCallback; //!< External callback to request animation mode.

  JTVis_TrihedronGeometryPtr myTrihedron;     //!< Geometry for trihedron object.
  JTVis_QuadGeometryPtr myTrihedronLabelQuad; //!< Geometry object for trihedron text.

  QMap<int, unsigned int> myTextures; //!< OpenGL texture ids mapped to some scene enumerations.

  JTVis_Settings mySettings; //!< Visualization settings.

  JTVis_CameraTransition myCameraTransition; //!< Helper class for camera animation.

  QString myScreenshotPath;    //!< Current path for screenshot saving.
  bool isPerformingScreenshot; //!< Indicates viewer need to render into myScreenshotFbo.

  JTGui_HudRendererPtr myHudRenderer; //!< Helper class to render HUD.

  int myMaxSamples; //!< Allowed multisampling value for framebuffers.

  bool myFirstReady; //!< Indicates if all visible parts loaded for the first time.

  bool toResetFBOs; //!< Specifies whether we need to reset FBOs

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_Scene> JTVis_ScenePtr;

#endif // JTVIS_SCENE_H
