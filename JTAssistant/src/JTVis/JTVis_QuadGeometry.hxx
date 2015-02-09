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

#ifndef JTVIS_SCREEN_QUAD_H
#define JTVIS_SCREEN_QUAD_H

#pragma warning (push, 0)
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>
#pragma warning (pop)

#include <Eigen/Core>

#include "JTCommon_Utils.hxx"

//! Class for managing quad geometry.
class JTVis_QuadGeometry
{
public:

  //! Creates QuadGeometry object.
  JTVis_QuadGeometry();

  //! Initializes OpenGL buffer objects.
  void InitializeGeometry (QOpenGLShaderProgram* theProgram,
                           Eigen::Vector4f* aQuadVertices = NULL,
                           bool isFlipped = false);

  //! Draws geometry using VAO.
  void Draw();

private:

  bool myInitialized; //!< Indicates when buffers already initialized.

  QOpenGLVertexArrayObject myVao; //!< OpenGL Vertex Array Object (VAO).
  QOpenGLBuffer myVertexBuffer;   //!< Vertex buffer.
  QOpenGLBuffer myTexcoordBuffer; //!< Buffer of texture coordinates.

  int myVerticeCount; //!< Number of vertices to draw.

};

typedef QSharedPointer<JTVis_QuadGeometry> JTVis_QuadGeometryPtr;

#endif // JTVIS_SCREEN_QUAD_H
