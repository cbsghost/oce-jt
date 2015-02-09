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

#ifndef JTVIS_PARTGEOMETRY_H
#define JTVIS_PARTGEOMETRY_H

#pragma warning (push, 0)
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>
#include <QVector3D>
#pragma warning (pop)

#include "JTCommon_Utils.hxx"

#pragma warning (push, 0)
#ifndef QT_OPENGL_ES_2
  #include <QtGui/QOpenGLFunctions_2_0>
  typedef QOpenGLFunctions_2_0 OpenGLFunctions;
#else
  #include <QtGui/QOpenGLFunctions_ES2>
  typedef QOpenGLFunctions_ES2 OpenGLFunctions;
#endif
#pragma warning (pop)

class JTVis_PartGeometry;

typedef void (*DrawRangedElementsType)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*);

//! Geometry aggregator object which helps to minimize
//! buffer switching while draw plenty of small objects.
class JTVis_PartGeometryAggregator
{
  friend class JTVis_PartGeometry;

public:

  //! Creates PartGeometryAggregator.
  JTVis_PartGeometryAggregator();

  //! Allocates OpenGL vertex buffer objects with size of "theMaxSize" vertices.
  //! Stores attribute bindings for given shader program in its VAO.
  void Initialize (QOpenGLShaderProgram* theProgram, Standard_Integer theMaxSize);

  //! Adds to its OpenGL vertex buffers data from the triangulation object.
  //! Initializes given index buffer applying correct offset to triangulation indices.
  //! @return the indices count.
  int AddMesh (const JTCommon_TriangleDataPtr& theTriangulation,
                              OpenGLFunctions* theOGL,
                                QOpenGLBuffer* theIndexBuffer,
                                          int& theStart,
                                          int& theEnd);

  //! Returns occupancy of vertex buffers.
  int BufferUsage() { return (int)(100.f * myVertexCount / myMaxSize); }

private:

  bool myInitialized; //!< Indicates when buffers already initialized.

  QOpenGLVertexArrayObject myVao; //!< OpenGL Vertex Array Object (VAO).
  QOpenGLBuffer myVertexBuffer;   //!< Vertex buffer.
  QOpenGLBuffer myNormalBuffer;   //!< Normal buffer.

  int myVertexCount; //!< Total counter for provided vertex data.

  unsigned int myMaxSize; //!< Capacity of vertex buffers.

};

//! Class for managing part geometry (triangulation).
//! JTVis_PartGeometry objects may share vertex buffers with
//! aid of JTVis_PartGeometryAggregator.
class JTVis_PartGeometry
{
public:

  //! Creates PartGeometry object.
  JTVis_PartGeometry();

  //! Initializes OpenGL buffer objects with data from the triangulation object.
  //! Stores attribute bindings for given shader program in VAO.
  void InitializeGeometry (QOpenGLShaderProgram* theProgram,
                           const JTCommon_TriangleDataPtr& theTriangulation);

  //! Initializes OpenGL buffer objects with data from the triangulation object.
  //! Uses vertex attribute buffers of the part geometry aggregator instead of its own buffers.
  //! Indices still stored locally in PartGeometry object.
  void InitializeGeometry (OpenGLFunctions* theOGL,
                           const JTCommon_TriangleDataPtr& theTriangulation,
                           JTVis_PartGeometryAggregator& theAggregator);

  //! Draws geometry using VAO assuming default shader program is bound
  //! and corresponding attribute locations did not change.
  unsigned int Draw (OpenGLFunctions* theOGL, unsigned int theCurrentVao);

  //! Draws geometry without VAO assuming "theProgram" shader program is bound
  //! and using its attribute locations.
  void Draw (OpenGLFunctions* theOGL, QOpenGLShaderProgram* theProgram);

  //! Returns true if buffers already initialized.
  bool IsReady() { return myInitialized; }

  //! Returns mesh triangle count.
  int TriangleCount() { return myIndicesCount / 3; }

  //! Returns true if PartGeometry uses aggregator.
  bool UsesAggregator() { return myAggregator != NULL; }

private:
  bool myInitialized; //!< Indicates when buffers already initialized.

  QOpenGLVertexArrayObject myVao; //!< OpenGL Vertex Array Object (VAO).
  QOpenGLBuffer myVertexBuffer;   //!< Vertex buffer.
  QOpenGLBuffer myNormalBuffer;   //!< Normal buffer.
  QOpenGLBuffer myIndexBuffer;    //!< Index buffer.

  int myIndicesCount; //!< Number of indices to draw.

  JTVis_PartGeometryAggregator* myAggregator; //!< Pointer to geometry aggregator.
                                              //!< When pointer is null aggregator doesn't used.
  int myStart; //!< Start vertex index (for glDrawRangeElements call).
  int myEnd;   //!< End vertex index (for glDrawRangeElements call).

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_PartGeometry> JTVis_PartGeometryPtr;

#endif // JTVIS_PARTGEOMETRY_H
