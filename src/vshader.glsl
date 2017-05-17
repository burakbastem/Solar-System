#version 150

in  vec4 vPosition;
in  vec3 vNormal;
in vec2 vTexCoords;

out vec2 texCoord;

uniform mat4 Projection;
uniform mat4 ModelView;


//----------------------------------------------------------------------------

void main() 
{         
   texCoord = vTexCoords;
   gl_Position = Projection * ModelView * vPosition;		
} 
