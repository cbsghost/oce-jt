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

#ifndef _BVH_PrimitiveSet_Header
#define _BVH_PrimitiveSet_Header

#include <BVH_Object.hxx>
#include <BVH_Builder.hxx>

//! Set of abstract geometric primitives organized with bounding
//! volume hierarchy (BVH). Unlike an object set, this collection
//! is designed for storing structural elements of a single object
//! (such as triangles in the object triangulation). Because there
//! may be a large number of such elements, the implementations of
//! this interface should be sufficiently optimized.
//! \tparam T Numeric data type
//! \tparam N Vector dimension
template<class T, int N>
class BVH_PrimitiveSet : public BVH_Object<T, N>, public BVH_Set<T, N>
{
protected:

  using BVH_Set<T, N>::Box;

public:

  //! Creates set of abstract primitives.
  BVH_PrimitiveSet();

  //! Releases resources of set of abstract primitives.
  virtual ~BVH_PrimitiveSet();

public:

  //! Returns AABB of primitive set.
  virtual BVH_Box<T, N> Box() const;

  //! Returns BVH tree (and builds it if necessary).
  virtual const NCollection_Handle<BVH_Tree<T, N> >& BVH();

  //! Returns the method (builder) used to construct BVH.
  virtual const NCollection_Handle<BVH_Builder<T, N> >& Builder() const;

  //! Sets the method (builder) used to construct BVH.
  virtual void SetBuilder (NCollection_Handle<BVH_Builder<T, N> >& theBuilder);

protected:

  //! Updates BVH of primitive set.
  virtual void Update();

protected:

  NCollection_Handle<BVH_Tree<T, N> >    myBVH;     //!< Constructed bottom-level BVH
  NCollection_Handle<BVH_Builder<T, N> > myBuilder; //!< Builder for bottom-level BVH

  mutable BVH_Box<T, N> myBox; //!< Cached bounding box of geometric primitives

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_PrimitiveSet.lxx>

#endif // _BVH_PrimitiveSet_Header
