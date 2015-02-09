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

#ifndef JTVIS_TARGETED_CAMERA
#define JTVIS_TARGETED_CAMERA

#include <Eigen/Core>
#include <Eigen/Geometry>

#pragma warning (push, 0)
#include <QSharedPointer>
#pragma warning (pop)

enum JTVis_CameraMode
{
  cmOrthographic,
  cmPerspective
};

//! Targeted camera class (to rotate the camera in local/view space).
class JTVis_TargetedCamera
{
public:

  //! Creates new camera.
  JTVis_TargetedCamera (const Eigen::Vector3f& thePosition = Eigen::Vector3f::Zero());

  //! Creates copy of another camera.
  JTVis_TargetedCamera (const JTVis_TargetedCamera& theCamera);

public:

  //! Returns camera position.
  Eigen::Vector3f Target() const;

  //! Sets camera position.
  void SetTarget (const Eigen::Vector3f& theTarget);

  //! Returns camera rotation (using angle-axis).
  Eigen::AngleAxisf Rotation() const;

  //! Sets camera rotation (using Euler angles).
  void SetRotation (const Eigen::Vector3f& theRotation);

  //! Sets camera rotation (using angle-axis).
  void SetRotation (const Eigen::AngleAxisf& theRotation);

  //! Rotates the camera in local space.
  void Rotate (const float theAngleAroundX,
               const float theAngleAroundY);

  //! Translates the camera in local space.
  void Translate (const Eigen::Vector3f& theDelta);

  //! Returns camera zoom factor.
  float Scale() const;

  //! Sets camera zoom factor.
  void SetScale (const float theScale);

  //! Returns camera eye position.
  Eigen::Vector3f EyePosition();

  //! Returns distance from camera target to eye.
  float Distance() const
  {
    return myDistance;
  }

  //! Sets distance from camera target to eye.
  void SetDistance (const float theDistance)
  {
    myDistance = theDistance; MarkViewMatrixDirty();
  }

  //! Returns camera aspect ratio.
  float AspectRatio () const
  {
    return myAspectRatio;
  }

  //! Sets camera aspect ratio.
  void SetAspectRatio (const float theAspectRatio)
  {
    myAspectRatio = theAspectRatio; MarkProjMatrixDirty();
  }

  //! Returns camera z-near value.
  float ZNear () const
  {
    return myZNear;
  }

  //! Sets camera z-near value.
  void SetZNear (const float theZNear)
  {
    myZNear = theZNear; MarkProjMatrixDirty();
  }

  //! Returns camera z-far value.
  float ZFar () const
  {
    return myZFar;
  }

  //! Sets camera z-far value.
  void SetZFar (const float theZFar)
  {
    myZFar = theZFar; MarkProjMatrixDirty();
  }

  //! Returns camera FOV value.
  float FieldOfView() { return myFieldOfView; }

  //! Sets camera FOV value.
  void SetFieldOfView (float theFOV) { myFieldOfView = theFOV; }

  //! Returns side vector of the camera.
  Eigen::Vector3f Side() const;

  //! Returns up vector of the camera.
  Eigen::Vector3f Up() const;

  //! Returns view vector of the camera.
  Eigen::Vector3f View() const;

  //! Returns model-view matrix of the camera.
  const Eigen::Matrix4f& ViewMatrix();

  //! Returns projection matrix of the camera.
  const Eigen::Matrix4f& ProjectionMatrix();

  //! Force camera to recalculate View matrix on next request.
  void MarkViewMatrixDirty() { myViewMatrixDirty = true; }

  //! Force camera to recalculate Projection matrix on next request.
  void MarkProjMatrixDirty() { myProjMatrixDirty = true; }

  //! Returns true whenever camera is orthographic.
  bool IsOrthographic() const { return myCameraMode == cmOrthographic; }

  //! Sets camera projection mode.
  void SetCameraMode (JTVis_CameraMode theMode);

private:

  //! Returns orthographic projection matrix.
  Eigen::Matrix4f OrthographicProjection (float theLeft, float theRight,
                                          float theBottom, float theTop,
                                          float theNearPlane, float theFarPlane) const;

  //! Returns perspective projection matrix.
  Eigen::Matrix4f PerspectiveProjection (float theLeft, float theRight,
                                         float theBottom, float theTop,
                                         float theNearPlane, float theFarPlane) const;

private:

  float myScale;       //!< Zooming factor.
  float myDistance;    //!< Distance from camera target to eye.
  float myAspectRatio; //!< Aspect ratio (width / height).
  float myFieldOfView; //!< FOV value for perspective camera.
  float myZNear;       //!< Near clipping value.
  float myZFar;        //!< Far clipping value.

  Eigen::Vector3f myTarget;           //!< Camera position.
  Eigen::AngleAxisf myRotation;       //!< Rotation of the camera.

  Eigen::Matrix4f myViewMatrixCached; //!< Cached View matrix.
  Eigen::Matrix4f myProjMatrixCached; //!< Cached Projection matrix.

  bool myViewMatrixDirty;             //!< Indicates camera need to recalculate View matrix on next request.
  bool myProjMatrixDirty;             //!< Indicates camera need to recalculate Projection matrix on next request.

  JTVis_CameraMode myCameraMode;      //!< Switches between orthographic and perspective projections.

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTVis_TargetedCamera> JTVis_TargetedCameraPtr;

#endif // JTVIS_TARGETED_CAMERA
