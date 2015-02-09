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

#include "JTGui_Window.hxx"

#include <JTVis/JTVis_Scene.hxx>

#pragma warning (push, 0)
#include <QCoreApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QTimer>
#include <QMessageBox>
#pragma warning (pop)

Qt::KeyboardModifier JTGui_MouseLayout::NoKey = Qt::NoModifier;

// =======================================================================
// function : JTGui_Window
// purpose  :
// =======================================================================
JTGui_Window::JTGui_Window (QScreen* theScreen)
  : QWindow (theScreen),
    myUpdatePending (false),
    myIsAnimating (false),
    myIsNeedToUpdate (false),
    myContext (NULL),
    myMaxSamples (0),
    myForcedViewOperation (voNone),
    myForcedOperationStarted (false)
{
  checkOpenGlCapabilities ();

  // Tell Qt we will use OpenGL for this window
  setSurfaceType (OpenGLSurface);

  // Specify the format we wish to use
  myFormat.setDepthBufferSize (24);
  myFormat.setMajorVersion (2);
  myFormat.setMinorVersion (1);
  if (myMaxSamples >= 8)
  {
    myFormat.setSamples (8);
  }
  else if (myMaxSamples >= 4)
  {
    myFormat.setSamples (4);
  }

  myFormat.setRenderableType (QSurfaceFormat::OpenGL);
  myFormat.setProfile (QSurfaceFormat::NoProfile);

  setFormat (myFormat);

  myButtonsPressed.insert (Qt::LeftButton, false);
  myButtonsPressed.insert (Qt::MiddleButton, false);
  myButtonsPressed.insert (Qt::RightButton, false);
  myButtonsPressed.insert (Qt::Key_Control, false);

  // This timer drives the scene updates
  {
    QTimer* timer = new QTimer (this);
    connect (timer, SIGNAL (timeout()), this, SLOT (checkUpdateState()));
    timer->start (10);
  }

  // This timer drives forced scene updates with comparable large interval
  {
    QTimer* timer = new QTimer (this);
    connect (timer, SIGNAL (timeout()), this, SLOT (forceUpdate()));
    timer->start (250);
  }
}

// =======================================================================
// function : checkOpenGlCapabilities
// purpose  :
// =======================================================================
void JTGui_Window::checkOpenGlCapabilities()
{
  QSurfaceFormat aFormat;

  QWindow aTempWindow;
  aTempWindow.setSurfaceType (QWindow::OpenGLSurface);
  aTempWindow.setFormat (aFormat);
  aTempWindow.create();

  QOpenGLContext aTempContext;
  aTempContext.setFormat (aFormat);
  if (!aTempContext.create())
  {
    QMessageBox aMsgBox;
    aMsgBox.setText ("Error! Cannot create the OpenGL context!");
    aMsgBox.exec();
    exit (0);
  }

  aTempContext.makeCurrent (&aTempWindow);

  glGetIntegerv (GL_MAX_SAMPLES, &myMaxSamples);
}

// =======================================================================
// function : setScene
// purpose  :
// =======================================================================
void JTGui_Window::setScene (JTVis_ScenePtr theScene)
{
  myScene = theScene;

  if (!myScene.isNull())
  {
    myScene->SetContext (myContext);
    myScene->Resize (width(), height());
    myScene->SetMaxSamples (myMaxSamples);

    connect (myScene.data(), SIGNAL (RequestViewUpdate()),         this, SLOT (needToUpdate()));
    connect (myScene.data(), SIGNAL (RequestAnimationMode (bool)), this, SLOT (setAnimationMode (bool)));

    myScene->SetAnimationModeCallback(
      JTGui_AnimationModeCallbackPtr (new JTGui_AnimationModeCallback<bool> (*this)));
  }
}

// =======================================================================
// function : renderLater
// purpose  :
// =======================================================================
void JTGui_Window::renderLater()
{
  if (!myUpdatePending)
  {
    myUpdatePending = true;
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
  }
}

// =======================================================================
// function : event
// purpose  :
// =======================================================================
bool JTGui_Window::event (QEvent *event)
{
  switch (event->type())
  {
  case QEvent::UpdateRequest:
    {
      renderNow();
      return true;
    }
  default:
    return QWindow::event (event);
  }
}

// =======================================================================
// function : exposeEvent
// purpose  :
// =======================================================================
void JTGui_Window::exposeEvent (QExposeEvent *event)
{
  Q_UNUSED(event);

  if (isExposed())
    renderNow();
}

// =======================================================================
// function : resizeEvent
// purpose  :
// =======================================================================
void JTGui_Window::resizeEvent (QResizeEvent *event)
{
  Q_UNUSED(event);

  resizeGL();

  if (isExposed())
    renderNow();
}

// =======================================================================
// function : checkUpdateState
// purpose  :
// =======================================================================
void JTGui_Window::checkUpdateState()
{
  if (myIsNeedToUpdate)
  {
    renderLater();
    myIsNeedToUpdate = false;
  }
}

// =======================================================================
// function : forceUpdate
// purpose  :
// =======================================================================
void JTGui_Window::forceUpdate()
{
  if (myScene != NULL)
    renderLater();
}

// =======================================================================
// function : renderNow
// purpose  :
// =======================================================================
void JTGui_Window::renderNow()
{
  if (!isExposed())
  {
    return;
  }

  myUpdatePending = false;

  if (!myContext)
  {
    myContext = new QOpenGLContext (this);
    myContext->setFormat (myFormat);

    if (!myContext->create())
    {
      QMessageBox aMsgBox;
      aMsgBox.setText ("Error! Cannot create the OpenGL context!");
      aMsgBox.exec();
      exit (0);
    }

    myContext->makeCurrent (this);

    if (myContext->format().version().first < 2)
    {
      paintClearBackground();

      myContext->swapBuffers (this);

      QMessageBox aMsgBox;
      aMsgBox.setText (QString ("This application requires at least OpenGl 2.0 (OpenGl %1.%2 available).")
        .arg (myContext->format().version().first)
        .arg (myContext->format().version().second));
      aMsgBox.exec();
      exit (0);
    }

    QOpenGLShaderProgram aCheckShader;
    if (!aCheckShader.addShaderFromSourceCode (QOpenGLShader::Vertex,
           "void main() { gl_Position = gl_Vertex; }") ||
        !aCheckShader.addShaderFromSourceCode (QOpenGLShader::Fragment, 
           "void main() { gl_FragColor = vec4 (0.0, 0.0, 0.0, 0.0); }") ||
        !aCheckShader.link())
    {
      paintClearBackground();

      myContext->swapBuffers (this);

      QMessageBox aMsgBox;
      aMsgBox.setText ("This application requires OpenGL shader support (shader compilation / linking failed).");
      aMsgBox.exec();
      exit (0);
    }

    myTime.start();
  }

  myContext->makeCurrent (this);

  updateScene();
  paintGL();

  myContext->swapBuffers (this);

  if (myIsAnimating)
  {
    myIsNeedToUpdate = true;
  }
}

// =======================================================================
// function : paintGL
// purpose  :
// =======================================================================
void JTGui_Window::paintGL()
{
  if (myScene != NULL)
  {
    myScene->Render();
  }
  else
  {
    paintClearBackground();
  }
}

// =======================================================================
// function : paintClearBackground
// purpose  :
// =======================================================================
void JTGui_Window::paintClearBackground()
{
  glClearColor (0.5f, 0.5f, 0.5f, 1.0);
  glClear (GL_COLOR_BUFFER_BIT);
}

// =======================================================================
// function : resizeGL
// purpose  :
// =======================================================================
void JTGui_Window::resizeGL()
{
  if (myContext == NULL) return;

  myContext->makeCurrent (this);

  glViewport (0, 0, width(), height());

  if (myScene != NULL)
    myScene->Resize (width(), height());
}

// =======================================================================
// function : updateScene
// purpose  :
// =======================================================================
void JTGui_Window::updateScene()
{
  float time = myTime.elapsed() / 1000.0f;

  if (myScene != NULL)
    myScene->Update (time);
}

// =======================================================================
// function : keyPressEvent
// purpose  :
// =======================================================================
void JTGui_Window::keyPressEvent(QKeyEvent* e)
{
  if (myScene == NULL) return;

  switch (e->key())
  {
    case Qt::Key_Control:
      myButtonsPressed[Qt::Key_Control] = true;
      break;

    default:
      QWindow::keyPressEvent (e);
  }
}

// =======================================================================
// function : keyReleaseEvent
// purpose  :
// =======================================================================
void JTGui_Window::keyReleaseEvent (QKeyEvent* e)
{
  if (myScene == NULL) return;

  switch (e->key())
  {
    case Qt::Key_Control:
      myButtonsPressed[Qt::Key_Control] = false;
      break;

    default:
      QWindow::keyReleaseEvent (e);
  }

  myScene->DebugKeyHandler (e->key());
}

// =======================================================================
// function : mousePressEvent
// purpose  :
// =======================================================================
void JTGui_Window::mousePressEvent (QMouseEvent* e)
{
  if (myScene == NULL) return;

  myButtonsPressed[e->button()] = true;
  myPos = myClickPos = myPrevPos = e->pos();

  if (myForcedViewOperation != voNone)
  {
    myForcedOperationStarted = true;
    if (myForcedViewOperation == voRotate)
    {
      myScene->StartRotation();
    }
  }

  if (e->button() == myMouseLayout.RotateButton)
  {
    myScene->StartRotation();
  }

  QWindow::mousePressEvent (e);
}

// =======================================================================
// function : mouseReleaseEvent
// purpose  :
// =======================================================================
void JTGui_Window::mouseReleaseEvent (QMouseEvent* e)
{
  if (myScene == NULL) return;

  myButtonsPressed[e->button()] = false;

  if (e->button() == myMouseLayout.SelectionButton)
  {
    QPoint aDelta = e->pos() - myClickPos;
    if (qAbs (aDelta.x()) < 2 && qAbs (aDelta.y()) < 2)
      myScene->SelectMesh (myButtonsPressed[Qt::Key_Control]);
  }

  myForcedOperationStarted = false;

  if (e->button() == Qt::RightButton)
  {
    bool isMenuShowed = false;
    if (myMouseLayout.InteractKey == Qt::NoModifier || !e->modifiers().testFlag (myMouseLayout.InteractKey))
    {
      emit contextMenuRequested (e->pos(), isMenuShowed);
    }

    if (isMenuShowed)
    {
      myButtonsPressed[Qt::LeftButton]   = false;
      myButtonsPressed[Qt::MiddleButton] = false;
      myButtonsPressed[Qt::RightButton]  = false;
      myButtonsPressed[Qt::Key_Control]  = false;
    }
  }

  QWindow::mouseReleaseEvent (e);
}

namespace {
  struct MouseOperatonHelper
  {
    MouseOperatonHelper (JTVis_Scene& theScene, const JTGui_Window& theWindow)
      : myScene (theScene),
        myWindow (theWindow)
    {}

    void rotate (const QPoint& theDelta)
    {
      float drx = 5e-3f * theDelta.x();
      float dry = 5e-3f * theDelta.y();
      myScene.Rotate (drx, dry);
    }

    void pan (const QPoint& theDelta)
    {
      float dx = - 1.f / myWindow.width()  * theDelta.x();
      float dy =   1.f / myWindow.height() * theDelta.y();
      myScene.Pan (dx, dy);
    }

    void zoom (const QPoint& theDelta)
    {
      float dx = - 1.f / myWindow.width()  * theDelta.x();
      float dy =   1.f / myWindow.height() * theDelta.y();
      myScene.Zoom ((dx + dy) * 5e3f);
    }

  private:
    MouseOperatonHelper(MouseOperatonHelper&);
    MouseOperatonHelper& operator=(MouseOperatonHelper&);

  private:

    JTVis_Scene& myScene;
    const JTGui_Window&  myWindow;
  };
}

// =======================================================================
// function : mouseMoveEvent
// purpose  :
// =======================================================================
void JTGui_Window::mouseMoveEvent (QMouseEvent* e)
{
  if (myScene == NULL) return;

  myScene->SetMousePosition (QPoint(e->pos().x(), height() - e->pos().y()));

  myPos = e->pos();
  QPoint aDeltaPos = myPos - myPrevPos;

  MouseOperatonHelper aHelper (*myScene, *this);

  if (myForcedOperationStarted)
  {
    switch (myForcedViewOperation)
    {
    case voRotate:
      aHelper.rotate (myPos - myClickPos);
      break;
    case voPan:
      aHelper.pan (aDeltaPos);
      break;
    case voZoom:
      aHelper.zoom (aDeltaPos);
      break;
    case voNone:
      break;
    }

    myIsNeedToUpdate = true;
  }
  else
  {
    if (e->modifiers().testFlag (myMouseLayout.InteractKey))
    {
      if (myButtonsPressed[myMouseLayout.RotateButton])
      {
        aHelper.rotate (myPos - myClickPos);
        myIsNeedToUpdate = true;
      }

      if (myButtonsPressed[myMouseLayout.PanButton])
      {
        aHelper.pan (aDeltaPos);
        myIsNeedToUpdate = true;
      }

      if (!myMouseLayout.ZoomWithWheel && myButtonsPressed[myMouseLayout.ZoomButton])
      {
        aHelper.zoom (aDeltaPos);
        myIsNeedToUpdate = true;
      }
    }
  }

  myPrevPos = myPos;

  QWindow::mouseMoveEvent (e);
}

// =======================================================================
// function : wheelEvent
// purpose  :
// =======================================================================
void JTGui_Window::wheelEvent (QWheelEvent* e)
{
  if (myScene == NULL) return;

  if (myMouseLayout.ZoomWithWheel)
  {
    myScene->Zoom (-(e->delta()) * 0.8f);
    myIsNeedToUpdate = true;
  }
}

// =======================================================================
// function : setMouseLayout
// purpose  :
// =======================================================================
void JTGui_Window::setMouseLayout (const JTGui_MouseLayout& theMouseLayout)
{
  myMouseLayout = theMouseLayout;

  myButtonsPressed[Qt::LeftButton]   = false;
  myButtonsPressed[Qt::MiddleButton] = false;
  myButtonsPressed[Qt::RightButton]  = false;
  myButtonsPressed[Qt::Key_Control]  = false;
}

// =======================================================================
// function : clearView
// purpose  :
// =======================================================================
void JTGui_Window::clearView()
{
  glClear (GL_COLOR_BUFFER_BIT);
}
