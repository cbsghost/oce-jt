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

#include "JTVis_AABBGeometry.hxx"

using namespace Eigen;

// =======================================================================
// function : JTVis_AABBGeometry
// purpose  :
// =======================================================================
JTVis_AABBGeometry::JTVis_AABBGeometry (const JTCommon_AABB& theAABB)
  : myInitialized (false),
    myVertexBuffer (QOpenGLBuffer::VertexBuffer),
    myVerticeCount(0),
    myAABB (theAABB)
{
}

// =======================================================================
// function : InitializeGeometry
// purpose  :
// =======================================================================
void JTVis_AABBGeometry::InitializeGeometry (QOpenGLShaderProgram* theProgram)
{
  // bounds lines
  QVector<Vector4f> aLinesVertices;

  const Vector4f& aMin = myAABB.CornerMin();
  const Vector4f adX = Vector4f (myAABB.Size().x(), 0.f, 0.f, 0.f);
  const Vector4f adY = Vector4f (0.f, myAABB.Size().y(), 0.f, 0.f);
  const Vector4f adZ = Vector4f (0.f, 0.f, myAABB.Size().z(), 0.f);

  Vector4f anAxisShifts[] = {adX, adY, adZ};
  float aLookup1[] = {0.0f, 1.0f, 0.0f, 1.0f};
  float aLookup2[] = {0.0f, 0.0f, 1.0f, 1.0f};

  for (int anAxis = 0; anAxis < 3; ++anAxis)
    for (int j = 0; j < 4; ++j)
    {
      aLinesVertices.push_back (aMin +
        anAxisShifts[(anAxis + 1) % 3] * aLookup1[j] +
        anAxisShifts[(anAxis + 2) % 3] * aLookup2[j]);

      aLinesVertices.push_back (aMin + anAxisShifts[anAxis] +
        anAxisShifts[(anAxis + 1) % 3] * aLookup1[j] +
        anAxisShifts[(anAxis + 2) % 3] * aLookup2[j]);
    }

  myVertexBuffer.create();
  myVertexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myVertexBuffer.bind())
  {
    qWarning() << "Could not bind vertex buffer to the context";
    return;
  }
  myVertexBuffer.allocate (
      reinterpret_cast<float*> (aLinesVertices.data()),
      aLinesVertices.size() * 4 * sizeof (float));
  myVertexBuffer.release();

  myVao.create();
  myVao.bind();
    theProgram->bind();

    myVertexBuffer.bind();
    theProgram->enableAttributeArray ("aPos");
    theProgram->setAttributeBuffer ("aPos", GL_FLOAT, 0, 4);

    theProgram->release();
  myVao.release();

  myVerticeCount = aLinesVertices.size();

  myInitialized = true;
}

// =======================================================================
// function : Draw
// purpose  :
// =======================================================================
void JTVis_AABBGeometry::Draw (OpenGLFunctions* theOGL)
{
  Q_UNUSED (theOGL);
  if (!myInitialized) return;

  myVao.bind();
    glDrawArrays (GL_LINES, 0, myVerticeCount);
  myVao.release();
}
