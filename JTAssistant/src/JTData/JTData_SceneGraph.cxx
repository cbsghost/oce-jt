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

#pragma warning (push, 0)
#include <QDir>
#include <QStack>
#include <QFileInfo>
#pragma warning (pop)

#include <JTData_Node.hxx>
#include <JtNode_Part.hxx>
#include <JtNode_RangeLOD.hxx>
#include <JtNode_Instance.hxx>
#include <JtNode_Partition.hxx>
#include <JtNode_Shape_TriStripSet.hxx>
#include <JtElement_ShapeLOD_TriStripSet.hxx>
#include <JtAttribute_GeometricTransform.hxx>
#include <JtAttribute_Material.hxx>

#include "JTData_SceneGraph.hxx"

#include <limits>
#include <cmath>

using namespace Eigen;

// =======================================================================
// function : JTData_SceneGraph
// purpose  :
// =======================================================================
JTData_SceneGraph::JTData_SceneGraph()
  : myLoadingThread (myLoadingQueue)
{
}

// =======================================================================
// function : ~JTData_SceneGraph
// purpose  :
// =======================================================================
JTData_SceneGraph::~JTData_SceneGraph()
{
  myLoadingThread.quit();
  myLoadingThread.wait();
}

// =======================================================================
// function : FillGroup
// purpose  :
// =======================================================================
void JTData_SceneGraph::FillGroup (const JTData_GroupNodePtr& theGroupNode,
  const Handle(JtNode_Group)& theGroupRecord, const QString& thePrefix)
{
  if (theGroupRecord->Children().IsEmpty())
    return;

  for (Standard_Integer aChildIdx = 0; aChildIdx < (Standard_Integer)theGroupRecord->Children().Count(); ++aChildIdx)
  {
    Handle(JtNode_Base) aChildRecord = Handle(JtNode_Base)::DownCast (theGroupRecord->Children()[aChildIdx]);

    if (aChildRecord.IsNull())
    {
      continue;
    }

    JTData_NodePtr aChildNode = PushNode (aChildRecord, thePrefix);

    Q_ASSERT_X (!aChildNode.isNull(),
      "PushNode", "Error! Failed to extract node from LSG segment");

    if (!aChildNode.isNull())
    {
      theGroupNode->Children.push_back (aChildNode);
    }
  }
}

static Handle(Standard_Type) TypeOf_JtNode_Partition        = STANDARD_TYPE(JtNode_Partition);
static Handle(Standard_Type) TypeOf_JtNode_Part             = STANDARD_TYPE(JtNode_Part);
static Handle(Standard_Type) TypeOf_JtNode_RangeLOD         = STANDARD_TYPE(JtNode_RangeLOD);
static Handle(Standard_Type) TypeOf_JtNode_LOD              = STANDARD_TYPE(JtNode_LOD);
static Handle(Standard_Type) TypeOf_JtNode_Group            = STANDARD_TYPE(JtNode_Group);
static Handle(Standard_Type) TypeOf_JtNode_Instance         = STANDARD_TYPE(JtNode_Instance);
static Handle(Standard_Type) TypeOf_JtNode_Shape_Vertex      = STANDARD_TYPE(JtNode_Shape_Vertex);
static Handle(Standard_Type) TypeOf_JtNode_Shape_TriStripSet = STANDARD_TYPE(JtNode_Shape_TriStripSet);
static Handle(Standard_Type) TypeOf_JtAttribute_Material    = STANDARD_TYPE(JtAttribute_Material);
static Handle(Standard_Type) TypeOf_JtAttribute_GeometricTransform = STANDARD_TYPE(JtAttribute_GeometricTransform);

// =======================================================================
// function : setNodeName
// purpose  : Auxiliary function to set the correct Node name
// =======================================================================
static void setNodeName (const Handle(JtNode_Base)& theNodeRecord, 
                         const JTData_NodePtr&      theNode)
{
  QString aName (QString::fromUtf16 (reinterpret_cast<const ushort*> (theNodeRecord->Name().ToExtString())));

  // Remove unnecessary suffixes from name
  if (aName.contains (".asm"))
  {
    aName.remove (".asm");
  }
  else if (aName.contains (".part"))
  {
    aName.remove (".part");
  }

  theNode->SetName (aName);
}

// =======================================================================
// function : PushNode
// purpose  :
// =======================================================================
JTData_NodePtr JTData_SceneGraph::PushNode (const Handle(JtNode_Base)& theNodeRecord, const QString& thePrefix)
{
  JTData_NodePtr aResult;

  // Extract LSG node

  if (theNodeRecord->IsKind (TypeOf_JtNode_Partition))
  {
    Handle(JtNode_Partition) aPartitionRecord =
      Handle(JtNode_Partition)::DownCast (theNodeRecord);

    QFileInfo anInfo (QString::fromUtf16 (reinterpret_cast<const ushort*> (
      aPartitionRecord->FileName().ToExtString()), aPartitionRecord->FileName().Length()));

    if (aPartitionRecord->FileName().Length() > 0)
    {
      aResult = JTData_PartitionNodePtr (new JTData_PartitionNode (
        QDir::cleanPath (QDir (thePrefix).absoluteFilePath (anInfo.filePath()))));
    }

    Q_ASSERT_X (!aResult.isNull(),
      "PushNode", "Error! Failed to create partition node");

    FillGroup (aResult.dynamicCast<JTData_GroupNode>(), aPartitionRecord, thePrefix);
  }
  else if (theNodeRecord->IsKind (TypeOf_JtNode_Part))
  {
    aResult = JTData_PartNodePtr (new JTData_PartNode);

    FillGroup (aResult.dynamicCast<JTData_GroupNode>(),
      Handle(JtNode_Group)::DownCast (theNodeRecord), thePrefix);
  }
  else if (theNodeRecord->IsKind (TypeOf_JtNode_RangeLOD))
  {
    JTData_RangeLODNodePtr aRangeLOD = JTData_RangeLODNodePtr (new JTData_RangeLODNode);

    Handle(JtNode_RangeLOD) aRangeLODRecord =
      Handle(JtNode_RangeLOD)::DownCast (theNodeRecord);

    FillGroup (aRangeLOD, aRangeLODRecord, thePrefix);

    aRangeLOD->Center() = Eigen::Vector4f (
      static_cast<Standard_ShortReal> (aRangeLODRecord->Center().X),
      static_cast<Standard_ShortReal> (aRangeLODRecord->Center().Y),
      static_cast<Standard_ShortReal> (aRangeLODRecord->Center().Z),
      1.0);

    if (!aRangeLODRecord->RangeLimits().IsEmpty())
    {
      for (Standard_Integer anIdx = 1; anIdx <= aRangeLODRecord->RangeLimits().Count(); ++anIdx)
      {
        aRangeLOD->Ranges().push_back (aRangeLODRecord->RangeLimits()[anIdx]);
      }
    }
    else
    {
      aRangeLOD->Ranges().push_back (std::numeric_limits<Standard_ShortReal>::max());

      for (Standard_Integer anIdx = 1; anIdx < (Standard_Integer)aRangeLODRecord->Children().Count(); ++anIdx)
      {
        aRangeLOD->Ranges().push_back (0.0);
      }
    }

    aResult = aRangeLOD;
  }
  else if (theNodeRecord->IsKind (TypeOf_JtNode_LOD))
  {
    aResult = JTData_LODNodePtr (new JTData_LODNode);

    FillGroup (aResult.dynamicCast<JTData_GroupNode>(),
      Handle(JtNode_Group)::DownCast (theNodeRecord), thePrefix);
  }
  else if (theNodeRecord->IsKind (TypeOf_JtNode_Group))
  {
    aResult = JTData_GroupNodePtr (new JTData_GroupNode);

    FillGroup (aResult.dynamicCast<JTData_GroupNode>(),
      Handle(JtNode_Group)::DownCast (theNodeRecord), thePrefix);
  }
  else if (theNodeRecord->IsKind (TypeOf_JtNode_Instance))
  {
    Handle(JtNode_Instance) anInstance = Handle(JtNode_Instance)::DownCast (theNodeRecord);

    // Note: To support JT Viewer operations (such as object hiding) it is convenient
    // to eliminate instance nodes from the scene graph. In result, using of sub-tree
    // references becomes impossible. But the actual geometric data is not duplicated
    // by decomposing the node on the description part and data source part.

    Handle(JtNode_Base) aNode = Handle(JtNode_Base)::DownCast (anInstance->Object());

    Q_ASSERT_X (!aNode.IsNull(),
      "PushNode", "Error! Invalid object in LSG segment");

    aResult = PushNode (aNode, thePrefix);
  }
  else if (theNodeRecord->IsKind (TypeOf_JtNode_Shape_Vertex))
  {
    Handle(JtNode_Shape_Vertex) aShapeRecord = Handle(JtNode_Shape_Vertex)::DownCast (theNodeRecord);

    if (aShapeRecord->IsKind (TypeOf_JtNode_Shape_TriStripSet))
    {
      Handle(JtNode_Shape_TriStripSet) aMeshRecord = Handle(JtNode_Shape_TriStripSet)::DownCast (aShapeRecord);

      JTCommon_AABB aBox (
        Eigen::Vector4f (aMeshRecord->Bounds().MinCorner.X,
                         aMeshRecord->Bounds().MinCorner.Y,
                         aMeshRecord->Bounds().MinCorner.Z,
                         1.f),
        Eigen::Vector4f (aMeshRecord->Bounds().MaxCorner.X,
                         aMeshRecord->Bounds().MaxCorner.Y,
                         aMeshRecord->Bounds().MaxCorner.Z,
                         1.f));

      if (!mySources.Contains (aMeshRecord))
      {
        JTData_MeshNodeSourcePtr aMeshSource (
          new JTData_MeshNodeSource (myLoadingQueue, aMeshRecord));

        // Add to new mesh source to the map (can be reused)
        mySources.Add (aMeshRecord, aMeshSource);
      }

      aResult = JTData_MeshNodePtr (new JTData_MeshNode (
        aBox, mySources.FindFromKey (aMeshRecord)));
    }
  }

  Q_ASSERT_X (!aResult.isNull(),
    "PushNode", "Error! Unknown type of LSG node");

  setNodeName (theNodeRecord, aResult);

  // Extract attributes

  if (theNodeRecord->Attributes().IsEmpty())
    return aResult;

  for (Standard_Integer anIdx = 0; anIdx < (Standard_Integer)theNodeRecord->Attributes().Count(); ++anIdx)
  {
    const Handle(JtData_Object)& anObject = theNodeRecord->Attributes()[anIdx];

    if (anObject.IsNull())
      continue;

    Handle(JtAttribute_Base) anAttrib = Handle(JtAttribute_Base)::DownCast (anObject);

    Q_ASSERT_X (!anAttrib.IsNull(),
      "PushNode", "Error! Invalid node attribute");

    if (anAttrib->IsKind (TypeOf_JtAttribute_GeometricTransform))
    {
      Handle(JtAttribute_GeometricTransform) aTransform =
        Handle(JtAttribute_GeometricTransform)::DownCast (anAttrib);

      Eigen::Matrix4f aMatrix;

      for (Standard_Integer aX = 0; aX < 4; ++aX)
      {
        for (Standard_Integer aY = 0; aY < 4; ++aY)
        {
          aMatrix (aX, aY) = static_cast<Standard_ShortReal> (aTransform->GetTrsf()[aY * 4 + aX]);
        }
      }

      if (aMatrix (3, 3) == 0.f)
      {
        aMatrix (3, 3) = 1.f; // fix problem with homogeneous coordinates
      }

      aResult->Attributes.push_back (JTData_TransformAttributePtr (new JTData_TransformAttribute (aMatrix)));
    }
    else if (anAttrib->IsKind (TypeOf_JtAttribute_Material))
    {
      Handle(JtAttribute_Material) aMaterial =
        Handle(JtAttribute_Material)::DownCast (anAttrib);

      Eigen::Vector4f aAmbientColor (
        static_cast<Standard_ShortReal> (aMaterial->AmbientColor()[0]),
        static_cast<Standard_ShortReal> (aMaterial->AmbientColor()[1]),
        static_cast<Standard_ShortReal> (aMaterial->AmbientColor()[2]),
        static_cast<Standard_ShortReal> (aMaterial->AmbientColor()[3]));

      Eigen::Vector4f aDiffuseColor (
        static_cast<Standard_ShortReal> (aMaterial->DiffuseColor()[0]),
        static_cast<Standard_ShortReal> (aMaterial->DiffuseColor()[1]),
        static_cast<Standard_ShortReal> (aMaterial->DiffuseColor()[2]),
        static_cast<Standard_ShortReal> (aMaterial->DiffuseColor()[3]));

      Eigen::Vector4f aSpecularColor (
        static_cast<Standard_ShortReal> (aMaterial->SpecularColor()[0]),
        static_cast<Standard_ShortReal> (aMaterial->SpecularColor()[1]),
        static_cast<Standard_ShortReal> (aMaterial->SpecularColor()[2]),
        static_cast<Standard_ShortReal> (aMaterial->SpecularColor()[3]));

      Eigen::Vector4f aEmissionColor (
        static_cast<Standard_ShortReal> (aMaterial->EmissionColor()[0]),
        static_cast<Standard_ShortReal> (aMaterial->EmissionColor()[1]),
        static_cast<Standard_ShortReal> (aMaterial->EmissionColor()[2]),
        static_cast<Standard_ShortReal> (aMaterial->EmissionColor()[3]));

      Standard_ShortReal aShininess =
        static_cast<Standard_ShortReal> (aMaterial->Shininess());

      JTData_MaterialAttributePtr aMaterialAttrib (new JTData_MaterialAttribute (
        aAmbientColor, aDiffuseColor, aSpecularColor, aEmissionColor, aShininess));

      aResult->Attributes.push_back (aMaterialAttrib);
    }
  }

  return aResult;
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
Standard_Boolean JTData_SceneGraph::Init (const Handle(JtNode_Partition)& thePartitionRecord, JTData_PartitionNode* thePartition)
{
  if (thePartitionRecord.IsNull())
  {
    return Standard_False;
  }

  JTData_PartitionNodePtr aRoot = PushNode (thePartitionRecord,
    QFileInfo (thePartition->FileName()).path()).dynamicCast<JTData_PartitionNode>();

  if (aRoot.isNull())
  {
    return Standard_False;
  }

  for (size_t anIdx = 0; anIdx < aRoot->Children.size(); ++anIdx)
  {
    thePartition->Children.push_back (aRoot->Children.at (anIdx));
  }

  return Standard_True;
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
Standard_Boolean JTData_SceneGraph::Init (const Handle(JtData_Model)& theModel, const QString& theFileName)
{
  if (!myRoot.isNull())
    myRoot.reset();

  Handle(JtNode_Partition) aNode = theModel->Init();

  if (!aNode.IsNull())
  {
    myRoot = PushNode (aNode, QFileInfo (theFileName).path());
  }

  Q_ASSERT_X (!myRoot.isNull(),
    "Init", "Error! failed to create root node for LSG segment");

  return Standard_True;
}

// =======================================================================
// function : GenerateRanges
// purpose  :
// =======================================================================
void GenerateRanges (const JTData_RangeLODNodePtr& theRangeLOD,
  const JTCommon_AABB& theGlobalBox, const Standard_ShortReal theScale)
{
  const Standard_ShortReal aLength = theGlobalBox.Size().norm();

  theRangeLOD->Ranges().clear();

  for (size_t anIdx = 0; anIdx < theRangeLOD->Children.size() - 1; ++anIdx)
  {
    theRangeLOD->Ranges().push_back (aLength / (theRangeLOD->Children.size() - anIdx) * theScale);
  }
}

// =======================================================================
// function : GenerateRanges
// purpose  :
// =======================================================================
void JTData_SceneGraph::GenerateRanges (
  const JTCommon_AABB& theGlobalBox, const Standard_ShortReal theScale)
{
  QStack<JTData_NodePtr> aStack;

  if (myRoot.isNull())
  {
    return;
  }

  aStack.push (myRoot);

  for (;;)
  {
    if (aStack.isEmpty())
    {
      break;
    }

    JTData_NodePtr aNode = aStack.pop();

    if (!aNode.dynamicCast<JTData_RangeLODNode>().isNull())
    {
      JTData_RangeLODNodePtr aRangeLOD = aNode.dynamicCast<JTData_RangeLODNode>();

      ::GenerateRanges (aRangeLOD, theGlobalBox, theScale);
    }
    else if (!aNode.dynamicCast<JTData_InstanceNode>().isNull())
    {
      JTData_InstanceNodePtr anInstance = aNode.dynamicCast<JTData_InstanceNode>();

      aStack.push (anInstance->Reference);
    }

    if (!aNode.dynamicCast<JTData_GroupNode>().isNull())
    {
      JTData_GroupNodePtr aGroup = aNode.dynamicCast<JTData_GroupNode>();

      for (size_t anIdx = 0; anIdx < aGroup->Children.size(); ++anIdx)
      {
        aStack.push (aGroup->Children.at (anIdx));
      }
    }
  }
}

// =======================================================================
// function : Clear
// purpose  :
// =======================================================================
void JTData_SceneGraph::Clear()
{
  myRoot.reset();
}

// =======================================================================
// function : EstimateMemoryUsed
// purpose  :
// =======================================================================
Standard_Integer JTData_SceneGraph::EstimateMemoryUsed() const
{
  JTData_InstanceMap aMap;

  if (!myRoot.isNull())
  {
    return sizeof (JTData_SceneGraph) + myRoot->EstimateMemoryUsed (aMap);
  }

  return sizeof (JTData_SceneGraph);
}
