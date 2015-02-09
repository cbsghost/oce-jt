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

#include "JTVis_PartNode.hxx"

using namespace Eigen;

// =======================================================================
// function : JTVis_PartNode
// purpose  :
// =======================================================================
JTVis_PartNode::JTVis_PartNode()
  : MeshNode  (NULL),
    RangeNode (NULL),
    PartNodeId (0),
    TriangleCount (0),
    myState   (0)
{
  myTransform = Matrix4f::Identity();
}

// =======================================================================
// function : JTVis_PartNode
// purpose  :
// =======================================================================
JTVis_PartNode::JTVis_PartNode (JTData_MeshNode* theMesh) 
  : MeshNode  (theMesh),
    RangeNode (NULL),
    PartNodeId (0),
    TriangleCount (0),
    myState   (0)
{
  myTransform = Matrix4f::Identity();
}

// =======================================================================
// function : SetGeometry
// purpose  :
// =======================================================================
void JTVis_PartNode::SetGeometry (JTVis_PartGeometryPtr theGeometry)
{
  myGeometry = theGeometry;
}

// =======================================================================
// function : Clear
// purpose  :
// =======================================================================
void JTVis_PartNode::Clear()
{
  myGeometry.reset();
}

// =======================================================================
// function : SetTransform
// purpose  :
// =======================================================================
void JTVis_PartNode::SetTransform (const Eigen::Matrix4f& theTransform)
{
  myTransform = theTransform;
  myTransformInversed = myTransform.inverse();
}
