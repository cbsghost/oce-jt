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

#include "JTVis_HudRenderer.hxx"
#include "JTVis_Scene.hxx"

#pragma warning (push, 0)
#include <QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions_2_0>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#pragma warning (pop)

using namespace Eigen;

// =======================================================================
// function : JTGui_HudRenderer
// purpose  :
// =======================================================================
JTVis_HudRenderer::JTVis_HudRenderer (const QSize& theViewport, QOpenGLShaderProgram* theShaderProgram)
  : myHudFbo (NULL),
    myHudMultisampledFbo (NULL),
    myPaintDevice (NULL),
    myTexQuadShaderProgram (theShaderProgram),
    myViewport (theViewport),
    myPosition (0, 0),
    isMultisampled (false)
{
  myHudQuad.reset (new JTVis_QuadGeometry());
  myHudQuad->InitializeGeometry (myTexQuadShaderProgram);

  myPaintDevice = new QOpenGLPaintDevice (myViewport.width(), myViewport.height());

  QOpenGLFramebufferObjectFormat anFboFormat;
#ifndef QT_OPENGL_ES_2
  anFboFormat.setInternalTextureFormat (GL_RGBA8);
#else
  anFboFormat.setInternalTextureFormat (GL_RGBA);
#endif
  anFboFormat.setSamples (0);
  anFboFormat.setAttachment (QOpenGLFramebufferObject::CombinedDepthStencil);
  myHudFbo = new QOpenGLFramebufferObject (myViewport.width(), myViewport.height(), anFboFormat);
  myHudFbo->bindDefault(); // solve issue with Qt GL integration on Linux.

  if (isMultisampled)
  {
    QOpenGLFramebufferObjectFormat aMultisampledFboFormat;

#ifndef QT_OPENGL_ES_2
    aMultisampledFboFormat.setInternalTextureFormat (GL_RGBA8);
#else
    aMultisampledFboFormat.setInternalTextureFormat (GL_RGBA);
#endif

    aMultisampledFboFormat.setSamples (16);
    aMultisampledFboFormat.setAttachment (QOpenGLFramebufferObject::CombinedDepthStencil);
    myHudMultisampledFbo = new QOpenGLFramebufferObject (myViewport.width(), myViewport.height(), aMultisampledFboFormat);
    myHudMultisampledFbo->bindDefault(); // solve issue with Qt GL integration on Linux.
  }
}

// =======================================================================
// function : ~JTGui_HudRenderer
// purpose  :
// =======================================================================
JTVis_HudRenderer::~JTVis_HudRenderer()
{
  delete myHudFbo;
  delete myHudMultisampledFbo;
  delete myPaintDevice;
}

// =======================================================================
// function : Update
// purpose  :
// =======================================================================
void JTVis_HudRenderer::Update (const JTVis_Stats& theStats, OpenGLFunctions* theGL)
{
  theGL->glUseProgram (0);
  theGL->glBindBuffer (GL_ARRAY_BUFFER, 0);
  theGL->glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

  theGL->glDisable (GL_DEPTH_TEST);

  const QRect drawRect (0, 0, myViewport.width(), myViewport.height());

  QOpenGLFramebufferObject* anFbo = isMultisampled ? myHudMultisampledFbo : myHudFbo;

  anFbo->bind();
  glClearColor (0.f, 0.f, 0.f, 0.f);
  theGL->glClear (GL_COLOR_BUFFER_BIT);

  {
    QPainter aPainter (myPaintDevice);
    aPainter.setRenderHints (QPainter::Antialiasing | QPainter::TextAntialiasing);

    aPainter.setPen (Qt::NoPen);
    aPainter.setBrush (QBrush (QColor (255, 255, 255, 170)));

    aPainter.drawRect (QRect (5, 5, 220, 145));

    int aViewCulledTris = theStats.FullTriangleCount - theStats.VisibleTriangleCount - theStats.SizeCulledTriangles;

    float aRatioOverall = theStats.FullTriangleCount == 0 ? 0 :
      100.f * (theStats.FullTriangleCount - theStats.VisibleTriangleCount) / theStats.FullTriangleCount;

    float aRatioSizeCulling = theStats.FullTriangleCount == 0 ? 0 :
      100.f * theStats.SizeCulledTriangles / theStats.FullTriangleCount;

    float aRatioViewCulling = theStats.FullTriangleCount == 0 ? 0 :
      100.f * aViewCulledTris / theStats.FullTriangleCount;

    QString aText = QString ("Visible items\n"
                             "Triangles: %1\n"
                             "Parts:     %2\n\n"
                             "%3% Overall culled\n"
                             "%4% View culling\n"
                             "%5% Size culling\n")
      .arg (theStats.VisibleTriangleCount)
      .arg (theStats.VisiblePartCount)
      .arg ((int ) Round (aRatioOverall))
      .arg ((int ) Round (aRatioViewCulling))
      .arg ((int ) Round (aRatioSizeCulling));

    aPainter.setPen (QPen (QColor (70, 70, 70, 200), 0));
    QFont aFont;
    aFont.setStyleHint (QFont::Monospace);
    aFont.setStyleStrategy (QFont::PreferBitmap);
    aFont.setFamily ("Courier New");
    aFont.setPointSize (11);
    aPainter.setFont (aFont);

    aPainter.drawText (QRect (15, 15, 220, 190), aText, QTextOption (Qt::AlignTop));

  }
  anFbo->release();

  theGL->glEnable (GL_DEPTH_TEST);
}

// =======================================================================
// function : Draw
// purpose  :
// =======================================================================
void JTVis_HudRenderer::Draw (OpenGLFunctions* theGL)
{
  theGL->glViewport (myPosition.x(), myPosition.y(), myViewport.width(), myViewport.height());
  theGL->glDisable (GL_DEPTH_TEST);
  theGL->glEnable (GL_TEXTURE_2D); 
  theGL->glEnable (GL_BLEND);

#ifndef QT_OPENGL_ES_2
  theGL->glAlphaFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
  theGL->glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

  if (isMultisampled)
    QOpenGLFramebufferObject::blitFramebuffer (myHudFbo, myHudMultisampledFbo);

  myTexQuadShaderProgram->bind();

  static Vector4f aTranslation = Vector4f (0.0f, 0.0f, 0.0f, 0.0f);

  myTexQuadShaderProgram->setUniformValue ("uColorTexture", 0);
  myTexQuadShaderProgram->setUniformValue ("uScale", 1.f);

  theGL->glBindTexture (GL_TEXTURE_2D, myHudFbo->texture());
  theGL->glUniform4fv (myTexQuadShaderProgram->uniformLocation ("uTranslation"), 1, aTranslation.data());

  myHudQuad->Draw();

  myTexQuadShaderProgram->release();

  theGL->glDisable (GL_TEXTURE_2D);
  theGL->glBindTexture (GL_TEXTURE_2D, 0);
  theGL->glDisable (GL_BLEND);
  theGL->glEnable (GL_DEPTH_TEST);
}
