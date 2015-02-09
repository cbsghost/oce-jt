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

#ifndef JTData_Node_HeaderFile
#define JTData_Node_HeaderFile

#pragma warning (push, 0)
#include <QSet>
#include <QString>
#pragma warning (pop)

#include <JtNode_Shape_TriStripSet.hxx>
#include <JtElement_ShapeLOD_TriStripSet.hxx>

#include "JTData_Attribute.hxx"
#include "JTData_DataLoader.hxx"


//! Map to handle instanced geometry data.
typedef QSet<void*> JTData_InstanceMap;


//! Node elements in the LSG can be categorized as either an internal or
//! leaf node. The leaf nodes are typically used to represent physically
//! components of the model and contain some graphical representation or
//! geometry. The internal nodes define the hierarchical organization of
//! the leaf nodes, forming both spatial and logical model relationships.
class JTData_Node : public JTData_Element
{
public:

  //! Creates new abstract scene graph node.
  JTData_Node() : myIsVisible (Standard_True)
  {
    //
  }

  //! Releases resources of abstract scene graph node.
  virtual ~JTData_Node() = 0;

  //! Returns name of the node.
  const QString& Name() const
  {
    return myName;
  }

  //! Sets name of the node.
  void SetName (const QString& theName)
  {
    myName = theName;
  }

  //! Returns node visibility flag.
  inline Standard_Boolean IsVisible() const
  {
    return myIsVisible;
  }

  //! Sets node visibility flag.
  inline void SetVisible (const Standard_Boolean theIsVisisble)
  {
    myIsVisible = theIsVisisble;
  }

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed (JTData_InstanceMap& theMap) const = 0;

protected:

  //! Name of the node.
  QString myName;

  //! Node visibility flag.
  Standard_Boolean myIsVisible;

public:

  //! Array of node attributes.
  JTData_ArrayOfAttributes Attributes;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_Node> JTData_NodePtr;


//! Array of scene graph nodes.
typedef std::vector<JTData_NodePtr> JTData_NodeArray;


//! Group nodes contain an ordered list of references to other nodes, called
//! the children. Groups may contain zero or more children, the children may
//! be of any node type.
class JTData_GroupNode : public JTData_Node
{
public:

  //! Creates new empty group node.
  JTData_GroupNode() : JTData_Node()
  {
    //
  }

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed (JTData_InstanceMap& theMap) const;

public:

  //! Array of child nodes.
  JTData_NodeArray Children;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_GroupNode> JTData_GroupNodePtr;


//! Partition node represents an external JT file reference and provides a means
//! to partition a model into multiple physical files (separate JT file per part
//! in an assembly). When the referenced JT file is opened, the partition node’s
//! children are the children of the LSG root node for the underlying JT file.
class JTData_PartitionNode : public JTData_GroupNode
{
public:

  //! Creates new empty partition node.
  JTData_PartitionNode (const QString& theFileName);

  //! Returns path to corresponding JT file.
  const QString& FileName() const
  {
    return myFileName;
  }

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed (JTData_InstanceMap& theMap) const;

protected:

  //! Path to corresponding JT file.
  QString myFileName;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_PartitionNode> JTData_PartitionNodePtr;


//! Part node element represents a root node for a particular part within a LSG
//! structure. Every unique part represented within a LSG structure should have
//! a corresponding part node element.
class JTData_PartNode : public JTData_GroupNode
{
public:

  //! Creates new empty part node.
  JTData_PartNode() : JTData_GroupNode()
  {
    //
  }

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_PartNode> JTData_PartNodePtr;


//! Instance node contains a single reference to another node. Their purpose is
//! to allow sharing of LSG nodes and assignment of specific attributes for the
//! instanced node. In current implementation instance nodes are not used
class JTData_InstanceNode : public JTData_Node
{
public:

  //! Creates new instance node.
  JTData_InstanceNode() : JTData_Node()
  {
    //
  }

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed (JTData_InstanceMap& theMap) const;

public:

  //! Referenced LSG node.
  JTData_NodePtr Reference;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_InstanceNode> JTData_InstanceNodePtr;


//! LOD node holds a list of alternate representations. The list is
//! represented as the children of a base group node.
class JTData_LODNode : public JTData_GroupNode
{
public:

  //! Creates new LOD node.
  JTData_LODNode() : JTData_GroupNode()
  {
    //
  }

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_LODNode> JTData_LODNodePtr;


//! Range LOD nodes hold a list of alternate representations and the ranges
//! over which those representations are appropriate. Range limits indicate
//! the distance between a specified center point and the eye point, within
//! which the corresponding alternate representation is appropriate.
class JTData_RangeLODNode : public JTData_LODNode
{
public:

  //! Creates new range LOD node.
  JTData_RangeLODNode() : JTData_LODNode()
  {
    //
  }

  //! Returns NCS center point of the node.
  Eigen::Vector4f& Center()
  {
    return myCenter;
  }

  //! Returns NCS center point of the node.
  const Eigen::Vector4f& Center() const
  {
    return myCenter;
  }

  //! Returns WCS distances from the eye to select representation.
  std::vector<Standard_ShortReal>& Ranges()
  {
    return myRanges;
  }

  //! Returns WCS distances from the eye to select representation.
  const std::vector<Standard_ShortReal>& Ranges() const
  {
    return myRanges;
  }

  JTCommon_AABB Box;

protected:

  //! NCS center point upon which alternative representation defined.
  Eigen::Vector4f myCenter;

  //! Indicates the WCS distance from the eye to select specific representation.
  std::vector<Standard_ShortReal> myRanges;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_RangeLODNode> JTData_RangeLODNodePtr;


//! Shape node elements are leaf nodes within the LSG structure and contain
//! or reference the geometric shape definition data.
class JTData_ShapeNode : public JTData_Node
{
public:

  //! Creates new abstract shape node.
  JTData_ShapeNode (const JTCommon_AABB& theUntransformedBox)
    : JTData_Node(),
      myUntransformedBox (theUntransformedBox)
  {
    //
  }

  //! Releases resources of abstract shape node.
  ~JTData_ShapeNode() = 0;

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed (JTData_InstanceMap& theMap) const;

public:

  //! Returns LCS AABB for all geometry contained in the node.
  const JTCommon_AABB& UntransformedBox() const
  {
    return myUntransformedBox;
  }

protected:

  //! LCS AABB for all geometry contained in the node.
  JTCommon_AABB myUntransformedBox;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_ShapeNode> JTData_ShapeNodePtr;


//! Tool object to store mesh node geometry.
class JTData_MeshNodeSource
{
public:

  //! Creates new triangle mesh source.
  Standard_EXPORT JTData_MeshNodeSource (JTData_LoadingQueue& theQueue, const Handle(JtNode_Shape_TriStripSet)& theShape);

public:

  //! Requests triangulation data for the mesh.
  JTCommon_TriangleDataPtr RequestTriangulation (const Standard_Integer theIndex = 0, QObject* theFeedback = NULL);

  //! Returns triangulation data for the mesh.
  JTCommon_TriangleDataPtr Triangulation()
  {
    return myData;
  }

  //! Returns triangles number in the mesh.
  Standard_Integer TriangleCount() const
  {
    return myTriangleCount;
  }

private:

  //! Reference to loading queue.
  JTData_LoadingQueue& myQueue;

  //! Triangulation data for the mesh node.
  JTCommon_TriangleDataPtr myData;

  //! Vertex shape element to read from JT file.
  Handle(JtNode_Shape_TriStripSet) myShape;

  //! Total number of triangles in the loaded mesh.
  Standard_Integer myTriangleCount;

private:

  //! Creates triangulation from JT reader arrays.
  void BuildTriangulation (const Handle(JtElement_ShapeLOD_TriStripSet)& theLOD);

private:
  JTData_MeshNodeSource(JTData_MeshNodeSource&);
  JTData_MeshNodeSource& operator=(JTData_MeshNodeSource&);

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_MeshNodeSource> JTData_MeshNodeSourcePtr;


//! Mesh (triangle strip) shape node element defines a collection of independent
//! and unconnected triangle strips. Each strip constitutes one primitive of the
//! set and is defined by one list of vertex coordinates.
class JTData_MeshNode : public JTData_ShapeNode
{
  friend class JTData_SceneGraph;

public:

  //! Creates new triangle mesh node.
  JTData_MeshNode (const JTCommon_AABB& theBox, const JTData_MeshNodeSourcePtr& theSource);

  //! Releases resources of triangle mesh node.
  virtual ~JTData_MeshNode();

  //! Returns estimated memory consumption in bytes.
  virtual Standard_Integer EstimateMemoryUsed (JTData_InstanceMap& theMap) const;

  //! Checks to see if the mesh node must be visualized.
  inline Standard_Boolean RequiresDrawing (JtData_State theState) const
  {
    return (myState == theState) && myIsVisible;
  }

public:

  //! Returns source of the triangulation data.
  const JTData_MeshNodeSourcePtr& Source() const
  {
    return mySource;
  }

protected:

  //! Source of the triangulation data.
  JTData_MeshNodeSourcePtr mySource;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_MeshNode> JTData_MeshNodePtr;

#endif // JTData_Node_HeaderFile
