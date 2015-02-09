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

#include "JTData_Attribute.hxx"

// =======================================================================
// function : JTData_Attribute
// purpose  :
// =======================================================================
JTData_Attribute::JTData_Attribute()
  : myIsFinal (Standard_False),
    myIsForce (Standard_False)
{
  //
}

// =======================================================================
// function : ~JTData_Attribute
// purpose  :
// =======================================================================
JTData_Attribute::~JTData_Attribute()
{
  //
}

// =======================================================================
// function : JTData_MaterialAttribute
// purpose  :
// =======================================================================
JTData_MaterialAttribute ::JTData_MaterialAttribute (const Eigen::Vector4f& theAmbientColor,
                                                     const Eigen::Vector4f& theDiffuseColor,
                                                     const Eigen::Vector4f& theSpecularColor,
                                                     const Eigen::Vector4f& theEmissionColor,
                                                     const Standard_ShortReal theShininess)
  : myAmbientColor (theAmbientColor),
    myDiffuseColor (theDiffuseColor),
    mySpecularColor (theSpecularColor),
    myEmissionColor (theEmissionColor)
{
  myShininess = theShininess <= 128.f ?
    (theShininess >= 1.f ? theShininess : 1.f) : 128.f;
}

// =======================================================================
// function : JTData_TransformAttribute
// purpose  :
// =======================================================================
JTData_TransformAttribute::JTData_TransformAttribute (const Eigen::Matrix4f& theTransform)
  : myTransformation (theTransform)
{
  //
}
