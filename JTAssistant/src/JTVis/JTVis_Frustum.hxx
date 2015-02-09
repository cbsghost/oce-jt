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

#ifndef JTVIS_FRUSTUM_H
#define JTVIS_FRUSTUM_H

#pragma warning (push, 0)
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>
#include <QVector3D>
#pragma warning (pop)

#include <Eigen/Core>

#include <limits>

#include "JTCommon_Utils.hxx"

#include "JTVis_GraphicObject.hxx"

#pragma warning (push, 0)
#ifndef QT_OPENGL_ES_2
  #include <QtGui/QOpenGLFunctions_2_0>
  typedef QOpenGLFunctions_2_0 OpenGLFunctions;
#else
  #include <QtGui/QOpenGLFunctions_ES2>
  typedef QOpenGLFunctions_ES2 OpenGLFunctions;
#endif
#pragma warning (pop)

//! Helper structure representing 3D plane.
struct JTVis_Plane
{
  //! Default constructor.
  JTVis_Plane()
    : Point (0.f, 0.f, 0.f, 1.f),
      Normal (0.f, 0.f, 1.f, 0.f)
  {}

  //! Initializes Plane object with specific Point and Normal values.
  JTVis_Plane (const Eigen::Vector4f& thePoint, const Eigen::Vector4f& theNormal)
    : Point (thePoint),
      Normal (theNormal)
  {}

  Eigen::Vector4f Point;  //!< Reference point of plane.
  Eigen::Vector4f Normal; //!< Normal of plane.

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

//! Helper structure for 1D projection handling.
struct JTVis_Projection
{
  //! Initializes Projection object with specific min and max values.
  JTVis_Projection (float theMin =  std::numeric_limits<float>::max(),
                    float theMax = -std::numeric_limits<float>::max())
    : Min (theMin),
      Max (theMax)
  {}

  //! Extends projection with the new point.
  inline void Add (const float thePoint)
  {
    Min = thePoint < Min ? thePoint : Min;
    Max = thePoint > Max ? thePoint : Max;
  }

  //! Intersects projection with another one. Returns true if intersected.
  inline bool Intersect (const JTVis_Projection& theOther) const
  {
    return (Max >= theOther.Min) && (Min <= theOther.Max);
  }

  float Min; //!< Min bound of projection.
  float Max; //!< Max bound of projection.
};

struct JTVis_Frustum
{
  //! Makes frustum from AABB.
  JTVis_Frustum (const JTCommon_AABB& theAABB);

  //! Makes frustum from inversed view-projection camera matrix.
  JTVis_Frustum (const Eigen::Matrix4f& theMatrix);

  //! Intersects frustum with given bounds. Returns true if intersected.
  bool Intersect (const Eigen::Vector4f& theMinPoint,
                  const Eigen::Vector4f& theMaxPoint,
                  const bool             isOrthographic = false) const;

  //! Updates planes of frustum assuming points have been already updated.
  void UpdatePlanes();

  JTVis_Plane     Planes[6]; //!< Planes of frustum.
  Eigen::Vector4f Points[8]; //!< Points of frustum.

  JTVis_Projection MainProjections[3]; //!< Precalculated projections of frustum
                                       //!< onto main ijk world basis

  JTVis_Projection FrustumProjections[6]; //!< Precalculated projections of frustum
                                          //!< onto its own normal axes

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

//! Class for managing frustum geometry.
class JTVis_FrustumGeometry: public JTVis_GraphicObject
{
public:

  //! Makes FrustumGeometry object with given Frustum object.
  JTVis_FrustumGeometry (const JTVis_Frustum& theFrustum);

  //! Initializes OpenGL buffer objects.
  void InitializeGeometry (QOpenGLShaderProgram* theProgram);

  //! Draws geometry using VAO.
  void Draw (OpenGLFunctions* theOGL);

  //! Returns associated frustum object.
  JTVis_Frustum Frustum() { return myFrustum; }

  //! Returns true if buffers already initialized.
  bool IsReady() { return myInitialized; }

private:

  bool myInitialized; //!< Indicates when buffers already initialized.

  QOpenGLVertexArrayObject myVao; //!< OpenGL Vertex Array Object (VAO).
  QOpenGLBuffer myVertexBuffer;   //!< Vertex buffer.

  int myVerticeCount;      //!< Number of vertices to draw.
  JTVis_Frustum myFrustum; //!< Associated frustum object.

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_FrustumGeometry> JTVis_FrustumGeometryPtr;

#endif // JTVIS_FRUSTUM_H
