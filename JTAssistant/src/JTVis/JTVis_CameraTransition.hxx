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

#ifndef JTVIS_CAMERA_TRANSITION
#define JTVIS_CAMERA_TRANSITION

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <JTCommon_Utils.hxx>

#include "JTVis_TargetedCamera.hxx"

typedef QSharedPointer<JTCommon_Callback<bool> > JTVis_AnimationModeCallbackPtr;

//! Helper class for camera animation.
class JTVis_CameraTransition
{
public:

  //! Initializes camera transition object in 'finished' state.
  JTVis_CameraTransition ()
    : myStartScale (0.0),
      myEndScale (0.0),
      myStartTime (0.0),
      myTimeLength (0.0),
      isFinished (true),
      myStarted (false)
  {}

  //! Initializes camera transition object with given start and final transforms.
  JTVis_CameraTransition (const Eigen::AngleAxisf& theStartRotation,
                          const Eigen::AngleAxisf& theEndRotation,
                          const Eigen::Vector3f& theStartTarget,
                          const Eigen::Vector3f& theEndTarget,
                          const float theStartScale,
                          const float theEndScale,
                          const float theTimeLength,
                          JTVis_AnimationModeCallbackPtr theAnimationCallback);

  //! Initializes camera transition object with start transform from theCamera
  //! and given final transform.
  JTVis_CameraTransition (const JTVis_TargetedCamera& theCamera,
                          const Eigen::AngleAxisf& theEndRotation,
                          const Eigen::Vector3f& theEndTarget,
                          const float theEndScale,
                          const float theTimeLength,
                          JTVis_AnimationModeCallbackPtr theAnimationCallback);

  //! Linearly interpolates theCamera transform between start and final transform.
  void Apply (const float theTime, JTVis_TargetedCamera& theCamera);

  //! Sets camera transition state to 'started'.
  void Start()
  {
    isFinished = false;
    myAnimationCallback->Execute (true);
  }

  //! Returns true if camera transition object is in 'finished' state.
  bool IsFinished() { return isFinished; }

private:

  Eigen::Quaternionf myStartRotation; //!< Rotation component of start transformation.
  Eigen::Quaternionf myEndRotation;   //!< Rotation component of final transformation.

  Eigen::Vector3f myStartTarget; //!< Translation component of start transformation.
  Eigen::Vector3f myEndTarget;   //!< Translation component of final transformation.

  // Note: this scale corresponds to Camera.Scale(),
  // not to scale component of Camera.ViewMatrix().

  float myStartScale; //!< Scale component of start transformation.
  float myEndScale;   //!< Scale component of final transformation.

  float myStartTime;  //!< Start time of animation. 
  float myTimeLength; //!< Animation length.
  bool  isFinished;   //!< True represents 'finished' state, false -- 'started' state.

  JTVis_AnimationModeCallbackPtr myAnimationCallback; //!< Callback for viewer update.

  bool myStarted;
};

#endif // JTVIS_CAMERA_TRANSITION
