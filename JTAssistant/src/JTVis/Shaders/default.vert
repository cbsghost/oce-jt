/*
  JT format reading and visualization tools
  Copyright (C) 2015 OPEN CASCADE SAS
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 2 of the License, or any later
  version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  
  Copy of the GNU General Public License is in LICENSE.txt and  
  on <http://www.gnu.org/licenses/>.
*/

attribute vec4 aPosition;
attribute vec4 aNormal;

uniform mat4 uMvpMatrix;
uniform mat4 uModelView;
uniform mat4 uNormalMatrix;

varying vec4 vPosition;
varying vec4 vNormal;

void main() 
{
   vPosition = uModelView * vec4 (aPosition.xyz, 1.0);

   vNormal = vec4 (aNormal.xyz, 0.0) * uNormalMatrix;
   gl_Position = uMvpMatrix * aPosition;
}
