#version 150
uniform vec4 color;

uniform sampler2D texture;

uniform int picking_mode;
uniform int textureFlag;
uniform int Shading;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

// per-fragment interpolated values from the vertex shader
varying vec4 pos;

varying vec3 normal;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition;

uniform float Shininess;

varying  vec2 texCoord;

varying float s_coord;

uniform int TextureFlag;
uniform int OneDTextureFlag;

uniform sampler2D texture2d;

uniform int Modified_reflection;

void main() 
{ 
	if(picking_mode == 1){
		gl_FragColor = color;  // white
	} else {
		//gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 ); 
		if(textureFlag == 1)	gl_FragColor = texture2D(texture, texCoord);
		else gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );

		//Shading = 0;

		if (Shading == 1) {
			// if Phong shading mode is used


			vec3 fN = (ModelMatrix*vec4(normal, 0.0)).xyz; // normal direction in camera coordinates

    			vec3 fV = (-ViewMatrix * pos).xyz; //viewer direction in camera coordinates

    			vec3 fL = (LightPosition - pos).xyz;
			// Normalize the input lighting vectors
        		vec3 N = normalize(fN);
        		vec3 V = normalize(fV);

			// first point light source
        		vec3 L = normalize(fL);
			float a = 0.01;
			float b = 0.01;
			float c = 0.01;
			float light_distance = length(fL);

        		vec3 H;
        		float Ks;
        
        		vec4 ambient = AmbientProduct;

        		float Kd = max(dot(L, N), 0.0);
        		vec4 diffuse = Kd*DiffuseProduct;
        
			vec3 R;
	
			R = normalize( 2 * dot(L,N) * N - L);
			Ks = pow( max(dot(V, R), 0.0), Shininess );
        		vec4 specular = Ks*SpecularProduct;

        		// discard the specular highlight if the light's behind the vertex
        		if( dot(L, N) < 0.0 ) {
            			specular = vec4(0.0, 0.0, 0.0, 1.0);
        		}

        		gl_FragColor += ambient + (diffuse + specular) / (a + b*light_distance + c * light_distance * light_distance);
    		}
			
	}
} 

