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

#ifndef _BVH_Set_Header
#define _BVH_Set_Header

#include <BVH_Box.hxx>

//! Set of abstract entities (bounded by BVH boxes). This is
//! the minimal geometry interface needed to construct BVH.
//! \tparam T Numeric data type
//! \tparam N Vector dimension
template<class T, int N>
class BVH_Set
{
public:

  typedef BVH_Box<T, N> BVH_BoxNt;

public:

  //! Creates new abstract set of objects.
  BVH_Set();

  //! Releases resources of set of objects.
  virtual ~BVH_Set() = 0;

  //! Returns AABB of the entire set of objects.
  virtual BVH_Box<T, N> Box() const;

public:

  //! Returns total number of objects.
  virtual Standard_Integer Size() const = 0;

  //! Returns AABB of the given object.
  virtual BVH_Box<T, N> Box (const Standard_Integer theIndex) const = 0;

  //! Returns centroid position along the given axis.
  virtual T Center (const Standard_Integer theIndex,
                    const Standard_Integer theAxis) const = 0;

  //! Performs transposing the two given objects in the set.
  virtual void Swap (const Standard_Integer theIndex1,
                     const Standard_Integer theIndex2) = 0;

};

#include <BVH_Set.lxx>

#endif // _BVH_Set_Header
