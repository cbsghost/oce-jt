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

#ifndef JTData_Attribute_HeaderFile
#define JTData_Attribute_HeaderFile

#include "JTData_Element.hxx"


//! Attribute elements represent graphical data (appearance characteristics
//! or positional transformations) that can be attached to scene graph node,
//! and inherit down the scene graph (LSG). Each attribute type defines its
//! accumulation semantics. Generally attributes at lower levels of LSG take
//! precedence and replace / accumulate with attributes set at higher levels.
//! Nodes without associated attributes inherit those of their parents. Note
//! that attributes inherit only from parent nodes: attributes do not affect
//! the siblings of the node.
class JTData_Attribute : public JTData_Element
{
public:

  //! Creates new attribute element.
  JTData_Attribute();

  //! Releases resources of abstract node attribute.
  virtual ~JTData_Attribute() = 0;

public:

  //! Returns accumulated final flag.
  inline Standard_Boolean IsFinal() const
  {
    return myIsFinal;
  }

  //! Returns accumulated force flag.
  inline Standard_Boolean IsForce() const
  {
    return myIsForce;
  }

  //! Sets accumulated final flag.
  inline void SetFinal (const Standard_Boolean theIsFinal)
  {
    myIsFinal = theIsFinal;
  }

  //! Returns accumulated force flag.
  inline void SetForce (const Standard_Boolean theIsForce)
  {
    myIsForce = theIsForce;
  }

protected:

  //! Accumulated final flag.
  Standard_Boolean myIsFinal;

  //! Accumulated force flag.
  Standard_Boolean myIsForce;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_Attribute> JTData_AttributePtr;


//! Array of node attributes.
typedef std::vector<JTData_AttributePtr> JTData_ArrayOfAttributes;


//! Material attribute element defines the material properties of a object.
//! LSG traversal semantics state that material attributes accumulate down
//! the scene graph by replacement.
class JTData_MaterialAttribute : public JTData_Attribute
{
public:

  //! Creates new material attribute.
  JTData_MaterialAttribute (const Eigen::Vector4f& theAmbientColor = Eigen::Vector4f::Zero(),
                            const Eigen::Vector4f& theDiffuseColor = Eigen::Vector4f::Ones(),
                            const Eigen::Vector4f& theSpecularColor = Eigen::Vector4f::Zero(),
                            const Eigen::Vector4f& theEmissionColor = Eigen::Vector4f::Zero(),
                            const Standard_ShortReal theShininess = 32.f);

  //! Returns ambient color.
  inline const Eigen::Vector4f& AmbientColor() const
  {
    return myAmbientColor;
  }

  //! Returns diffuse color.
  inline const Eigen::Vector4f& DiffuseColor() const
  {
    return myDiffuseColor;
  }

  //! Returns specular color.
  inline const Eigen::Vector4f& SpecularColor() const
  {
    return mySpecularColor;
  }

  //! Returns emission color.
  inline const Eigen::Vector4f& EmissionColor() const
  {
    return myEmissionColor;
  }

  //! Returns shininess exponent.
  inline Standard_ShortReal Shininess() const
  {
    return myShininess;
  }

  //! Sets ambient color.
  inline void SetAmbientColor (const Eigen::Vector4f& theAmbientColor)
  {
    myAmbientColor = theAmbientColor;
  }

  //! Sets diffuse color.
  inline void SetDiffuseColor (const Eigen::Vector4f& theDiffuseColor)
  {
    myDiffuseColor = theDiffuseColor;
  }

  //! Sets specular color.
  inline void SetSpecularColor (const Eigen::Vector4f& theSpecularColor)
  {
    mySpecularColor = theSpecularColor;
  }

  //! Sets emission color.
  inline void SetEmissionColor (const Eigen::Vector4f& theEmissionColor)
  {
    myEmissionColor = theEmissionColor;
  }

  //! Sets shininess exponent.
  inline void SetShininess (Standard_ShortReal theShininess)
  {
    myShininess = theShininess;
  }

  //! Returns serialized representation of the material.
  inline float* Data()
  {
    return myAmbientColor.data();
  }

protected:

  //! Ambient color.
  Eigen::Vector4f myAmbientColor;

  //! Diffuse color.
  Eigen::Vector4f myDiffuseColor;

  //! Specular color.
  Eigen::Vector4f mySpecularColor;

  //! Emission color.
  Eigen::Vector4f myEmissionColor;

  //! Shininess value.
  Standard_ShortReal myShininess;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_MaterialAttribute> JTData_MaterialAttributePtr;


//! Geometric transform attribute element contains a 4x4 transformation
//! matrix that positions the associated LSG node relative to its parent
//! LSG node.
class JTData_TransformAttribute : public JTData_Attribute
{
public:

  //! Creates new transformation attribute.
  JTData_TransformAttribute (const Eigen::Matrix4f& theTransform);

public:

  //! Returns transformation matrix.
  inline const Eigen::Matrix4f& Transform() const
  {
    return myTransformation;
  }

  //! Sets transformation matrix.
  inline void SetTransform (const Eigen::Matrix4f& theTransform)
  {
    myTransformation = theTransform;
  }

protected:

  //! 4x4 transformation matrix.
  Eigen::Matrix4f myTransformation;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

typedef QSharedPointer<JTData_TransformAttribute> JTData_TransformAttributePtr;

#endif // JTData_Attribute_HeaderFile
