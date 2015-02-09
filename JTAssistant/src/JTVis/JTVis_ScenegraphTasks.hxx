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

#ifndef JTVIS_SCENEGRAPH_TASKS_H
#define JTVIS_SCENEGRAPH_TASKS_H

#include "JTVis_Scene.hxx"

#pragma warning (push, 0)
#include <QStack>
#pragma warning (pop)

//! Defines callback used for execution of tasks on scenegraph.
class JTVis_ScenegraphTask
{
public:

  JTVis_ScenegraphTask (JTVis_Scene* theScene, const JTData_NodePtr& theNode)
    : myScene (theScene), myNode (theNode)
  {}

  virtual ~JTVis_ScenegraphTask() {};
  virtual void Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack) = 0;
  virtual const JTVis_ScenegraphTaskPtr Copy (const JTData_NodePtr& theNode) = 0;

  virtual void Traverse (QStack<JTVis_ScenegraphTaskPtr>& theStack)
  {
    if (!myNode.dynamicCast<JTData_InstanceNode>().isNull())
    {
      JTData_InstanceNodePtr anInstance = myNode.dynamicCast<JTData_InstanceNode>();

      theStack.push (this->Copy (anInstance->Reference));
    }
    else if (!myNode.dynamicCast<JTData_GroupNode>().isNull())
    {
      JTData_GroupNodePtr aGroup = myNode.dynamicCast<JTData_GroupNode>();

      for (size_t anIdx = 0; anIdx < aGroup->Children.size(); ++anIdx)
      {
        theStack.push (this->Copy (aGroup->Children.at (anIdx)));
      }
    }
  }

protected:

  std::vector<JTVis_PartNodePtr>& ScenePartNodes() { return myScene->myPartNodes; }
  QMap<JTData_MeshNode*, JTVis_PartNodePtr>& SceneMeshToPartMap() { return myScene->myMeshToPartMap; }
  JTData_GeometrySourcePtr& SceneGeometrySource() { return myScene->myGeometrySource; }
  BVH_Geometry<float, 4>& SceneBvhGeometry() { return myScene->myBvhGeometry; }
  long long int& SceneCurrentState() { return myScene->myCurrentState; }
  unsigned int& SceneOldFrameCount() { return myScene->myOldFrameCount; }
  JTVis_TargetedCameraPtr& SceneCamera() { return myScene->myCamera; }
  std::set<JTVis_PartNode*>& SceneSelectedParts() { return myScene->mySelectedParts; }
  QOpenGLShaderProgram* SceneLinesShaderProgram() { return myScene->myLinesShaderProgram; }

  JTVis_Scene* myScene;
  JTData_NodePtr myNode;
};

//! Combines transform and material attributes while traversing scenegraph.
class JTVis_PrepareNodeTask: public JTVis_ScenegraphTask
{
public:

  JTVis_PrepareNodeTask (JTVis_Scene* theScene, const JTData_NodePtr& theNode)
    : JTVis_ScenegraphTask (theScene, theNode),
      myTransform (Eigen::Matrix4f::Identity()),
      myLastRangeNode (NULL)
  {}

  void Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack);

  static JTVis_PrepareNodeTask* Make (JTVis_Scene*           theScene,
                                      JTData_NodePtr         theNode,
                                      JTVis_PrepareNodeTask* theOldTask)
  {
    JTVis_PrepareNodeTask* aNewTask = new JTVis_PrepareNodeTask (theScene, theNode);

    aNewTask->myTransform = theOldTask->myTransform;
    aNewTask->myMaterial  = theOldTask->myMaterial;
    aNewTask->myLastRangeNode = theOldTask->myLastRangeNode;

    return aNewTask;
  }

  const JTVis_ScenegraphTaskPtr Copy (const JTData_NodePtr& theNode)
  {
    return JTVis_ScenegraphTaskPtr (Make (myScene, theNode, this));
  }

private:

  Eigen::Matrix4f myTransform;
  JTData_MaterialAttribute myMaterial;
  JTData_RangeLODNode* myLastRangeNode;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

//! Generates center points for range LOD nodes.
class JTVis_GenerateCentersTask: public JTVis_ScenegraphTask
{
public:

  static JTCommon_AABB Box;
  static JTCommon_AABB GlobalBox;

public:

  JTVis_GenerateCentersTask (JTVis_Scene* theScene, const JTData_NodePtr& theNode)
    : JTVis_ScenegraphTask (theScene, theNode), isGatheringBox (false) {}

  void Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack);

  void Traverse (QStack<JTVis_ScenegraphTaskPtr>& theStack);

  const JTVis_ScenegraphTaskPtr Copy (const JTData_NodePtr& theNode)
  {
    return JTVis_ScenegraphTaskPtr (new JTVis_GenerateCentersTask (myScene, theNode));
  }

private:

  bool isGatheringBox;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

//! Unloads nodes completely not visible for OldFrameCount frames.
class JTVis_UnloadOldTask: public JTVis_ScenegraphTask
{
public:

  JTVis_UnloadOldTask (JTVis_Scene* theScene, const JTData_NodePtr& theNode)
    : JTVis_ScenegraphTask (theScene, theNode) {}

  void Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack);

  const JTVis_ScenegraphTaskPtr Copy (const JTData_NodePtr& theNode)
  {
    return JTVis_ScenegraphTaskPtr (new JTVis_UnloadOldTask (myScene, theNode));
  }

};

//! Selects scenegraph subtree.
class JTVis_SelectTask: public JTVis_ScenegraphTask
{
public:

  JTVis_SelectTask (JTVis_Scene* theScene, const JTData_NodePtr& theNode)
    : JTVis_ScenegraphTask (theScene, theNode) {}

  void Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack);

  const JTVis_ScenegraphTaskPtr Copy (const JTData_NodePtr& theNode)
  {
    return JTVis_ScenegraphTaskPtr (new JTVis_SelectTask (myScene, theNode));
  }

};

#endif // JTVIS_SCENEGRAPH_TASKS_H
