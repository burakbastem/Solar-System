#version 150

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;

out vec2 texCoord;

uniform mat4 Projection;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

uniform vec4 LightPosition;

varying vec4 pos;
varying vec3 normal;
varying  vec3 fN;
varying  vec3 fV;
varying  vec3 fL;
uniform int Shading;

//----------------------------------------------------------------------------

void main() 
{         
   //Shading = 0;
   if(Shading == 1) {
    // Phong shading
    pos = ModelMatrix * vPosition;
    normal = vNormal;
    }
   texCoord = vTexCoords;
   gl_Position = Projection*ViewMatrix*ModelMatrix*vPosition;		
} 
