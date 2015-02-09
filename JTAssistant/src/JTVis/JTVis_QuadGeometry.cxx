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

#include "JTVis_QuadGeometry.hxx"

#pragma warning (push, 0)
#include <QtGui/QOpenGLFunctions_2_0>
#pragma warning (pop)

#include <Eigen/Geometry>

using namespace Eigen;

// =======================================================================
// function : JTVis_QuadGeometry
// purpose  :
// =======================================================================
JTVis_QuadGeometry::JTVis_QuadGeometry ()
  : myInitialized (false),
    myVertexBuffer (QOpenGLBuffer::VertexBuffer),
    myTexcoordBuffer (QOpenGLBuffer::VertexBuffer),
    myVerticeCount (6)
{
}

// =======================================================================
// function : InitializeGeometry
// purpose  :
// =======================================================================
void JTVis_QuadGeometry::InitializeGeometry (QOpenGLShaderProgram* theProgram, 
                                             Vector4f*             theQuadVertices,
                                             bool                  isFlipped)
{
  float* aVertexData = reinterpret_cast<float*> (theQuadVertices);

  QVector<Vector4f> aQuadVertices;

  if (aVertexData == NULL)
  {
    aQuadVertices.push_back (Vector4f (-1.f, -1.f, 0.f, 1.f));
    aQuadVertices.push_back (Vector4f (-1.f,  1.f, 0.f, 1.f));
    aQuadVertices.push_back (Vector4f ( 1.f,  1.f, 0.f, 1.f));
    aQuadVertices.push_back (Vector4f ( 1.f,  1.f, 0.f, 1.f));
    aQuadVertices.push_back (Vector4f ( 1.f, -1.f, 0.f, 1.f));
    aQuadVertices.push_back (Vector4f (-1.f, -1.f, 0.f, 1.f));

    aVertexData = reinterpret_cast<float*> (aQuadVertices.data());
  }

  QVector<Vector2f> aTexCoords;
  if (isFlipped)
  {
    aTexCoords.push_back (Vector2f (0.f, 1.f));
    aTexCoords.push_back (Vector2f (0.f, 0.f));
    aTexCoords.push_back (Vector2f (1.f, 0.f));
    aTexCoords.push_back (Vector2f (1.f, 0.f));
    aTexCoords.push_back (Vector2f (1.f, 1.f));
    aTexCoords.push_back (Vector2f (0.f, 1.f));
  }
  else
  {
    aTexCoords.push_back (Vector2f (0.f, 0.f));
    aTexCoords.push_back (Vector2f (0.f, 1.f));
    aTexCoords.push_back (Vector2f (1.f, 1.f));
    aTexCoords.push_back (Vector2f (1.f, 1.f));
    aTexCoords.push_back (Vector2f (1.f, 0.f));
    aTexCoords.push_back (Vector2f (0.f, 0.f));
  }

  myVertexBuffer.create();
  myVertexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myVertexBuffer.bind())
  {
    qWarning() << "Could not bind vertex buffer to the context";
    return;
  }
  myVertexBuffer.allocate (
      aVertexData,
      myVerticeCount * 4 * sizeof (float));
  myVertexBuffer.release();

  myTexcoordBuffer.create();
  myTexcoordBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myTexcoordBuffer.bind())
  {
    qWarning() << "Could not bind texcoord buffer to the context";
    return;
  }
  myTexcoordBuffer.allocate (
      reinterpret_cast<float*> (aTexCoords.data()),
      myVerticeCount * 2 * sizeof (float));
  myTexcoordBuffer.release();

  myVao.create();
  myVao.bind();
    theProgram->bind();

    myVertexBuffer.bind();
    theProgram->enableAttributeArray ("aPos");
    theProgram->setAttributeBuffer ("aPos", GL_FLOAT, 0, 4);

    myTexcoordBuffer.bind();
    theProgram->enableAttributeArray ("aTexcoord");
    theProgram->setAttributeBuffer ("aTexcoord", GL_FLOAT, 0, 2);

  myVao.release();

  myInitialized = true;
}

// =======================================================================
// function : Draw
// purpose  :
// =======================================================================
void JTVis_QuadGeometry::Draw ()
{
  if (!myInitialized) return;

  myVao.bind();
    glDrawArrays (GL_TRIANGLES, 0, myVerticeCount);
  myVao.release();
}
