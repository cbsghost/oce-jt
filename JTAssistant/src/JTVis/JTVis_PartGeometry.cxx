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

#include "JTVis_PartGeometry.hxx"

#include <Eigen/Core>
using namespace Eigen;

//=======================================================================
// function : JTVis_PartGeometry
// purpose  :
//=======================================================================
JTVis_PartGeometry::JTVis_PartGeometry ()
  : myInitialized (false),
    myVertexBuffer (QOpenGLBuffer::VertexBuffer),
    myNormalBuffer (QOpenGLBuffer::VertexBuffer),
    myIndexBuffer (QOpenGLBuffer::IndexBuffer),
    myIndicesCount (0),
    myAggregator (NULL),
    myStart (0),
    myEnd (0)
{
  //
}

//=======================================================================
// function : InitializeGeometry
// purpose  :
//=======================================================================
void JTVis_PartGeometry::InitializeGeometry (QOpenGLShaderProgram* theProgram,
                                             const JTCommon_TriangleDataPtr& theTriangulation)
{
  myVertexBuffer.create();
  myVertexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myVertexBuffer.bind())
  {
    qWarning() << "Could not bind vertex buffer to the context";
    return;
  }
  myVertexBuffer.allocate (theTriangulation->Data->Vertices().Data(),
                           theTriangulation->Data->Vertices().Count() * 3 * sizeof (float));
  myVertexBuffer.release();

  myIndicesCount = theTriangulation->Data->Indices().Count();

  myNormalBuffer.create();
  myNormalBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myNormalBuffer.bind())
  {
    qWarning() << "Could not bind normal buffer to the context";
    return;
  }
  myNormalBuffer.allocate (theTriangulation->Data->Normals().Data(),
                           theTriangulation->Data->Normals().Count() * 3 * sizeof (float));
  myNormalBuffer.release();

  myIndexBuffer.create();
  myIndexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myIndexBuffer.bind())
  {
    qWarning() << "Could not bind index buffer to the context";
    return;
  }
  myIndexBuffer.allocate (theTriangulation->Data->Indices().Data(),
                          theTriangulation->Data->Indices().Count() * sizeof (int));
  myIndexBuffer.release();

  myVao.create();
  myVao.bind();
  theProgram->bind();

  myVertexBuffer.bind();
  theProgram->enableAttributeArray ("aPosition");
  theProgram->setAttributeBuffer ("aPosition", GL_FLOAT, 0, 3);

  myNormalBuffer.bind();
  theProgram->enableAttributeArray ("aNormal");
  theProgram->setAttributeBuffer ("aNormal", GL_FLOAT, 0, 3);

  myVao.release();

  myInitialized = true;
}

//=======================================================================
// function : InitializeGeometry
// purpose  :
//=======================================================================
void JTVis_PartGeometry::InitializeGeometry (OpenGLFunctions* theOGL,
                                             const JTCommon_TriangleDataPtr& theTriangulation,
                                             JTVis_PartGeometryAggregator& theAggregator)
{
  myIndicesCount = theAggregator.AddMesh (theTriangulation, theOGL, &myIndexBuffer, myStart, myEnd);

  if (myIndicesCount > 0)
  {
    myInitialized = true;
    myAggregator  = &theAggregator;
  }
}


//=======================================================================
// function : Draw
// purpose  :
//=======================================================================
unsigned int JTVis_PartGeometry::Draw (OpenGLFunctions* theOGL, unsigned int theCurrentVao)
{
  Q_UNUSED (theOGL);

  if (!myInitialized) return 0xffffff;

  QOpenGLVertexArrayObject& aVao = myAggregator == NULL ? myVao : myAggregator->myVao;

  unsigned int aCurrentVao = 0;

  aCurrentVao = aVao.objectId();
  if (aCurrentVao != theCurrentVao)
    aVao.bind();

  myIndexBuffer.bind();
  if (myAggregator == NULL)
  {
    glDrawElements (GL_TRIANGLES, myIndicesCount, GL_UNSIGNED_INT, NULL);
  }
  else
  {
#ifndef QT_OPENGL_ES_2
    theOGL->glDrawRangeElements (GL_TRIANGLES, myStart, myEnd, myIndicesCount, GL_UNSIGNED_INT, NULL);
#else
    glDrawElements (GL_TRIANGLES, myIndicesCount, GL_UNSIGNED_INT, NULL);
#endif
  }

  return aCurrentVao;
}

//=======================================================================
// function : Draw
// purpose  :
//=======================================================================
void JTVis_PartGeometry::Draw (OpenGLFunctions* theOGL, QOpenGLShaderProgram* theProgram)
{
  Q_UNUSED (theOGL);

  QOpenGLVertexArrayObject& aVao = myAggregator == NULL ? myVao : myAggregator->myVao;

  aVao.release();

  QOpenGLBuffer& aVertBuffer = myAggregator == NULL ? myVertexBuffer : myAggregator->myVertexBuffer;
  QOpenGLBuffer& aNormBuffer = myAggregator == NULL ? myNormalBuffer : myAggregator->myNormalBuffer;

  aVertBuffer.bind();
  theProgram->enableAttributeArray ("aPosition");
  theProgram->setAttributeBuffer ("aPosition", GL_FLOAT, 0, 3);

  aNormBuffer.bind();
  theProgram->enableAttributeArray ("aNormal");
  theProgram->setAttributeBuffer ("aNormal", GL_FLOAT, 0, 3);

  myIndexBuffer.bind();
  if (myAggregator == NULL)
  {
    glDrawElements (GL_TRIANGLES, myIndicesCount, GL_UNSIGNED_INT, NULL);
  }
  else
  {
#ifndef QT_OPENGL_ES_2
    theOGL->glDrawRangeElements (GL_TRIANGLES, myStart, myEnd, myIndicesCount, GL_UNSIGNED_INT, NULL);
#else
    glDrawElements (GL_TRIANGLES, myIndicesCount, GL_UNSIGNED_INT, NULL);
#endif
  }

  theProgram->disableAttributeArray ("aNormal");
  theProgram->disableAttributeArray ("aPosition");
}

//=======================================================================
// function : JTVis_PartGeometryAggregator
// purpose  :
//=======================================================================
JTVis_PartGeometryAggregator::JTVis_PartGeometryAggregator()
  : myInitialized  (false),
    myVertexBuffer (QOpenGLBuffer::VertexBuffer),
    myNormalBuffer (QOpenGLBuffer::VertexBuffer),
    myVertexCount  (0),
    myMaxSize      (1)
{
  //
}

//=======================================================================
// function : Initialize
// purpose  :
//=======================================================================
void JTVis_PartGeometryAggregator::Initialize (QOpenGLShaderProgram* theProgram,
                                               Standard_Integer theMaxSize)
{
  myVertexCount  = 0;

  myMaxSize = static_cast<int>(theMaxSize);

  myVertexBuffer.create();
  myVertexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myVertexBuffer.bind())
  {
    qWarning() << "Could not bind vertex buffer to the context";
    return;
  }
  myVertexBuffer.allocate (static_cast<int>(theMaxSize * 3 * sizeof (float)));
  myVertexBuffer.release();

  myNormalBuffer.create();
  myNormalBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myNormalBuffer.bind())
  {
    qWarning() << "Could not bind normal buffer to the context";
    return;
  }
  myNormalBuffer.allocate (static_cast<int>(theMaxSize * 3 * sizeof (float)));
  myNormalBuffer.release();

  myVao.create();
  myVao.bind();
  theProgram->bind();

  myVertexBuffer.bind();
  theProgram->enableAttributeArray ("aPosition");
  theProgram->setAttributeBuffer ("aPosition", GL_FLOAT, 0, 3);

  myNormalBuffer.bind();
  theProgram->enableAttributeArray ("aNormal");
  theProgram->setAttributeBuffer ("aNormal", GL_FLOAT, 0, 3);

  myVao.release();

  myInitialized = true;
}

//=======================================================================
// function : AddMesh
// purpose  :
//=======================================================================
int JTVis_PartGeometryAggregator::AddMesh (const JTCommon_TriangleDataPtr& theTriangulation,
                                                          OpenGLFunctions* theOGL,
                                                            QOpenGLBuffer* theIndexBuffer,
                                                                      int& theStart,
                                                                      int& theEnd)
{
  if (!myInitialized ||
      myVertexCount + theTriangulation->Data->Vertices().Count() >= (int )myMaxSize)
  {
    return -1;
  }

  const float* aData;
  int aVertexCount;

  aData = theTriangulation->Data->Vertices().Data();
  aVertexCount = theTriangulation->Data->Vertices().Count();

  myVertexBuffer.bind();
  theOGL->glBufferSubData (GL_ARRAY_BUFFER, 
                           myVertexCount * 3 * sizeof (float),
                           aVertexCount * 3 * sizeof (float),
                           aData);

  aData = theTriangulation->Data->Normals().Data();

  myNormalBuffer.bind();
  theOGL->glBufferSubData (GL_ARRAY_BUFFER,
                           myVertexCount * 3 * sizeof (float),
                           aVertexCount * 3 * sizeof (float),
                           aData);

  theStart = myVertexCount;
  theEnd   = myVertexCount + aVertexCount - 1;

  std::vector<int> aCorrectedIndices (theTriangulation->Data->Indices().Count());
  for (int anIdx = 0; anIdx < theTriangulation->Data->Indices().Count(); ++anIdx)
  {
    int aValue = theTriangulation->Data->Indices() [anIdx] + theStart;

    aCorrectedIndices[anIdx] = aValue;
  }

  myVertexCount += aVertexCount;

  theIndexBuffer->create();
  theIndexBuffer->setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!theIndexBuffer->bind())
  {
    qWarning() << "Could not bind index buffer to the context";
    return -1;
  }
  theIndexBuffer->allocate (aCorrectedIndices.data(),
                            static_cast<int>(aCorrectedIndices.size() * sizeof (int)));
  theIndexBuffer->release();

  return static_cast<int>(aCorrectedIndices.size());
}
