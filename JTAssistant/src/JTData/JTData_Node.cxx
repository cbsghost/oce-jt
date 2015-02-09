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

#include "JTData_Node.hxx"

// =======================================================================
// function : ~JTData_Node
// purpose  :
// =======================================================================
JTData_Node::~JTData_Node()
{
  //
}

// =======================================================================
// function : EstimateMemoryUsed
// purpose  :
// =======================================================================
Standard_Integer JTData_GroupNode::EstimateMemoryUsed (JTData_InstanceMap& theMap) const
{
  Standard_Integer aSize = sizeof (JTData_GroupNode);

  for (size_t anIdx = 0; anIdx < Children.size(); ++anIdx)
  {
    aSize += Children.at (anIdx)->EstimateMemoryUsed (theMap) + sizeof (JTData_NodePtr);
  }

  return aSize;
}

// =======================================================================
// function : EstimateMemoryUsed
// purpose  :
// =======================================================================
Standard_Integer JTData_InstanceNode::EstimateMemoryUsed (JTData_InstanceMap& /*theMap*/) const
{
  return sizeof (JTData_InstanceNode) + sizeof (JTData_NodePtr);
}

// =======================================================================
// function : JTData_PartitionNode
// purpose  :
// =======================================================================
JTData_PartitionNode::JTData_PartitionNode (const QString& theFileName)
  : JTData_GroupNode(),
    myFileName (theFileName)
{
  //
}

// =======================================================================
// function : EstimateMemoryUsed
// purpose  :
// =======================================================================
Standard_Integer JTData_PartitionNode::EstimateMemoryUsed (JTData_InstanceMap& theMap) const
{
  return JTData_GroupNode::EstimateMemoryUsed (theMap) + myFileName.toUtf8().length();
}

// =======================================================================
// function : ~JTData_ShapeNode
// purpose  :
// =======================================================================
JTData_ShapeNode::~JTData_ShapeNode()
{
  //
}

// =======================================================================
// function : EstimateMemoryUsed
// purpose  :
// =======================================================================
Standard_Integer JTData_ShapeNode::EstimateMemoryUsed (JTData_InstanceMap& /*theMap*/) const
{
  return sizeof (JTData_ShapeNode) + sizeof (JTCommon_AABB);
}

// =======================================================================
// function : JTData_MeshNode
// purpose  :
// =======================================================================
JTData_MeshNode::JTData_MeshNode (const JTCommon_AABB& theBox, const JTData_MeshNodeSourcePtr& theSource)
  : JTData_ShapeNode (theBox),
    mySource (theSource)
{
  //
}

// =======================================================================
// function : ~JTData_MeshNode
// purpose  :
// =======================================================================
JTData_MeshNode::~JTData_MeshNode()
{
  //
}

// =======================================================================
// function : EstimateMemoryUsed
// purpose  :
// =======================================================================
Standard_Integer JTData_MeshNode::EstimateMemoryUsed (JTData_InstanceMap& theMap) const
{
  Standard_Integer aSize = JTData_ShapeNode::EstimateMemoryUsed (theMap);

  if (mySource.isNull() || theMap.contains (mySource.data()))
    return aSize;

  JTCommon_TriangleDataPtr aData = mySource->Triangulation();

  if (!aData.isNull())
  {
    aSize += aData->Data->Vertices().Count() * 24 + aData->Data->Indices().Count() * 4;
  }

  theMap.insert (mySource.data());

  return aSize;
}

// =======================================================================
// function : JTData_MeshNodeSource
// purpose  :
// =======================================================================
JTData_MeshNodeSource::JTData_MeshNodeSource (JTData_LoadingQueue& theQueue, const Handle(JtNode_Shape_TriStripSet)& theShape)
  : myQueue (theQueue),
    myShape (theShape)
{
  myTriangleCount = 0;
}

// =======================================================================
// function : RequestTriangulation
// purpose  :
// =======================================================================
JTCommon_TriangleDataPtr JTData_MeshNodeSource::RequestTriangulation (const Standard_Integer theIndex, QObject* theFeedback)
{
  if (myData.isNull())
  {
    const JtData_Object::VectorOfLateLoads& aLateLoaded = myShape->LateLoads();

    if (aLateLoaded.IsEmpty())
    {
      return myData;
    }

    Handle(JtData_Object) anObject = aLateLoaded[theIndex]->DefferedObject();

    if (!anObject.IsNull())
    {
      Handle(JtElement_ShapeLOD_TriStripSet) aLOD =
        Handle(JtElement_ShapeLOD_TriStripSet)::DownCast (anObject);

      if (!aLOD.IsNull())
      {
        BuildTriangulation (aLOD);
      }

      aLateLoaded[theIndex]->Unload();
    }
    else
    {
      if (myQueue.Size() < 1000) // fix overflow of the loading queue
      {
        JTData_WorkItem anItem (aLateLoaded[theIndex], theFeedback);

        if (!myQueue.Enqueued (anItem))
        {
          myQueue.Enqueue (anItem);
        }
      }
    }
  }

  return myData;
}

// =======================================================================
// function : BuildTriangulation
// purpose  :
// =======================================================================
void JTData_MeshNodeSource::BuildTriangulation (const Handle(JtElement_ShapeLOD_TriStripSet)& theShapeLOD)
{
  if (theShapeLOD.IsNull())
  {
    return;
  }

  myData.reset (new JTCommon_TriangleData);

  myData->Data = theShapeLOD;

  myTriangleCount = theShapeLOD->Indices().Count() / 3;
}
