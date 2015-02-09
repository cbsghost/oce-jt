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

#ifndef JTVIS_PARTNODE_H
#define JTVIS_PARTNODE_H

#pragma warning (push, 0)
#include <QtCore/QVector>
#include <QtCore/QSharedPointer>
#include <QtGui/QMatrix4x4>
#pragma warning (pop)

#include "JTVis_PartGeometry.hxx"
#include "JTVis_AABBGeometry.hxx"
#include "JTCommon_Utils.hxx"
#include <JTData_Node.hxx>

#include "JTData_Attribute.hxx"

#include <Eigen/Core>
#include <Eigen/Geometry>

//! Part representation.
class JTVis_PartNode
{
public:

  //! Creates PartNode.
  JTVis_PartNode();

  //! Creates PartNode with reference to given mesh.
  JTVis_PartNode (JTData_MeshNode* theMesh);

  //! Sets new mesh for part.
  void SetGeometry (JTVis_PartGeometryPtr theGeometry);

  //! Returns current mesh of part.
  inline JTVis_PartGeometryPtr& Geometry() { return myGeometry; }

  //! Sets new transformation matrix for part.
  void SetTransform (const Eigen::Matrix4f& theTransform);

  //! Returns transformation matrix of part.
  const Eigen::Matrix4f& Transform() const { return myTransform; }

  //! Returns inversed transformation matrix of part.
  Eigen::Matrix4f& TransformInversed() { return myTransformInversed; }

  //! Returns true if geometry data already loaded.
  bool IsReady() { return myGeometry != NULL; }

  //! Cleans reference to geometry data.
  void Clear();

  //! Returns LOD state.
  inline JtData_State State() const
  {
    return myState;
  }

  //! Sets LOD state.
  inline void SetState (const JtData_State theLodState)
  {
    myState = theLodState;
  }

  //! Sets material of the mesh node.
  void SetMaterial (const JTData_MaterialAttribute& theMaterial)
  {
    Standard_Integer anOffset = 0;

    for (Standard_Integer anIdx = 0; anIdx < 4; ++anIdx)
      myMaterial[anOffset++] = theMaterial.AmbientColor()[anIdx];

    for (Standard_Integer anIdx = 0; anIdx < 4; ++anIdx)
      myMaterial[anOffset++] = theMaterial.DiffuseColor()[anIdx];

    for (Standard_Integer anIdx = 0; anIdx < 3; ++anIdx)
      myMaterial[anOffset++] = theMaterial.SpecularColor()[anIdx];

    myMaterial[anOffset] = theMaterial.Shininess();
  }

  //! Returns serialized material parameters.
  inline const Standard_ShortReal* Material() const
  {
    return myMaterial + 0;
  }

  //! Returns ambient color.
  inline const Standard_ShortReal* AmbientColor() const
  {
    return myMaterial + 0;
  }

  //! Returns diffuse color.
  inline const Standard_ShortReal* DiffuseColor() const
  {
    return myMaterial + 4;
  }

  //! Returns specular color.
  inline const Standard_ShortReal* SpecularColor() const
  {
    return myMaterial + 8;
  }

public:

  JTData_MeshNode* MeshNode; //!< Reference to corresponding scenegraph mesh node.
  JTData_RangeLODNode* RangeNode; //!< Reference to nearest scenegraph range-LOD node.

  JTCommon_AABB Bounds; //!< Transformed bounds of part.

  JTVis_AABBGeometryPtr BoxGeometry; //!< Helper object to draw bounds of part.

  int PartNodeId; //!< Index of part in main part array (in JTVis_Scene object).

  int TriangleCount; //!< Triangle count of part triangulation.

private:

  JtData_State myState; //!< Indicates visibility state of the part.

  JTVis_PartGeometryPtr myGeometry;    //!< Geometry data for part.

  Eigen::Matrix4f myTransform;         //!< Transformation matrix of part.
  Eigen::Matrix4f myTransformInversed; //!< Inversed transformation matrix of part.

  Standard_ShortReal myMaterial[12];   //!< Serialized material of the mesh node.

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_PartNode> JTVis_PartNodePtr;

#endif // JTVIS_PARTNODE_H
