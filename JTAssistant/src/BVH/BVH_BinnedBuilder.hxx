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

#ifndef _BVH_BinnedBuilder_Header
#define _BVH_BinnedBuilder_Header

#include <BVH_QueueBuilder.hxx>

#include <algorithm>

//! Stores parameters of single node bin (slice of AABB).
template<class T, int N>
struct BVH_Bin
{
  //! Creates new node bin.
  BVH_Bin() : Count (0) {}

  Standard_Integer Count; //!< Number of primitives in the bin
  BVH_Box<T, N>    Box;   //!< AABB of primitives in the bin
};

//! Performs building of BVH tree using binned SAH algorithm.
//! Number of Bins controls tree's quality (greater - better) in cost of construction time.
template<class T, int N, int Bins = 32>
class BVH_BinnedBuilder : public BVH_QueueBuilder<T, N>
{
public:

  //! Type for the array of bins of BVH tree node.
  typedef BVH_Bin<T, N> BVH_BinVector[Bins];

public:

  //! Creates binned SAH BVH builder.
  BVH_BinnedBuilder (const Standard_Integer theLeafNodeSize = 5,
                     const Standard_Integer theMaxTreeDepth = 32);

  //! Releases resources of binned SAH BVH builder.
  virtual ~BVH_BinnedBuilder();

protected:

  //! Builds BVH node for specified task info.
  virtual void BuildNode (BVH_Set<T, N>*         theSet,
                          BVH_Tree<T, N>*        theBVH,
                          const Standard_Integer theNode);

  //! Arranges node primitives into bins.
  virtual void GetSubVolumes (BVH_Set<T, N>*         theSet,
                              BVH_Tree<T, N>*        theBVH,
                              const Standard_Integer theNode,
                              BVH_BinVector&         theBins,
                              const Standard_Integer theAxis);

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_BinnedBuilder.lxx>

#endif // _BVH_BinnedBuilder_Header
