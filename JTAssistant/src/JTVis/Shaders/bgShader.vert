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

attribute vec4 aPos;
attribute vec2 aTexcoord;

varying vec3 vColor;

void main() 
{
   vColor = mix (vec3 (0.91, 0.93, 0.94),
                 vec3 (0.62, 0.64, 0.67),
                 sign (aPos.y));

   gl_Position = aPos;
}
