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

#ifndef JTGUI_WINDOW_H
#define JTGUI_WINDOW_H

#pragma warning (push, 0)
#include <QWindow>
#include <QTime>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#pragma warning (pop)

#include <JTCommon_Utils.hxx>

class JTVis_Scene;
typedef QSharedPointer<JTVis_Scene> JTVis_ScenePtr;

class QOpenGLContext;

//! Mouse button bindings.
struct JTGui_MouseLayout
{
  static Qt::KeyboardModifier NoKey;

  JTGui_MouseLayout (Qt::MouseButton      theSelectionButton = Qt::LeftButton,
                     Qt::MouseButton      theRotateButton = Qt::LeftButton,
                     Qt::MouseButton      thePanButton = Qt::MiddleButton,
                     Qt::MouseButton      theZoomButton = Qt::RightButton,
                     Qt::KeyboardModifier theInteractKey = JTGui_MouseLayout::NoKey,
                     bool                 theZoomWithWheel = true)
    : SelectionButton (theSelectionButton),
      RotateButton    (theRotateButton),
      PanButton       (thePanButton),
      ZoomButton      (theZoomButton),
      InteractKey     (theInteractKey),
      ZoomWithWheel   (theZoomWithWheel)
  {}

  Qt::MouseButton      SelectionButton;
  Qt::MouseButton      RotateButton;
  Qt::MouseButton      PanButton;
  Qt::MouseButton      ZoomButton;
  Qt::KeyboardModifier InteractKey;

  bool ZoomWithWheel;
};

//! Enum for currently enabled view operation (for forced operations only).
enum JTGui_ViewOperation
{
  voRotate, voPan, voZoom, voNone
};

//! Window for 3d scene display.
class JTGui_Window : public QWindow
{
  Q_OBJECT

public:

  //! Creates window.
  JTGui_Window (QScreen* screen = 0);

  //! Returns associated scene.
  const JTVis_ScenePtr scene() { return myScene; }

  //! Sets 3d scene for window.
  void setScene (JTVis_ScenePtr theScene);

  //! Sets mouse layout struct.
  void setMouseLayout (const JTGui_MouseLayout& theMouseLayout);

  //! Returns reference to mouse layout struct.
  const JTGui_MouseLayout& mouseLayout() const { return myMouseLayout; }

  //! Sets forced view operation.
  void setViewOperation (const JTGui_ViewOperation theViewOperation)
  {
    myForcedViewOperation = theViewOperation;
  }

  //! Gets forced view operation.
  JTGui_ViewOperation viewOperation () const { return myForcedViewOperation; }

  //! Forces view clearing.
  void clearView();

public slots:

  //! Requests scene update.
  void needToUpdate() { myIsNeedToUpdate = true; }

  //! Enables animation mode.
  void setAnimationMode (bool isEnabled) { myIsAnimating = isEnabled; }

signals:

  //! Emits signal then context menu requested.
  void contextMenuRequested (const QPoint& thePoint, bool& isSucceed);

  //! Emits signal then forced operations need to be reset.
  void resetForcedOperations();

private slots:

  //! Handles any event.
  bool event (QEvent *theEvent);

  //! Handles window expose event.
  void exposeEvent (QExposeEvent *theEvent);

  //! Handles window resize event.
  void resizeEvent (QResizeEvent *theEvent);

  //! Handles window resize.
  void resizeGL();

  //! Posts update message to event loop.
  void renderLater();

  //! Does actual rendering;
  void renderNow();

  //! Draws scene.
  void paintGL();

  //! Paints clear background.
  void paintClearBackground();

  //! Posts update message if update requested.
  void checkUpdateState();

  //! Forces scene update.
  void forceUpdate();

  //! Updates scene.
  void updateScene();

  //! Check some OpenGL capabilities.
  void checkOpenGlCapabilities();

  //! Handle key press events.
  //! Made public since it need to be able to receive keyboard events translated from QTreeWidget.
  void keyPressEvent (QKeyEvent* theEvent);

  //! Handle key release events.
  void keyReleaseEvent (QKeyEvent* theEvent);

  //! Handle mouse button press events.
  void mousePressEvent (QMouseEvent* theEvent);

  //! Handle mouse button release events.
  void mouseReleaseEvent (QMouseEvent* theEvent);

  //! Handle mouse move event.
  void mouseMoveEvent (QMouseEvent* theEvent);

  //! Handle mouse wheel events.
  void wheelEvent (QWheelEvent* theEvent);

private:

  bool myUpdatePending;  //! Indicates when update is pending.
  bool myIsAnimating;    //! Indicates when animation mode enabled.
  bool myIsNeedToUpdate; //! Indicates when update requested.

  QSurfaceFormat myFormat; //! OpenGL format.

  QOpenGLContext* myContext;  //!< OpenGl context.
  JTVis_ScenePtr myScene;     //!< Associated 3d scene.
  QMap <int, bool> myButtonsPressed; //!< Indicates which buttons are pressed.
  QPoint myPrevPos;  //!< Previous mouse position.
  QPoint myPos;      //!< Current mouse position.
  QPoint myClickPos; //!< Mouse position at last click event.
  QTime myTime;      //!< Time helper object.
  int myMaxSamples;  //!< Max OpenGL MSAA samples.

  JTGui_MouseLayout myMouseLayout; //!< Mouse button bindings.

  JTGui_ViewOperation myForcedViewOperation; //!< Used when view forced to perform certain operation.
  bool myForcedOperationStarted; //!< Used to determine whether forced operation started.
};

//! Callback object to propagate animation mode request from 3d scene to renderwindow.
template <typename T>
class JTGui_AnimationModeCallback : public JTCommon_Callback<T>
{
  //
};

// Template specialization contains actual implementation of callback object.
template <>
class JTGui_AnimationModeCallback<bool> : public JTCommon_Callback<bool>
{
public:

  JTGui_AnimationModeCallback (JTGui_Window& theWindow)
    : myWindow (theWindow)
  {}

  void Execute (bool theParam)
  {
    myWindow.setAnimationMode (theParam);
    myWindow.needToUpdate();
  }

private:
  //closing of assigment operator and copy constructor
  JTGui_AnimationModeCallback& operator=(JTGui_AnimationModeCallback&);

  JTGui_AnimationModeCallback(JTGui_AnimationModeCallback&);

private:

  JTGui_Window& myWindow;

};

typedef QSharedPointer<JTGui_AnimationModeCallback<bool> > JTGui_AnimationModeCallbackPtr;

#endif // JTGUI_WINDOW_H
