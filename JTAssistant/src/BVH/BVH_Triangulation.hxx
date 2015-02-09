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

#ifndef _BVH_Triangulation_Header
#define _BVH_Triangulation_Header

#include <BVH_PrimitiveSet.hxx>

//! Triangulation as an example of BVH primitive set.
//! \tparam T Numeric data type
//! \tparam N Vector dimension
template<class T, int N>
class BVH_Triangulation : public BVH_PrimitiveSet<T, N>
{
public:

  typedef typename BVH::VectorType<T, N>::Type BVH_VecNt;

public:

  //! Creates empty triangulation.
  BVH_Triangulation();

  //! Releases resources of triangulation.
  virtual ~BVH_Triangulation();

public:

  //! Array of vertex coordinates.
  typename BVH::ArrayType<T, N>::Type Vertices;

  //! Array of indices of triangle vertices.
  BVH_Array4i Elements;

public:

  //! Returns total number of triangles.
  virtual Standard_Integer Size() const;

  //! Returns AABB of entire set of objects.
  using BVH_PrimitiveSet<T, N>::Box;

  //! Returns AABB of the given triangle.
  virtual BVH_Box<T, N> Box (const Standard_Integer theIndex) const;

  //! Returns centroid position along the given axis.
  virtual T Center (const Standard_Integer theIndex,
                    const Standard_Integer theAxis) const;

  //! Performs transposing the two given triangles in the set.
  virtual void Swap (const Standard_Integer theIndex1,
                     const Standard_Integer theIndex2);

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_Triangulation.lxx>

#endif // _BVH_Triangulation_Header
