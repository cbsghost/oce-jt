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

#ifndef JTVIS_BVHGEOMETRYWRAPPER_HXX
#define JTVIS_BVHGEOMETRYWRAPPER_HXX

#include <BVH/BVH_Geometry.hxx>
#include <BVH/BVH_Tree.hxx>

#include "JTCommon_Utils.hxx"
#include "JTVis_Frustum.hxx"

//! Helper structure to handle intersections of ray with BVH-tree.
struct JTVis_Intersection
{
  JTVis_Intersection (float theTime = -1.f) : Time (theTime) {}

  float Time; //!< Time of intersection.
};

//! Helper structure to handle intersections of frustum with BVH-tree.
struct JTVis_FrustumIntersection
{
  std::vector<Standard_Integer> Elements; //!< Intersected objects.
};

typedef BVH_Box<float, 4> BVH_Box4f;

class JTVis_PartNode;

//! Part bounding object.
class JTVis_PartBvhObject : public BVH_Object<float, 4>
{
public:

  //! Creates PartBvhObject from node id and bounding box.
  JTVis_PartBvhObject (JTVis_PartNode* thePartNode, const BVH_Box4f& theBox);

  //! Releases resources of PartBvhObject.
  ~JTVis_PartBvhObject();

  //! Returns pointer to corresponding part node.
  JTVis_PartNode* PartNode() { return myPartNode; }

  //! Returns AABB of object.
  BVH_Box4f Box() const;

  //! Sets AABB of object.
  void SetBox (const BVH_Box4f& theBox);

private:

  BVH_Box4f myBox; //!< Node bounding box.

  JTVis_PartNode* myPartNode; //!< Pointer to part node.

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

//! Helper class to store BVH traverse functions.
class JTVis_BvhTraverser
{
private:
  // Forbid class instantiation.
  JTVis_BvhTraverser() {}

public:

  //! Finds intersection of frustum with geometry elements.
  static void Traverse (const JTVis_Frustum&       theFrustum,
                        JTVis_FrustumIntersection& theElements,
                        BVH_Geometry<float, 4>&    theBvhGeometry,
                        const bool                 isOrthographic = false);

};

#endif // JTVIS_BVHGEOMETRYWRAPPER_HXX
