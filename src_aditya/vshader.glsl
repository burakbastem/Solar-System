#version 150

in  vec4 vPosition;
in  vec3 vNormal;

uniform mat4 Projection;
uniform mat4 ModelView;


//----------------------------------------------------------------------------

void main() 
{         
   gl_Position = Projection * ModelView * vPosition;		
} 
