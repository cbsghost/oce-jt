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

#ifndef JTVIS_AABBGEOMETRY_H
#define JTVIS_AABBGEOMETRY_H

#pragma warning (push, 0)
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>
#include <QVector3D>
#pragma warning (pop)

#include <Eigen/Core>

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

//! Class for managing wireframe bounding box geometry.
class JTVis_AABBGeometry: public JTVis_GraphicObject
{
public:

  //! Creates wireframe bounding box object.
  JTVis_AABBGeometry (const JTCommon_AABB& theAABB);

  //! Initializes OpenGL buffer objects.
  void InitializeGeometry (QOpenGLShaderProgram* theProgram);

  //! Draws geometry using VAO.
  void Draw (OpenGLFunctions* theOGL);

  //! Returns associated bounding box.
  JTCommon_AABB Bounds() { return myAABB; }

  //! Sets new bounding box.
  void SetBounds (const JTCommon_AABB& theBounds) { myAABB = theBounds; myInitialized = false; }

  //! Returns true if buffers already initialized.
  bool IsReady() { return myInitialized; }

private:
  bool myInitialized; //!< Indicates when buffers already initialized.

  QOpenGLVertexArrayObject myVao; //!< OpenGl Vertex Array Object (VAO).
  QOpenGLBuffer myVertexBuffer;   //!< Vertex buffer.

  int myVerticeCount;   //!< Number of vertices to draw.
  JTCommon_AABB myAABB; //!< Associated bounds object.

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_AABBGeometry> JTVis_AABBGeometryPtr;

#endif // JTVIS_AABBGEOMETRY_H
