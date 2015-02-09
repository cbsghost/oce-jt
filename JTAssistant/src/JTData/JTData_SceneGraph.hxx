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

#ifndef JTData_SceneGraph_HeaderFile
#define JTData_SceneGraph_HeaderFile

#pragma warning (push, 0)
#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QTreeWidgetItem>
#pragma warning (pop)

#include <JTCommon_Utils.hxx>
#include <JTData_Node.hxx>
#include <JtData_Reader.hxx>
#include <JtNode_Base.hxx>
#include <JtNode_Group.hxx>
#include <JtNode_Partition.hxx>

#include <NCollection_IndexedDataMap.hxx>


//! Data map to store instanced mesh triangulation sources.
typedef NCollection_IndexedDataMap<Handle(JtData_Object), JTData_MeshNodeSourcePtr> JTData_MeshSources;

Q_DECLARE_METATYPE (JTData_NodePtr)

//! Scene graph (LSG) contains a collection of objects (elements) connected
//! through directed references to form a acyclic graph structure. The LSG
//! is the graphical description of the model and contains graphics shapes
//! and attributes representing the components.
class JTData_SceneGraph
{
public:

  //! Creates new empty scene graph.
  JTData_SceneGraph();

  //! Releases resources of scene graph.
  virtual ~JTData_SceneGraph();

public:

  //! Returns root node of the scene graph.
  JTData_NodePtr Tree() const
  {
    return myRoot;
  }

  //! Extracts scene graph from JT data model.
  Standard_Boolean Init (const Handle(JtData_Model)& theModel, const QString& theFileName);

  //! Extracts scene graph from JT data model.
  Standard_Boolean Init (const Handle(JtNode_Partition)& thePartitionRecord, JTData_PartitionNode* thePartition);

  //! Generates ranges for LODs based on scene bounding box.
  void GenerateRanges (const JTCommon_AABB& theGlobalBox, const Standard_ShortReal theScale = 1.f);

  //! Clears data of scene graph.
  void Clear();

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed() const;

  //! Returns reference to object mapping nodes to TreeWidget items.
  const QMap<JTData_Node*, QTreeWidgetItem*>& NodeToItemMap() { return myNodeToItemMap; }

protected:

  //! Creates new scene graph node from JT reader node.
  JTData_NodePtr PushNode (const Handle(JtNode_Base)& theNodeRecord, const QString& thePrefix);

  //! Fills children for specified group node of the scene graph.
  void FillGroup (const JTData_GroupNodePtr& theGroupNode,
    const Handle(JtNode_Group)& theGroupRecord, const QString& thePrefix);

protected:

  //! Root node of the scene graph.
  JTData_NodePtr myRoot;

  //! Data map to store indices of LSG nodes.
  JTData_MeshSources mySources;

  //! Command queue to manage data loading tasks.
  JTData_LoadingQueue myLoadingQueue;

  //! Separate thread for loading triangulation data.
  JTData_LoadingThread myLoadingThread;

  //! RangeLod and Mesh nodes mapped to tree items.
  QMap<JTData_Node*, QTreeWidgetItem*> myNodeToItemMap;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_SceneGraph> JTData_SceneGraphPtr;

#endif // JTData_SceneGraph_HeaderFile
