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

#ifndef JTVIS_TRIHEDRON_GEOMETRY_H
#define JTVIS_TRIHEDRON_GEOMETRY_H

#pragma warning (push, 0)
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>
#include <QVector3D>

#ifndef QT_OPENGL_ES_2
  #include <QtGui/QOpenGLFunctions_2_0>
  typedef QOpenGLFunctions_2_0 OpenGLFunctions;
#else
  #include <QtGui/QOpenGLFunctions_ES2>
  typedef QOpenGLFunctions_ES2 OpenGLFunctions;
#endif
#pragma warning (pop)

#include <Eigen/Core>

#include "JTCommon_Utils.hxx"

#include "JTVis_GraphicObject.hxx"

//! Class for managing trihedron geometry.
class JTVis_TrihedronGeometry: public JTVis_GraphicObject
{
public:

  //! Creates TrihedronGeometry object.
  JTVis_TrihedronGeometry();

  //! Initializes OpenGL buffer objects.
  void InitializeGeometry (QOpenGLShaderProgram* theProgram);

  //! Draws geometry using VAO.
  void Draw (OpenGLFunctions* theOGL);

  //! Returns true if buffers already initialized.
  bool IsReady() { return myInitialized; }

private:
  bool myInitialized; //!< Indicates when buffers already initialized.

  QOpenGLVertexArrayObject myVao; //!< OpenGl Vertex Array Object (VAO).
  QOpenGLBuffer myVertexBuffer;   //!< Vertex buffer.
  QOpenGLBuffer myColorBuffer;    //!< Color buffer.

  int myVerticeCount; //!< Number of vertices to draw.

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_TrihedronGeometry> JTVis_TrihedronGeometryPtr;

#endif // JTVIS_TRIHEDRON_GEOMETRY_H
