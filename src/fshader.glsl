#version 150

in  vec4  color;
in vec2 texCoord;

uniform sampler2D texture;

void main() 
{ 
	//gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );  // white
	gl_FragColor = texture2D(texture, texCoord);
} 

