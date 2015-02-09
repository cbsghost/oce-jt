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

#include "JTVis_TrihedronGeometry.hxx"

using namespace Eigen;

// =======================================================================
// function : JTVis_TrihedronGeometry
// purpose  :
// =======================================================================
JTVis_TrihedronGeometry::JTVis_TrihedronGeometry ()
  : myInitialized (false),
    myVertexBuffer (QOpenGLBuffer::VertexBuffer),
    myColorBuffer (QOpenGLBuffer::VertexBuffer),
    myVerticeCount (0)
{
}

// =======================================================================
// function : InitializeGeometry
// purpose  :
// =======================================================================
void JTVis_TrihedronGeometry::InitializeGeometry (QOpenGLShaderProgram* theProgram)
{
  QVector<Vector3f> aLinesVertices;
  QVector<Vector3f> aVertexColors;

  aLinesVertices.push_back (Vector3f (0.f, 0.f, 0.f));
  aVertexColors.push_back  (Vector3f (1.f, 0.f, 0.f));
  aLinesVertices.push_back (Vector3f (0.9f, 0.f, 0.f));
  aVertexColors.push_back  (Vector3f (1.f, 0.f, 0.f));

  aLinesVertices.push_back (Vector3f (0.f, 0.f, 0.f));
  aVertexColors.push_back  (Vector3f (0.f, 1.f, 0.f));
  aLinesVertices.push_back (Vector3f (0.f, 0.9f, 0.f));
  aVertexColors.push_back  (Vector3f (0.f, 1.f, 0.f));

  aLinesVertices.push_back (Vector3f (0.f, 0.f, 0.f));
  aVertexColors.push_back  (Vector3f (0.f, 0.f, 1.f));
  aLinesVertices.push_back (Vector3f (0.f, 0.f, 0.9f));
  aVertexColors.push_back  (Vector3f (0.f, 0.f, 1.f));

  myVertexBuffer.create();
  myVertexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myVertexBuffer.bind())
  {
    qWarning() << "Could not bind vertex buffer to the context";
    return;
  }
  myVertexBuffer.allocate (
    reinterpret_cast<float*> (aLinesVertices.data()),
    aLinesVertices.size() * 3 * sizeof (float));
  myVertexBuffer.release();

  myColorBuffer.create();
  myColorBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myColorBuffer.bind())
  {
    qWarning() << "Could not bind color buffer to the context";
    return;
  }
  myColorBuffer.allocate (
    reinterpret_cast<float*> (aVertexColors.data()),
    aVertexColors.size() * 3 * sizeof (float));
  myColorBuffer.release();

  myVao.create();
  myVao.bind();
    theProgram->bind();

    myVertexBuffer.bind();
    theProgram->enableAttributeArray ("aPos");
    theProgram->setAttributeBuffer ("aPos", GL_FLOAT, 0, 3);

    myColorBuffer.bind();
    theProgram->enableAttributeArray ("aColor");
    theProgram->setAttributeBuffer ("aColor", GL_FLOAT, 0, 3);

    theProgram->release();
  myVao.release();

  myVerticeCount = aLinesVertices.size();

  myInitialized = true;
}

// =======================================================================
// function : Draw
// purpose  :
// =======================================================================
void JTVis_TrihedronGeometry::Draw (OpenGLFunctions* theOGL)
{
  Q_UNUSED (theOGL);
  if (!myInitialized) return;

  myVao.bind();
    glDrawArrays (GL_LINES, 0, myVerticeCount);
  myVao.release();
}
