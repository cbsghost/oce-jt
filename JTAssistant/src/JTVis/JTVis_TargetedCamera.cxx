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

#include "JTVis_TargetedCamera.hxx"

using namespace Eigen;

// =======================================================================
// function : JTVis_TargetedCamera
// purpose  :
// =======================================================================
JTVis_TargetedCamera::JTVis_TargetedCamera (const Vector3f& thePosition)
  :  myScale    (1.f),
     myDistance (1.f),
     myAspectRatio (1.f),
     myFieldOfView (45.f),
     myZNear (1.f),
     myZFar  (4000.f),
     myTarget (thePosition),
     myRotation (AngleAxisf::Identity()),
     myViewMatrixDirty (true),
     myProjMatrixDirty (true),
     myCameraMode (cmOrthographic)
{
  //
}

// =======================================================================
// function : JTVis_TargetedCamera
// purpose  :
// =======================================================================
JTVis_TargetedCamera::JTVis_TargetedCamera (const JTVis_TargetedCamera& theCamera)
  :  myScale    (theCamera.myScale),
     myDistance (theCamera.myDistance),
     myAspectRatio (theCamera.myAspectRatio),
     myFieldOfView (theCamera.myFieldOfView),
     myZNear (theCamera.myZNear),
     myZFar  (theCamera.myZFar),
     myTarget   (theCamera.myTarget),
     myRotation (theCamera.myRotation),
     myViewMatrixDirty (true),
     myProjMatrixDirty (true),
     myCameraMode (theCamera.myCameraMode)
{
  //
}

// =======================================================================
// function : Target
// purpose  :
// =======================================================================  
Vector3f JTVis_TargetedCamera::Target() const
{
  return myTarget;
}

// =======================================================================
// function : SetTarget
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::SetTarget (const Vector3f& theTarget)
{
  MarkViewMatrixDirty();
  myTarget = theTarget;
}

// =======================================================================
// function : Rotation
// purpose  :
// =======================================================================
AngleAxisf JTVis_TargetedCamera::Rotation() const
{
  return myRotation;
}

// =======================================================================
// function : SetRotation
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::SetRotation (const Vector3f& theRotation)
{
  MarkViewMatrixDirty();
  myRotation = AngleAxisf (theRotation.z(), Vector3f::UnitZ()) *
               AngleAxisf (theRotation.y(), Vector3f::UnitY()) *
               AngleAxisf (theRotation.x(), Vector3f::UnitX());
}

// =======================================================================
// function : SetRotation
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::SetRotation (const AngleAxisf& theRotation)
{
  MarkViewMatrixDirty();
  myRotation = theRotation;
}

// =======================================================================
// function : Rotate
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::Rotate (const float theAngleAroundX,
                                   const float theAngleAroundY)
{
  MarkViewMatrixDirty();
  myRotation = AngleAxisf (theAngleAroundX, Vector3f::UnitX()) *
               AngleAxisf (theAngleAroundY, Vector3f::UnitY()) *
               myRotation;
}

// =======================================================================
// function : Translate
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::Translate (const Vector3f& theDelta)
{
  MarkViewMatrixDirty();
  myTarget += theDelta;
}

// =======================================================================
// function : Scale
// purpose  :
// =======================================================================
float JTVis_TargetedCamera::Scale() const
{
  if (IsOrthographic())
  {
    return myScale;
  }
  else
  {
    return myDistance * tanf (myFieldOfView * 0.5f / 180.f * static_cast<float> (M_PI)) * 2.f;
  }
}

// =======================================================================
// function : SetScale
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::SetScale (const float theScale)
{
  MarkProjMatrixDirty();

  if (IsOrthographic())
  {
    myScale = theScale;
  }
  else
  {
    myDistance = theScale / (tanf (myFieldOfView * 0.5f / 180.f * static_cast<float> (M_PI)) * 2.f);
  }
}

// =======================================================================
// function : View
// purpose  :
// =======================================================================
Vector3f JTVis_TargetedCamera::View() const
{
  return (myRotation.inverse() * Vector3f::UnitZ()).normalized();
}

// =======================================================================
// function : Up
// purpose  :
// =======================================================================
Vector3f JTVis_TargetedCamera::Up() const
{
  return (myRotation.inverse() * Vector3f::UnitY()).normalized();
}

// =======================================================================
// function : Side
// purpose  :
// =======================================================================
Vector3f JTVis_TargetedCamera::Side() const
{
  return (myRotation.inverse() * Vector3f::UnitX()).normalized();
}

// =======================================================================
// function : ProjectionMatrix
// purpose  :
// =======================================================================
const Matrix4f& JTVis_TargetedCamera::ProjectionMatrix()
{
  if (myProjMatrixDirty)
  {
    Vector4f anProjWindow (-1.f * myAspectRatio,
                            1.f * myAspectRatio,
                           -1.f, 
                            1.f);

    if (IsOrthographic())
    {
      anProjWindow *= myScale * 0.5f;

      myProjMatrixCached = OrthographicProjection (anProjWindow.x(),
                                                   anProjWindow.y(),
                                                   anProjWindow.z(),
                                                   anProjWindow.w(),
                                                   myZNear,
                                                   myZFar);
    }
    else
    {
      anProjWindow *= myZNear * tanf (myFieldOfView * 0.5f / 180.f * static_cast<float> (M_PI));

      myProjMatrixCached = PerspectiveProjection (anProjWindow.x(),
                                                  anProjWindow.y(),
                                                  anProjWindow.z(),
                                                  anProjWindow.w(),
                                                  myZNear,
                                                  myZFar);
    }


    myProjMatrixDirty = false;
  }

  return myProjMatrixCached;
}

// =======================================================================
// function : ViewMatrix
// purpose  :
// =======================================================================
const Matrix4f& JTVis_TargetedCamera::ViewMatrix()
{
  if (myViewMatrixDirty)
  {
    Affine3f aView = Affine3f::Identity();
    Affine3f aTranslation = Affine3f::Identity();

    // set translation part - eye
    aTranslation.translation() = -EyePosition();

    // set linear part - rotation matrix
    aView.linear() = myRotation.matrix();

    myViewMatrixCached = aView.matrix() * aTranslation.matrix();
    myViewMatrixDirty = false;
  }

  return myViewMatrixCached;
}

// =======================================================================
// function : OrthographicProjection
// purpose  :
// =======================================================================
Matrix4f JTVis_TargetedCamera::OrthographicProjection (float theLeft, float theRight,
                                                       float theBottom, float theTop,
                                                       float theNearPlane, float theFarPlane) const
{
  Matrix4f aResult = Matrix4f::Identity();

  aResult (0, 0) =  2.f / (theRight - theLeft);
  aResult (1, 1) =  2.f / (theTop - theBottom);
  aResult (2, 2) = -2.f / (theFarPlane - theNearPlane);

  aResult (0, 3) = -(theRight + theLeft) / (theRight - theLeft);
  aResult (1, 3) = -(theTop + theBottom) / (theTop - theBottom);
  aResult (2, 3) = -(theFarPlane + theNearPlane) / (theFarPlane - theNearPlane);

  return aResult;
}

// =======================================================================
// function : PerspectiveProjection
// purpose  :
// =======================================================================
Matrix4f JTVis_TargetedCamera::PerspectiveProjection (float theLeft, float theRight,
                                                      float theBottom, float theTop,
                                                      float theNearPlane, float theFarPlane) const
{
  Matrix4f aResult = Matrix4f::Identity();

  aResult (0, 0) =  2.f * theNearPlane / (theRight - theLeft);

  aResult (1, 1) =  2.f * theNearPlane / (theTop - theBottom);

  aResult (0, 2) =  (theRight + theLeft) / (theRight - theLeft);
  aResult (1, 2) =  (theTop + theBottom) / (theTop - theBottom);
  aResult (2, 2) = -(theFarPlane + theNearPlane) / (theFarPlane - theNearPlane);
  aResult (3, 2) = -1.f;

  aResult (2, 3) = -(2.f * theFarPlane * theNearPlane) / (theFarPlane - theNearPlane);
  aResult (3, 3) =  0.f;

  return aResult;
}

// =======================================================================
// function : SetCameraMode
// purpose  :
// =======================================================================
void JTVis_TargetedCamera::SetCameraMode (JTVis_CameraMode theMode)
{
  if (theMode == myCameraMode)
    return;

  myCameraMode = theMode;

  if (theMode == cmPerspective)
  {
    myDistance = myScale / (tanf (myFieldOfView * 0.5f / 180.f * static_cast<float> (M_PI)) * 2.f);
  }
  else
  {
    myScale = myDistance * tanf (myFieldOfView * 0.5f / 180.f * static_cast<float> (M_PI)) * 2.f;

    if (myScale < 1e-4f)
      myScale = 1.f;
  }

  MarkViewMatrixDirty();
  MarkProjMatrixDirty();
}

// =======================================================================
// function : EyePosition
// purpose  :
// =======================================================================
Vector3f JTVis_TargetedCamera::EyePosition()
{
  return myTarget + View() * myDistance;
}
