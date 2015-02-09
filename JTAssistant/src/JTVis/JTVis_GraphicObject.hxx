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

#ifndef JTVIS_GRAPHICOBJECT_H
#define JTVIS_GRAPHICOBJECT_H

#pragma warning (push, 0)
#include <QSharedPointer>
#include <qvector4d.h>

#ifndef QT_OPENGL_ES_2
  #include <QtGui/QOpenGLFunctions_2_0>
  typedef QOpenGLFunctions_2_0 OpenGLFunctions;
#else
  #include <QtGui/QOpenGLFunctions_ES2>
  typedef QOpenGLFunctions_ES2 OpenGLFunctions;
#endif
#pragma warning (pop)

//! Abstract graphics object to render with OpenGL.
class JTVis_GraphicObject
{
public:
  virtual ~JTVis_GraphicObject() {}

  virtual void InitializeGeometry (QOpenGLShaderProgram* theProgram) = 0;

  virtual void Draw (OpenGLFunctions* theOGL) = 0;

  QVector4D Color;
};

typedef QSharedPointer<JTVis_GraphicObject> JTVis_GraphicObjectPtr;

#endif // JTVIS_GRAPHICOBJECT_H
