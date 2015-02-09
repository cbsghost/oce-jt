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

#ifndef _BVH_SpatialMedianBuilder_Header
#define _BVH_SpatialMedianBuilder_Header

#include <BVH_BinnedBuilder.hxx>

//! Performs building of BVH tree using spatial median split algorithm.
template<class T, int N>
class BVH_SpatialMedianBuilder : public BVH_BinnedBuilder<T, N, 2>
{
public:

  //! Creates spatial median split builder.
  BVH_SpatialMedianBuilder (const Standard_Integer theLeafNodeSize = 5,
                            const Standard_Integer theMaxTreeDepth = 32);

  //! Releases resources of spatial median split builder.
  virtual ~BVH_SpatialMedianBuilder();

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_SpatialMedianBuilder.lxx>

#endif // _BVH_SpatialMedianBuilder_Header
