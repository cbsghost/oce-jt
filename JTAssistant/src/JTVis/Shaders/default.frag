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

varying vec4 vPosition; //!< Vertex position in view space.
varying vec4 vNormal;   //!< Vertex normal in view space.

uniform vec4 uColors[3];

vec3 Ambient;
vec3 Diffuse;
vec3 Specular;

//! Computes contribution of directional light source
void directionalLight (in vec3 theNormal,
                       in vec3 theView)
{
  // headlight
  vec3 aLight = theView;

  vec3 aHalf = normalize (aLight + theView);

  float aNdotL = abs (dot (theNormal, aLight));
  float aNdotH = abs (dot (theNormal, aHalf ));

  float aSpecl = 0.0;
  if (aNdotL > 0.0)
  {
    aSpecl = pow (aNdotH, uColors[2].w);
  }

  Diffuse  += vec3 (0.8, 0.8, 0.8) * aNdotL;
  Specular += vec3 (0.5, 0.5, 0.5) * aSpecl;
}

//! Computes illumination from light sources
vec4 computeLighting (in vec3 theNormal,
                      in vec3 theView,
                      in vec4 thePoint)
{
  // Clear the light intensity accumulators
  Ambient  = vec3 (0.4);
  Diffuse  = vec3 (0.0);
  Specular = vec3 (0.0);
  vec3 aPoint = thePoint.xyz / thePoint.w;

  directionalLight (theNormal, theView);

  return vec4 (  Ambient  * uColors[0].xyz
               + Diffuse  * uColors[1].xyz
               + Specular * uColors[2].xyz, 1.0);
}

//! Entry point to the Fragment Shader
void main()
{
  gl_FragColor = computeLighting (normalize (vNormal.xyz),
                                  vec3 (0.0, 0.0, -1.0), // legit for ortho camera only
                                  vPosition);
}
