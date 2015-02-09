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

#ifndef _BVH_Object_Header
#define _BVH_Object_Header

#include <BVH_Box.hxx>
#include <BVH_Properties.hxx>

#include <NCollection_Handle.hxx>

//! Abstract geometric object bounded by BVH box.
//! \tparam T Numeric data type
//! \tparam N Vector dimension
template<class T, int N>
class BVH_Object
{
public:

  //! Creates new abstract geometric object.
  BVH_Object();

  //! Releases resources of geometric object.
  virtual ~BVH_Object() = 0;

public:

  //! Returns AABB of the geometric object.
  virtual BVH_Box<T, N> Box() const = 0;

  //! Returns properties of the geometric object.
  virtual const NCollection_Handle<BVH_Properties>& Properties() const;

  //! Sets properties of the geometric object.
  virtual void SetProperties (const NCollection_Handle<BVH_Properties>& theProperties);

  //! Marks object state as outdated (needs BVH rebuilding).
  virtual void MarkDirty();

protected:

  Standard_Boolean                   myIsDirty;    //!< Marks internal object state as outdated
  NCollection_Handle<BVH_Properties> myProperties; //!< Generic properties assigned to the object

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_Object.lxx>

#endif // _BVH_Object_Header
