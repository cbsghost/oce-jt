// JT format reading and visualization tools
// Copyright (C) 2013-2015 OPEN CASCADE SAS
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

#ifndef _BVH_SweepPlaneBuilder_Header
#define _BVH_SweepPlaneBuilder_Header

#include <BVH_QueueBuilder.hxx>

//! Performs building of BVH tree using sweep plane SAH algorithm.
template<class T, int N>
class BVH_SweepPlaneBuilder : public BVH_QueueBuilder<T, N>
{
public:

  //! Creates sweep plane SAH BVH builder.
  BVH_SweepPlaneBuilder (const Standard_Integer theLeafNodeSize = 5,
                         const Standard_Integer theMaxTreeDepth = 32);

  //! Releases resources of sweep plane SAH BVH builder.
  virtual ~BVH_SweepPlaneBuilder();

protected:

  //! Builds specified BVH node.
  virtual void BuildNode (BVH_Set<T, N>*         theSet,
                          BVH_Tree<T, N>*        theBVH,
                          const Standard_Integer theNode);

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_SweepPlaneBuilder.lxx>

#endif // _BVH_SweepPlaneBuilder_Header
