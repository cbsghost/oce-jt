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

#include "JTVis_Frustum.hxx"

#include <Eigen/Geometry>

using namespace Eigen;

namespace
{
  const int THE_FRUSTUM_VERTICES = 24;
}

// =======================================================================
// function : JTVis_Frustum
// purpose  :
// =======================================================================
JTVis_Frustum::JTVis_Frustum (const JTCommon_AABB& theAABB)
{
  const Vector4f& aMin = theAABB.CornerMin();
  const Vector4f anXUnit = Vector4f (1.f, 0.f, 0.f, 0.f);
  const Vector4f anYUnit = Vector4f (0.f, 1.f, 0.f, 0.f);
  const Vector4f  aZUnit = Vector4f (0.f, 0.f, 1.f, 0.f);

  const Vector4f aSize = theAABB.Size();

  Planes[0] = JTVis_Plane (aMin, -anYUnit);
  Planes[1] = JTVis_Plane (aMin + anYUnit.cwiseProduct (aSize), anYUnit);

  Planes[2] = JTVis_Plane (aMin, -anXUnit);
  Planes[3] = JTVis_Plane (aMin + anXUnit.cwiseProduct (aSize), anXUnit);

  Planes[4] = JTVis_Plane (aMin, -aZUnit);
  Planes[5] = JTVis_Plane (aMin + aZUnit.cwiseProduct (aSize), aZUnit);

  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      for (int k = 0; k < 2; ++k)
      {
        Points[i*2*2 + j*2 + k] = aMin
            + anXUnit.cwiseProduct (aSize) * static_cast<float> (i)
            + anYUnit.cwiseProduct (aSize) * static_cast<float> (j)
            + aZUnit.cwiseProduct  (aSize) * static_cast<float> (k);
      }
}

// =======================================================================
// function : JTVis_Frustum
// purpose  :
// =======================================================================
JTVis_Frustum::JTVis_Frustum (const Eigen::Matrix4f& theMatrix)
{
  JTCommon_AABB aNormalizedBox (Vector4f (-1.f, -1.f, -1.f,  1.f),
                                Vector4f ( 1.f,  1.f,  1.f,  1.f));

  JTVis_Frustum aFrustum (aNormalizedBox);

  for (int i = 0; i < 8; ++i)
  {
    Points[i] = theMatrix * aFrustum.Points[i];
    Points[i] /= Points[i].w();
  }
}

// =======================================================================
// function : Intersect
// purpose  :
// =======================================================================
bool JTVis_Frustum::Intersect (const Vector4f& theMinPoint,
                               const Vector4f& theMaxPoint,
                               const bool      isOrthographic) const
{
  JTVis_Projection aFrustumProj = MainProjections[0]; // Unit X

  if (theMinPoint.x() > aFrustumProj.Max || theMaxPoint.x() < aFrustumProj.Min)
  {
    return false;
  }

  aFrustumProj = MainProjections[1]; // Unit Y

  if (theMinPoint.y() > aFrustumProj.Max || theMaxPoint.y() < aFrustumProj.Min)
  {
    return false;
  }

  aFrustumProj = MainProjections[2]; // Unit Z

  if (theMinPoint.z() > aFrustumProj.Max || theMaxPoint.z() < aFrustumProj.Min)
  {
    return false;
  }

  int aPlaneStep = isOrthographic ? 2 : 1;
  for (Standard_Integer anIdx = 0; anIdx < 5; anIdx += aPlaneStep) // Frustum normals
  {
    Vector4f anAxis = Planes[anIdx].Normal;

    const Vector4f aPVertex (anAxis.x() > 0.f ? theMaxPoint.x() : theMinPoint.x(),
                             anAxis.y() > 0.f ? theMaxPoint.y() : theMinPoint.y(),
                             anAxis.z() > 0.f ? theMaxPoint.z() : theMinPoint.z(),
                             0.f);

    aFrustumProj = FrustumProjections[anIdx];

    const Standard_ShortReal aPnt0 = aPVertex.dot (anAxis);

    if (aPnt0 >= aFrustumProj.Min && aPnt0 <= aFrustumProj.Max)
      continue;

    const Vector4f aNVertex (anAxis.x() > 0.f ? theMinPoint.x() : theMaxPoint.x(),
                             anAxis.y() > 0.f ? theMinPoint.y() : theMaxPoint.y(),
                             anAxis.z() > 0.f ? theMinPoint.z() : theMaxPoint.z(),
                             0.f);

    const Standard_ShortReal aPnt1 = aNVertex.dot (anAxis);

    const Standard_ShortReal aMin = aPnt0 < aPnt1 ? aPnt0 : aPnt1;
    const Standard_ShortReal aMax = aPnt0 > aPnt1 ? aPnt0 : aPnt1;

    if (aFrustumProj.Max < aMin || aFrustumProj.Min > aMax)
    {
      return false;
    }
  }

  return true;
}

// =======================================================================
// function : UpdatePlanes
// purpose  :
// =======================================================================
void JTVis_Frustum::UpdatePlanes()
{
  Vector4f aTempPoints[3];
  int aShifts[]  = { 0, 0, 0 };
  int aLookup1[] = { 0, 1, 0, 1 };
  int aLookup2[] = { 0, 0, 1, 1 };

  // Planes go in order:
  // -X, +X, -Y, +Y, -Z, +Z
  // (in local frustum space)

  for (Standard_Integer anAxis = 0; anAxis < 3; ++anAxis)
  {
    for (Standard_Integer i = 0; i < 2; ++i)
    {
      for (Standard_Integer j = 0; j < 3; ++j)
      {
        aShifts[anAxis] = i;
        aShifts[(anAxis + 1) % 3] = aLookup1[j];
        aShifts[(anAxis + 2) % 3] = aLookup2[j];

        aTempPoints[j] = Points[aShifts[0]*2*2 + aShifts[1]*2 + aShifts[2]];
      }

      Planes[anAxis*2 + i].Point = aTempPoints[0];
      Planes[anAxis*2 + i].Normal =
        (aTempPoints[1] - aTempPoints[0]).cross3
        (aTempPoints[2] - aTempPoints[0]).normalized() * (i == 0 ? -1.f : 1.f);
    }
  }

  Vector4f anAxes[] = { Vector4f (1.f, 0.f, 0.f, 0.f),
                        Vector4f (0.f, 1.f, 0.f, 0.f),
                        Vector4f (0.f, 0.f, 1.f, 0.f) };

  for (int anAxis = 0; anAxis < 3; ++anAxis)
  {
    MainProjections[anAxis] = JTVis_Projection();

    // project frustum onto main axes
    for (int aVertex = 0; aVertex < 8; ++aVertex)
    {
      float aProjPoint = Points[aVertex].dot (anAxes[anAxis]);

      MainProjections[anAxis].Min = qMin (MainProjections[anAxis].Min, aProjPoint);
      MainProjections[anAxis].Max = qMax (MainProjections[anAxis].Max, aProjPoint);
    }
  }

  for (int anAxis = 0; anAxis < 6; ++anAxis)
  {
    FrustumProjections[anAxis] = JTVis_Projection();

    // project frustum onto its own normals
    for (int aVertex = 0; aVertex < 8; ++aVertex)
    {
      float aProjPoint = Points[aVertex].dot (Planes[anAxis].Normal);

      FrustumProjections[anAxis].Min = qMin (FrustumProjections[anAxis].Min, aProjPoint);
      FrustumProjections[anAxis].Max = qMax (FrustumProjections[anAxis].Max, aProjPoint);
    }
  }
}

// =======================================================================
// function : JTVis_FrustumGeometry
// purpose  :
// =======================================================================
JTVis_FrustumGeometry::JTVis_FrustumGeometry (const JTVis_Frustum& theFrustum)
  : myInitialized (false),
    myVertexBuffer (QOpenGLBuffer::VertexBuffer),
    myVerticeCount (THE_FRUSTUM_VERTICES),
    myFrustum (theFrustum)
{
}

// =======================================================================
// function : InitializeGeometry
// purpose  :
// =======================================================================
void JTVis_FrustumGeometry::InitializeGeometry (QOpenGLShaderProgram* theProgram)
{
  Vector4f aLinesVertices[THE_FRUSTUM_VERTICES];

  int aShifts[] = {0, 0, 0};
  int aLookup1[] = {0, 1, 0, 1};
  int aLookup2[] = {0, 0, 1, 1};

  int aVertex = 0;
  for (int anAxis = 0; anAxis < 3; ++anAxis)
    for (int j = 0; j < 4; ++j)
    {
      aShifts[anAxis] = 0;
      aShifts[(anAxis + 1) % 3] = aLookup1[j];
      aShifts[(anAxis + 2) % 3] = aLookup2[j];
      aLinesVertices[aVertex++] = myFrustum.Points[aShifts[0]*2*2 + aShifts[1]*2 + aShifts[2]];

      aShifts[anAxis] = 1;
      aLinesVertices[aVertex++] = myFrustum.Points[aShifts[0]*2*2 + aShifts[1]*2 + aShifts[2]];
    }

  myVertexBuffer.create();
  myVertexBuffer.setUsagePattern (QOpenGLBuffer::StaticDraw);
  if (!myVertexBuffer.bind())
  {
    qWarning() << "Could not bind vertex buffer to the context";
    return;
  }
  myVertexBuffer.allocate (aLinesVertices[0].data(), THE_FRUSTUM_VERTICES * 4 * sizeof (float));
  myVertexBuffer.release();

  myVao.create();
  myVao.bind();
    theProgram->bind();

    myVertexBuffer.bind();
    theProgram->enableAttributeArray ("aPos");
    theProgram->setAttributeBuffer ("aPos", GL_FLOAT, 0, 4);

    theProgram->release();
  myVao.release();

  myInitialized = true;
}

// =======================================================================
// function : Draw
// purpose  :
// =======================================================================
void JTVis_FrustumGeometry::Draw (OpenGLFunctions* theOGL)
{
  Q_UNUSED (theOGL);
  if (!myInitialized) return;

  myVao.bind();
    glDrawArrays (GL_LINES, 0, myVerticeCount);
  myVao.release();
}
