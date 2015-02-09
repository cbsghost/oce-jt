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

#ifndef _BVH_QueueBuilder_Header
#define _BVH_QueueBuilder_Header

#include <BVH_Builder.hxx>

#include <NCollection_Vector.hxx>

//! Abstract BVH builder based on the concept of work queue.
template<class T, int N>
class BVH_QueueBuilder : public BVH_Builder<T, N>
{
public:

  //! Creates new BVH queue based builder.
  BVH_QueueBuilder (const Standard_Integer theLeafNodeSize,
                    const Standard_Integer theMaxTreeDepth);

  //! Releases resources of BVH queue based builder.
  virtual ~BVH_QueueBuilder() = 0;

public:

  //! Builds BVH using specific algorithm.
  virtual void Build (BVH_Set<T, N>*       theSet,
                      BVH_Tree<T, N>*      theBVH,
                      const BVH_Box<T, N>& theBox);

protected:

  //! Builds BVH node for the given task info.
  virtual void BuildNode (BVH_Set<T, N>*         theSet,
                          BVH_Tree<T, N>*        theBVH,
                          const Standard_Integer theTask);

protected:

  NCollection_Vector<Standard_Integer> myTasksQueue;   //!< Queue to manage BVH node building tasks

};

#include <BVH_QueueBuilder.lxx>

#endif // _BVH_QueueBuilder_Header
