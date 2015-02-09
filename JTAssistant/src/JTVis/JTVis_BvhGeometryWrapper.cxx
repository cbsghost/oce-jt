// JT format reading and visualization tools
// Copyright (C) 2015 OPEN CASCADE SAS
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

#include "JTVis_BvhGeometryWrapper.hxx"
#include "JTVis_PartNode.hxx"

using namespace Eigen;

namespace
{

  //! The maximum positive float value.
  const Standard_Real THE_MAX_VAL = std::numeric_limits<float>::max();

  //! The 'small' (safe for inversion) 4F vector of floats.
  const BVH_Vec4f THE_MIN_VEC (1e-10f, 1e-10f, 1e-10f, 1e-10f);

}

typedef std::pair<int, float> NodeInfo;

// =======================================================================
// function : PartBvhObject
// purpose  :
// =======================================================================
JTVis_PartBvhObject::JTVis_PartBvhObject (JTVis_PartNode* thePartNode, const BVH_Box4f& theBox)
  : myBox (theBox), myPartNode (thePartNode)
{
}

// =======================================================================
// function : ~PartBvhObject
// purpose  :
// =======================================================================
JTVis_PartBvhObject::~JTVis_PartBvhObject()
{
  //
}

// =======================================================================
// function : Box
// purpose  :
// =======================================================================
BVH_Box4f JTVis_PartBvhObject::Box() const
{
  if (Properties().IsNull())
    return myBox;

  const BVH_Transform<Standard_ShortReal, 4>* aTransform =
    dynamic_cast<const BVH_Transform<Standard_ShortReal, 4>* > (Properties().operator->());

  if (aTransform)
  {
    return aTransform->Apply (myBox);
  }

  return myBox;
}

// =======================================================================
// function : SetBox
// purpose  :
// =======================================================================
void JTVis_PartBvhObject::SetBox(const BVH_Box4f &theBox)
{
  myBox = theBox;
  MarkDirty();
}

// =======================================================================
// function : stackPush
// purpose  : Pushes BVH node index to local stack
// =======================================================================
template<class T>
void stackPush (T*       theStack,
                short&   thePos,
                const T& theValue)
{
  ++thePos;
  theStack[thePos] = theValue;
}

// =======================================================================
// function : stackPop
// purpose  : Pops BVH node index from local stack
// =======================================================================
template<class T>
void stackPop (T*     theStack,
               short& thePos,
               T&     theValue)
{
  theValue = theStack[thePos];
  --thePos;
}

// =======================================================================
// function : Traverse
// purpose  :
// =======================================================================
void JTVis_BvhTraverser::Traverse (const JTVis_Frustum&       theFrustum,
                                   JTVis_FrustumIntersection& theElements,
                                   BVH_Geometry<float, 4>&    theBvhGeometry,
                                   const bool                 isOrthographic)
{
  const NCollection_Handle<BVH_Tree<float, 4> >& aBVH = theBvhGeometry.BVH();

  int aStack[32];
  signed short aHead = -1;

  int aNode = 0; // root node

  theElements.Elements.clear();
  if (aBVH.IsNull()
   || aBVH->Length() == 0)
  {
    return;
  }

  for (;;)
  {
    const BVH_Vec4i aData = aBVH->NodeInfoBuffer() [aNode];

    if (aData.x() != 1) // if inner node
    {
      const bool aHitLft = theFrustum.Intersect (aBVH->MinPoint (aData.y()),
                                                 aBVH->MaxPoint (aData.y()),
                                                 isOrthographic);

      const bool aHitRgh = theFrustum.Intersect (aBVH->MinPoint (aData.z()),
                                                 aBVH->MaxPoint (aData.z()),
                                                 isOrthographic);

      if (aHitLft && aHitRgh)
      {
        aNode = aData.y();
        stackPush (aStack, aHead, aData.z());
      }
      else
      {
        if (aHitLft || aHitRgh)
        {
          aNode = aHitLft ? aData.y() : aData.z();
        }
        else
        {
          if (aHead < 0)
          {
            return;
          }

          stackPop (aStack, aHead, aNode);
        }
      }
    }
    else  // if leaf node
    {
      theElements.Elements.push_back (aNode);

      if (aHead < 0)
      {
        return;
      }

      stackPop (aStack, aHead, aNode);
    }
  }
}
