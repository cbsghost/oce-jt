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

#ifndef _BVH_Geometry_Header
#define _BVH_Geometry_Header

#include <BVH_ObjectSet.hxx>
#include <BVH_Builder.hxx>

//! BVH geometry as a set of abstract geometric objects
//! organized with bounding volume hierarchy (BVH).
//! \tparam T Numeric data type
//! \tparam N Vector dimension
template<class T, int N>
class BVH_Geometry : public BVH_ObjectSet<T, N>
{
public:

  //! Creates uninitialized BVH geometry.
  BVH_Geometry();

  //! Releases resources of BVH geometry.
  virtual ~BVH_Geometry();

public:

  //! Marks geometry as outdated.
  virtual void MarkDirty();

  //! Returns AABB of the given object.
  using BVH_ObjectSet<T, N>::Box;

  //! Returns AABB of the whole geometry.
  virtual BVH_Box<T, N> Box() const;

  //! Returns BVH tree (and builds it if necessary).
  virtual const NCollection_Handle<BVH_Tree<T, N> >& BVH();

  //! Returns the method (builder) used to construct BVH.
  virtual const NCollection_Handle<BVH_Builder<T, N> >& Builder() const;

  //! Sets the method (builder) used to construct BVH.
  virtual void SetBuilder (NCollection_Handle<BVH_Builder<T, N> >& theBuilder);

protected:

  //! Updates internal geometry state.
  virtual void Update();

protected:

  Standard_Boolean                       myIsDirty; //!< Is geometry state outdated?
  NCollection_Handle<BVH_Tree<T, N> >    myBVH;     //!< Constructed hight-level BVH
  NCollection_Handle<BVH_Builder<T, N> > myBuilder; //!< Builder for hight-level BVH

  mutable BVH_Box<T, N> myBox; //!< Cached bounding box of geometric objects

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_Geometry.lxx>

#endif // _BVH_Geometry_Header
