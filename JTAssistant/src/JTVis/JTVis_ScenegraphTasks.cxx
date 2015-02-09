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

#include "JTVis_ScenegraphTasks.hxx"

#include <Eigen/Core>

#include <typeinfo>

using namespace Eigen;

// =======================================================================
// function : transformBox
// purpose  :
// =======================================================================
static JTCommon_AABB transformBox (const JTCommon_AABB& theBox, const Matrix4f& theMatrix)
{
  Vector4f aSize = theBox.Size();

  JTCommon_AABB aBox;
  for (int aX = 0; aX <= 1; ++aX)
  {
    for (int aY = 0; aY <= 1; ++aY)
    {
      for (int aZ = 0; aZ <= 1; ++aZ)
      {
        Vector4f aCorner = theBox.CornerMin() +
          Vector4f (aSize.x(), 0.f, 0.f, 0.f) * static_cast<float> (aX) +
          Vector4f (0.f, aSize.y(), 0.f, 0.f) * static_cast<float> (aY) +
          Vector4f (0.f, 0.f, aSize.z(), 0.f) * static_cast<float> (aZ);

        aBox.Add (theMatrix * aCorner);
      }
    }
  }

  aBox.CornerMin().w() = 1.f;
  aBox.CornerMax().w() = 1.f;

  return aBox;
}

// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
void JTVis_PrepareNodeTask::Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack)
{
  Q_UNUSED (theStack)

  int aTransIndex = -1;
  int aMaterIndex = -1;

  for (int anIdx = 0; anIdx < static_cast<int>(myNode->Attributes.size()); ++anIdx)
  {
    if (!myNode->Attributes.at (anIdx).dynamicCast<JTData_TransformAttribute>().isNull())
      aTransIndex = anIdx;
    else
      if (!myNode->Attributes.at (anIdx).dynamicCast<JTData_MaterialAttribute>().isNull())
        aMaterIndex = anIdx;
  }

  if (aTransIndex >= 0)
  {
    myTransform *= myNode->Attributes.at (
      aTransIndex).dynamicCast<JTData_TransformAttribute>()->Transform();
  }

  if (aMaterIndex >= 0)
  {
    myMaterial = *myNode->Attributes.at (
      aMaterIndex).dynamicCast<JTData_MaterialAttribute>();
  }

  if (typeid (*myNode) == typeid (JTData_RangeLODNode))
  {
    myLastRangeNode = static_cast<JTData_RangeLODNode*> (myNode.data());
  }  
  else if (!myNode.dynamicCast<JTData_MeshNode>().isNull()) // Extract scene graph leaf nodes
  {
    JTData_MeshNodePtr aMesh = myNode.staticCast<JTData_MeshNode>();

    const JTCommon_AABB& aBounds = aMesh->UntransformedBox();

    JTVis_PartNodePtr aNewPartNode = JTVis_PartNodePtr (new JTVis_PartNode (aMesh.data()));
    aNewPartNode->RangeNode = myLastRangeNode;

    SceneMeshToPartMap().insert (aMesh.data(), aNewPartNode);

    aNewPartNode->SetTransform (myTransform);
    aNewPartNode->SetMaterial (myMaterial);
    aNewPartNode->Bounds = transformBox (aBounds, aNewPartNode->Transform());

    aNewPartNode->PartNodeId = static_cast<int>(ScenePartNodes().size());

    aNewPartNode->BoxGeometry.reset (new JTVis_AABBGeometry (aNewPartNode->Bounds));

    aNewPartNode->BoxGeometry->Color = QVector3D (
      aNewPartNode->DiffuseColor()[0],
      aNewPartNode->DiffuseColor()[1],
      aNewPartNode->DiffuseColor()[2]);

    aNewPartNode->BoxGeometry->InitializeGeometry (SceneLinesShaderProgram());


    ScenePartNodes().push_back (aNewPartNode);

    NCollection_Handle<BVH_Object<float, 4> > anObject (new JTVis_PartBvhObject (aNewPartNode.data(), aNewPartNode->Bounds));

    SceneBvhGeometry().Objects().Append (anObject);
  }
}

JTCommon_AABB JTVis_GenerateCentersTask::Box;
JTCommon_AABB JTVis_GenerateCentersTask::GlobalBox;

// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
void JTVis_GenerateCentersTask::Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack)
{
  if (typeid (*myNode) == typeid (JTData_RangeLODNode))
  {
    JTData_RangeLODNode* aLodRangeNode = static_cast<JTData_RangeLODNode*> (myNode.data());

    if (!isGatheringBox)
    {
      Box = JTCommon_AABB();
      JTVis_ScenegraphTaskPtr aNewTask = this->Copy (myNode);
      aNewTask.dynamicCast<JTVis_GenerateCentersTask>()->isGatheringBox = true;
      theStack.push (aNewTask);
    }
    else
    {
      aLodRangeNode->Center() = Box.Center();
      aLodRangeNode->Box = Box;
      GlobalBox.Combine (Box);
    }
  }
  else if (typeid (*myNode) == typeid (JTData_MeshNode))
  {
    JTData_MeshNode* aMesh = static_cast<JTData_MeshNode*> (myNode.data());
    JTVis_PartNodePtr aPartNode = SceneMeshToPartMap().value (aMesh);

    if (aPartNode != NULL)
    {
      Box.Combine (aPartNode->Bounds);
    }
  }
}

// =======================================================================
// function : Traverse
// purpose  :
// =======================================================================
void JTVis_GenerateCentersTask::Traverse (QStack<JTVis_ScenegraphTaskPtr>& theStack)
{
  if (isGatheringBox)
    return;

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

// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
void JTVis_UnloadOldTask::Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack)
{
  Q_UNUSED (theStack)

  if (!myNode.dynamicCast<JTData_MeshNode>().isNull())
  {
    JTData_MeshNodePtr aMesh = myNode.dynamicCast<JTData_MeshNode>();
    JTVis_PartNodePtr aPartNode = SceneMeshToPartMap().value (aMesh.data());

    if (aPartNode != NULL)
    {
      long long int aStateDelta = qMax (SceneCurrentState() - aMesh->State(),
                                        SceneCurrentState() - aPartNode->State());

      if (aStateDelta > SceneOldFrameCount() && aPartNode->IsReady())
      {
        if (!aPartNode->Geometry()->UsesAggregator())
          aPartNode->Clear();
      }
    }
  }
}

// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
void JTVis_SelectTask::Perform (QStack<JTVis_ScenegraphTaskPtr>& theStack)
{
  Q_UNUSED (theStack)

  if (!myNode.dynamicCast<JTData_MeshNode>().isNull())
  {
    JTData_MeshNodePtr aMesh = myNode.dynamicCast<JTData_MeshNode>();
    JTVis_PartNodePtr aPartNode = SceneMeshToPartMap().value (aMesh.data());

    if (aPartNode != NULL)
    {
      SceneSelectedParts().insert (aPartNode.data());
    }
  }
}
