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

#ifndef JTVIS_HUDRENDERER_HXX
#define JTVIS_HUDRENDERER_HXX

#include "JTVis_QuadGeometry.hxx"

struct JTVis_Stats;

#ifndef QT_OPENGL_ES_2
  class QOpenGLFunctions_2_0;
  typedef QOpenGLFunctions_2_0 OpenGLFunctions;
#else
  class QOpenGLFunctions_ES2;
  typedef QOpenGLFunctions_ES2 OpenGLFunctions;
#endif

class QOpenGLFramebufferObject;
class QOpenGLPaintDevice;
class QOpenGLShaderProgram;
class QPainter;

//! Helper class to render HUD.
class JTVis_HudRenderer
{
public:

  //! Initializes FBOs and other objects.
  JTVis_HudRenderer (const QSize& theViewport, QOpenGLShaderProgram* theShaderProgram);

  //! Frees objects managed manually.
  ~JTVis_HudRenderer();

  //! Updates offscreen HUD image.
  void Update (const JTVis_Stats& theStats, OpenGLFunctions* theGL);

  //! Draws prepared HUD texture to screen.
  void Draw (OpenGLFunctions* theGL);

  //! Sets current HUD position on screen.
  void SetPosition (QPoint thePosition) { myPosition = thePosition; }

  //! Returns current HUD position on screen.
  QPoint Position() { return myPosition; }

private:

  JTVis_HudRenderer (const JTVis_HudRenderer& );
  JTVis_HudRenderer& operator=(const JTVis_HudRenderer& );

private:

  QOpenGLFramebufferObject* myHudFbo;             //!< OpenGL Frame buffer object.
  QOpenGLFramebufferObject* myHudMultisampledFbo; //!< Multisampled OpenGL Frame buffer object.
  QOpenGLPaintDevice* myPaintDevice;              //!< Qt paint device suitable to rendering into FBO.
  JTVis_QuadGeometryPtr myHudQuad;                //!< Geometry object for HUD rendering.
  QOpenGLShaderProgram* myTexQuadShaderProgram;   //!< Shader program for drawing textured quad.

  QSize myViewport;  //!< Current viewport dimensions.
  QPoint myPosition; //!< Current HUD position on screen.

  bool isMultisampled; //!< Indicates if multisample buffer used.

};

typedef QSharedPointer<JTVis_HudRenderer> JTGui_HudRendererPtr;

#endif // JTVIS_HUDRENDERER_HXX
