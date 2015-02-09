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

#ifndef _BVH_Sorter_Header
#define _BVH_Sorter_Header

#include <BVH_Set.hxx>

//! Performs centroid-based sorting of abstract set.
template<class T, int N>
class BVH_Sorter
{
public:

  //! Sorts the set by centroids coordinates in specified axis.
  static void Perform (BVH_Set<T, N>*         theSet,
                       const Standard_Integer theAxis);

  //! Sorts the set by centroids coordinates in specified axis.
  static void Perform (BVH_Set<T, N>*         theSet,
                       const Standard_Integer theAxis,
                       const Standard_Integer theBegElement,
                       const Standard_Integer theEndElement);

};

#include <BVH_Sorter.lxx>

#endif // _BVH_Sorter_Header
