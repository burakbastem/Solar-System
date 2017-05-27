#version 150

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;

out vec2 texCoord;

uniform mat4 Projection;
uniform mat4 ModelView;

uniform vec4 LightPosition;

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
	
    fN = (ModelView*vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates

    fV = (ModelView * vPosition).xyz; //viewer direction in camera coordinates

    fL = LightPosition.xyz - fV;
    }
   texCoord = vTexCoords;
   gl_Position = Projection * ModelView * vPosition;		
} 
