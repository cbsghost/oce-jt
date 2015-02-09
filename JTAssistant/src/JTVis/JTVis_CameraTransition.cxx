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

#include "JTVis_CameraTransition.hxx"


// =======================================================================
// function : JTVis_CameraTransition
// purpose  :
// =======================================================================
JTVis_CameraTransition::JTVis_CameraTransition (const Eigen::AngleAxisf& theStartRotation,
                                                const Eigen::AngleAxisf& theEndRotation,
                                                const Eigen::Vector3f& theStartTarget,
                                                const Eigen::Vector3f& theEndTarget,
                                                const float theStartScale,
                                                const float theEndScale,
                                                const float theTimeLength,
                                                JTVis_AnimationModeCallbackPtr theAnimationCallback)
  : myStartRotation (Eigen::Quaternionf (theStartRotation)),
    myEndRotation (Eigen::Quaternionf (theEndRotation)),
    myStartTarget (theStartTarget),
    myEndTarget (theEndTarget),
    myStartScale (theStartScale),
    myEndScale (theEndScale),
    myStartTime (0.0),
    myTimeLength (theTimeLength),
    isFinished (true),
    myAnimationCallback (theAnimationCallback),
    myStarted (false)
{
  //
}

// =======================================================================
// function : JTVis_CameraTransition
// purpose  :
// =======================================================================
JTVis_CameraTransition::JTVis_CameraTransition (const JTVis_TargetedCamera& theCamera,
                                                const Eigen::AngleAxisf& theEndRotation,
                                                const Eigen::Vector3f& theEndTarget,
                                                const float theEndScale,
                                                const float theTimeLength,
                                                JTVis_AnimationModeCallbackPtr theAnimationCallback)
  : myStartRotation (Eigen::Quaternionf (theCamera.Rotation())),
    myEndRotation (Eigen::Quaternionf (theEndRotation)),
    myStartTarget (theCamera.Target()),
    myEndTarget (theEndTarget),
    myStartScale (theCamera.Scale()),
    myEndScale (theEndScale),
    myStartTime (0.0),
    myTimeLength (theTimeLength),
    isFinished (true),
    myAnimationCallback (theAnimationCallback),
    myStarted (false)
{
  //
}

// =======================================================================
// function : Apply
// purpose  :
// =======================================================================
void JTVis_CameraTransition::Apply (const float theTime, JTVis_TargetedCamera& theCamera)
{
  if (!myStarted)
  {
    myStarted = true;
    myStartTime = theTime;
  }

  if (theTime < myStartTime || theTime > myStartTime + myTimeLength)
  {
    isFinished = true;
    if (!myAnimationCallback.isNull())
      myAnimationCallback->Execute (false);
    theCamera.SetRotation (Eigen::AngleAxisf (myEndRotation));
    theCamera.SetTarget (myEndTarget);
    theCamera.SetScale (myEndScale);

    return;
  }

  float t = (theTime - myStartTime) / myTimeLength;
  t = sin (t * static_cast<float> (M_PI_2));
  theCamera.SetRotation (Eigen::AngleAxisf (myStartRotation.slerp (t, myEndRotation)));
  theCamera.SetTarget (myStartTarget * (1.f - t) + myEndTarget * t);
  theCamera.SetScale (myStartScale * (1.f - t) + myEndScale * t);
}
