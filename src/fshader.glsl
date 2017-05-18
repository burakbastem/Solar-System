#version 150

in vec2 texCoord;
uniform vec4 color;

uniform sampler2D texture;

uniform int picking_mode;

void main() 
{ 
	if(picking_mode == 1)
		gl_FragColor = color;  // white
	else {
		gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 ); 	
		//gl_FragColor = texture2D(texture, texCoord);
	}
} 

