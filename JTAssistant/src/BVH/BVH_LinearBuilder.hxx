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

#ifndef _BVH_LinearBuilder_Header
#define _BVH_LinearBuilder_Header

#include <BVH_Builder.hxx>

typedef std::pair<Standard_Integer, Standard_Integer> BVH_EncodedLink;

//! Performs fast BVH construction using LBVH building approach.
//! Algorithm uses spatial Morton codes to reduce the BVH construction
//! problem to a sorting problem (radix sort -- O(N) complexity). This
//! Linear Bounding Volume Hierarchy (LBVH) builder produces BVH trees
//! of lower quality compared to SAH-based BVH builders but it is over
//! an order of magnitude faster (up to 3M triangles per second).
//! 
//! For more details see:
//! C. Lauterbach, M. Garland, S. Sengupta, D. Luebke, and D. Manocha.
//! Fast BVH construction on GPUs. Eurographics, 2009.
template<class T, int N>
class BVH_LinearBuilder : public BVH_Builder<T, N>
{
public:

  typedef typename BVH::VectorType<T, N>::Type BVH_VecNt;

public:

  //! Creates binned LBVH builder.
  BVH_LinearBuilder (const Standard_Integer theLeafNodeSize = 5,
                     const Standard_Integer theMaxTreeDepth = 32);

  //! Releases resources of LBVH builder.
  virtual ~BVH_LinearBuilder();

  //! Builds BVH.
  void Build (BVH_Set<T, N>*       theSet,
              BVH_Tree<T, N>*      theBVH,
              const BVH_Box<T, N>& theBox);

protected:

  //! Emits hierarchy from sorted Morton codes.
  Standard_Integer EmitHierachy (BVH_Tree<T, N>*                        theBVH,
                                 const Standard_Integer                 theBit,
                                 const Standard_Integer                 theShift,
                                 std::vector<BVH_EncodedLink>::iterator theStart,
                                 std::vector<BVH_EncodedLink>::iterator theFinal);

};

#include <BVH_LinearBuilder.lxx>

#endif // _BVH_LinearBuilder_Header
