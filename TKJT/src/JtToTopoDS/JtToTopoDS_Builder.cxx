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

#include <JtToTopoDS_Builder.hxx>

#include <JtData_Message.hxx>

#include <JtNode_Part.hxx>
#include <JtNode_Instance.hxx>
#include <JtAttribute_GeometricTransform.hxx>

#include <BRep_Builder.hxx>
#include <Standard_IStream.hxx>
#include <Standard_Mutex.hxx>
#include <TColStd_HArray1OfCharacter.hxx>
#include <TopoDS_Compound.hxx>

#define OCC_JTREADER_NO_XT
#ifndef OCC_JTREADER_NO_XT

#include <TransferBRep.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XtControl_Reader.hxx>

#pragma comment(lib,"TKXSBase.lib")
#pragma comment(lib,"TKXT.lib")
#pragma comment(lib,"TKOCCLisence.lib")
#endif

//=======================================================================
//function : Build
//purpose  :
//=======================================================================

TopoDS_Shape JtToTopoDS_Builder::Build (
#ifdef OCC_JTREADER_NO_XT
  const Handle(JtNode_Base)&, const Handle(Transfer_TransientProcess)&
#else
  const Handle(JtNode_Base)&               theObject,
  const Handle(Transfer_TransientProcess)& theTP
#endif
  )
{
#ifdef OCC_JTREADER_NO_XT
  return TopoDS_Shape();
#else
  if (theObject.IsNull())
  {
    return TopoDS_Shape();
  }
  else if (theTP->IsBound (theObject))
  {
    return TransferBRep::ShapeResult (theTP, theObject);
  }

  TopoDS_Shape aResult;
  if (theObject->IsKind (STANDARD_TYPE (JtNode_Part)))
  {
    // take a part and iterate among late loads
    const JtData_Object::VectorOfLateLoads& aVec =
      Handle(JtNode_Part)::DownCast (theObject)->LateLoads();
    for (JtData_Object::VectorOfLateLoads::SizeType i = 0; i < aVec.Count(); i++)
    {
      // Read the referred XT B-Rep
      aVec[i]->Load();
      Handle(JtElement_XTBRep) aXTBRep =
        Handle(JtElement_XTBRep)::DownCast (aVec[i]->DefferedObject());
      aVec[i]->Unload();
      if (!aXTBRep.IsNull())
        aResult = Build (aXTBRep, theTP);
    }
  }
  else if (theObject->IsKind (STANDARD_TYPE (JtNode_Group)))
  {
    const JtData_Object::VectorOfObjects& aChildren = Handle(JtNode_Group)::DownCast (theObject)->Children();
    if (!aChildren.IsEmpty())
    {
      TopoDS_Compound aComp;
      BRep_Builder B;
      B.MakeCompound (aComp);
      for (JtData_Object::VectorOfObjects::SizeType i = 0; i < aChildren.Count(); i++)
      {
        Handle(JtNode_Base) aObj = Handle(JtNode_Base)::DownCast (aChildren[i]);
        if (!aObj.IsNull())
        {
          TopoDS_Shape aShape = Build (aObj, theTP);
          if (!aShape.IsNull())
            B.Add (aComp, aShape);
        }
      }
      aResult = aComp;
    }
  }
  else if (theObject->IsKind (STANDARD_TYPE (JtNode_Instance)))
  {
    Handle(JtNode_Instance) anInstance = Handle(JtNode_Instance)::DownCast (theObject);
    Handle(JtNode_Base)     aObj       = Handle(JtNode_Base)::DownCast (anInstance->Object());
    if (!aObj.IsNull())
    {
      aResult = Build (aObj, theTP);
      if (!aResult.IsNull())
      {
        // Find transformation attribute
        const JtData_Object::VectorOfObjects& anAttributes = anInstance->Attributes();
        for (JtData_Object::VectorOfObjects::SizeType i = 0; i < anAttributes.Count(); i++)
        {
          const Handle(JtData_Object)& anAttribute = anAttributes[i];
          if (anAttribute->IsKind (STANDARD_TYPE (JtAttribute_GeometricTransform)))
          {
            gp_Trsf aTrsf;
            Handle(JtAttribute_GeometricTransform)::DownCast (anAttribute)->GetTrsf (aTrsf);
            // Apply transformation
            if (aTrsf.Form() != gp_Identity)
            {
              TopLoc_Location aLoc (aTrsf);
              aResult.Move (aLoc);
            }
          }
        }
      }
    }
  }

  TransferBRep::SetShapeResult (theTP, theObject, aResult);
  return aResult;
#endif
}

//=======================================================================
//function : Build
//purpose  :
//=======================================================================

TopoDS_Shape JtToTopoDS_Builder::Build (
#ifdef OCC_JTREADER_NO_XT
  const Handle(JtElement_XTBRep)&,
  const Handle(Transfer_TransientProcess)&,
  Standard_Mutex*
#else
  const Handle(JtElement_XTBRep)&          theObject,
  const Handle(Transfer_TransientProcess)& theTP,
  Standard_Mutex*                          theMutex
#endif
  )
{
#ifdef OCC_JTREADER_NO_XT
  return TopoDS_Shape();
#else
  {
    Standard_Mutex::Sentry aSentry (theMutex);
    if (theTP->IsBound (theObject))
    {
      return TransferBRep::ShapeResult (theTP, theObject);
    }
  }

  // Create stream from array
  struct membuf : std::streambuf
  {
    membuf (const Jt_String& theString)
    {
      char* b = const_cast<char*> (reinterpret_cast<const char*> (theString.Data()));
      setg (b, b, b + theString.Count());
    }
  } sbuf (theObject->Data());
  Standard_IStream aStream (&sbuf);

  XtControl_Reader aReader;
  const IFSelect_ReturnStatus aReadState = aReader.ReadStream (aStream);
  theObject->ClearData();
  TopoDS_Shape aResult;
  if (aReadState != IFSelect_RetDone)
  {
    ALARM ("Error: Could not read Parasolid data");
  }
  else
  {
    const Standard_Integer aNbRoots = aReader.TransferRoots();
    if (aNbRoots == 0)
    {
      ALARM ("Error: Could not translate Parasolid data");
    }
    else
    {
      const Standard_Integer aNbShapes = aReader.NbShapes();
      if (aNbShapes == 0)
      {
        ALARM ("Error: No shapes in Parasolid data");
      }
      else if (aNbShapes == 1)
      {
        aResult = aReader.Shape (1);
      }
      else
      {
        TopoDS_Compound aComp;
        BRep_Builder B;
        B.MakeCompound (aComp);
        for (Standard_Integer aShapeIter = 1; aShapeIter <= aNbShapes; ++aShapeIter)
        {
          TopoDS_Shape aS = aReader.Shape (aShapeIter);
          if (!aS.IsNull())
          {
            B.Add (aComp, aS);
          }
        }
        aResult = aComp;
      }
    }
  }

  Standard_Mutex::Sentry aSentry (theMutex);
  TransferBRep::SetShapeResult (theTP, theObject, aResult);
  return aResult;
#endif
}
