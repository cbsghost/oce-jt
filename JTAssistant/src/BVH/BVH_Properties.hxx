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

#ifndef _BVH_Properties_Header
#define _BVH_Properties_Header

#include <BVH_Box.hxx>

#include <Standard_Macro.hxx>

//! Abstract properties of geometric object.
class BVH_Properties
{
public:

  //! Releases resources of object properties.
  Standard_EXPORT virtual ~BVH_Properties() = 0;

};

//! Stores transform properties of geometric object.
template<class T, int N>
class BVH_Transform : public BVH_Properties
{
public:

  //! Type of transformation matrix.
  typedef typename BVH::MatrixType<T, N>::Type BVH_MatNt;

public:

  //! Creates new identity transformation.
  BVH_Transform();

  //! Creates new transformation with specified matrix.
  BVH_Transform (const BVH_MatNt& theTransform);

  //! Releases resources of transformation properties.
  virtual ~BVH_Transform();

  //! Returns transformation matrix.
  const BVH_MatNt& Transform() const;

  //! Sets new transformation matrix.
  void SetTransform (const BVH_MatNt& theTransform);

  //! Returns inversed transformation matrix.
  const BVH_MatNt& Inversed() const;

  //! Applies transformation matrix to bounding box.
  BVH_Box<T, N> Apply (const BVH_Box<T, N>& theBox) const;

protected:

  BVH_MatNt myTransform;         //!< Transformation matrix
  BVH_MatNt myTransformInversed; //!< Inversed transformation matrix

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

#include <BVH_Properties.lxx>

#endif // _BVH_Properties_Header
