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

#ifndef JTData_Element_HeaderFile
#define JTData_Element_HeaderFile

#include "JTCommon_Utils.hxx"


//! Specific type for object state.
typedef unsigned long long int JtData_State;


//! Graph elements form the backbone of the LSG directed acyclic graph
//! structure. There are two general classifications of graph elements:
//! node elements and attribute elements. Each of these classifications
//! (node/attribute elements) is sub-typed into specific/concrete types
//! based on data content and specialized behavior.
class JTData_Element
{
public:

  //! Creates abstract scene graph element.
  JTData_Element();

  //! Releases resources of abstract scene graph element.
  virtual ~JTData_Element() = 0;

public:

  //! Returns element ignore flag.
  inline Standard_Boolean IsIgnored() const
  {
    return myIsIgnored;
  }

  //! Sets element ignore flag.
  inline void SetIgnored (const Standard_Boolean theIsIgnored)
  {
    myIsIgnored = theIsIgnored;
  }

  //! Returns object state.
  inline JtData_State State() const
  {
    return myState;
  }

  //! Sets object state.
  inline void SetState (const JtData_State theState)
  {
    myState = theState;
  }

protected:

  //! Marks that element should be ignored by traversing algorithms.
  Standard_Boolean myIsIgnored;

  //! Indicates state of the JT object (used by service sub-routines).
  JtData_State myState;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_Element> JTData_ElementPtr;

#endif // JTData_Element_HeaderFile
