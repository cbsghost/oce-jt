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

#ifndef _JtToTopoDS_Builder_HeaderFile
#define _JtToTopoDS_Builder_HeaderFile

#include <JtNode_Base.hxx>
#include <JtElement_XTBRep.hxx>

#include <TopoDS_Shape.hxx>
#include <Handle_Transfer_TransientProcess.hxx>

class Standard_Mutex;

class JtToTopoDS_Builder
{
public:

  Standard_EXPORT static TopoDS_Shape Build (const Handle(JtNode_Base)&               theObject,
                                             const Handle(Transfer_TransientProcess)& theTP);

  Standard_EXPORT static TopoDS_Shape Build (const Handle(JtElement_XTBRep)&          theObject,
                                             const Handle(Transfer_TransientProcess)& theTP,
                                             Standard_Mutex*                          theMutex = NULL);

};

#endif // _JtToTopoDS_Builder_HeaderFile
