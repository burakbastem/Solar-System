/*
 * Ahmad Najeeb
 * Bastem Burak
 * Sasongko Muhammad Aditya
 *
 * 12 May 2017
 *
 * Acknowledgement: Based on code provided by 
 * 	https://www.cs.unm.edu/~angel/BOOK/INTERACTIVE_COMPUTER_GRAPHICS/SIXTH_EDITION/
 *
 * Information about objects is from https://solarsystem.nasa.gov/.
 *		- https://solarsystem.nasa.gov/planetinfo/charchart.cfms
 *		- Select metric notation.
 */

#include "Angel.h"
#include "TextureFromJPG.h"
#include <math.h>
#include <iostream>
#define NUM_OF_OBJECTS 8
double EARTH_REVOLUTION_ANGULAR_SPEED = 0.05;

using namespace std;

enum {
	Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3
};

int Axis = Yaxis;
double prejection_coef = 40;
mat4 view;
double camera_theta = 0;
double camera_phi = 0;
mat4 camera_translate;

class AstronomicalObject {
public:
	string name;							// name of the object
	double average_orbit_distance;			// in 10^3 km
	double equatorial_radius;				// in 10^3 km 
	double rotation_period;					// in earth days
	double orbit_period;					// in earth years
	AstronomicalObject* orbiting_objects;// for example Earth and other planets orbit Sun.
	int num_orbiting_objects;
	mat4 object_model_view;
	mat4 TiltingAngle;
	GLfloat RevolutionTheta[NumAxes];
	GLfloat RotationTheta[NumAxes];
	GLuint TexID;
};

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumTimesToSubdivide = 6;
const int NumTrianglesSphere = 16384;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVerticesSphere = 3 * NumTrianglesSphere;
point4 spherePoints[NumVerticesSphere];
vec3 sphereNormals[NumVerticesSphere];
vec2 texCoords[NumVerticesSphere];
static int l = 0;
GLuint program;
GLuint Color;
GLuint PickingMode;
GLuint TextureFlag;

AstronomicalObject sun;
AstronomicalObject* planets;
AstronomicalObject* satellites;

// before

GLuint AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition; 
 
GLfloat Shininess;

GLuint ModelView;
GLuint Shading_mode;

// for the first point light source
color4 ambient_product; // k_a * L_a
color4 diffuse_product; // k_d * L_d 
color4 specular_product; // k_s * L_s


point4 light_position( 0.0, 0.0, 0.0, 1.0 );
color4 light_ambient( 0.2, 0.2, 0.2, 1.0 ); // L_a
color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 ); // L_d
color4 light_specular( 1.0, 1.0, 1.0, 1.0 ); // L_s

color4 material_ambient( 1.0, 0.0, 1.0, 1.0 ); // k_a
color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 ); // k_d
color4 material_specular( 1.0, 0.8, 0.0, 1.0 ); // k_s

//----------------------------------------------------------------------------

// Loading texture images

void LoadTextureImages() {
	CTexture LoadImages;
	// Load texture for sun
	LoadImages.loadTexture2D("../images/texture_sun.jpg", sun.TexID, true);	
	// Load texture for planets
	LoadImages.loadTexture2D("../images/texture_mercury.jpg", planets[0].TexID, true);
	LoadImages.loadTexture2D("../images/texture_venus.jpg", planets[1].TexID, true);
	LoadImages.loadTexture2D("../images/texture_earth.jpg", planets[2].TexID, true);
	LoadImages.loadTexture2D("../images/texture_mars.jpg", planets[3].TexID, true);
	LoadImages.loadTexture2D("../images/texture_jupiter.jpg", planets[4].TexID, true);
	LoadImages.loadTexture2D("../images/texture_saturn.jpg", planets[5].TexID, true);
	LoadImages.loadTexture2D("../images/texture_uranus.jpg", planets[6].TexID, true);
	LoadImages.loadTexture2D("../images/texture_neptune.jpg", planets[7].TexID, true);	
	// Load texture for the moon
	LoadImages.loadTexture2D("../images/texture_moon.jpg",
			planets[2].orbiting_objects[0].TexID, true);
	LoadImages.loadTexture2D("../images/texture_ganymede.jpg",
			planets[4].orbiting_objects[0].TexID, true);
	LoadImages.loadTexture2D("../images/texture_europa.jpg",
			planets[4].orbiting_objects[1].TexID, true);
	LoadImages.loadTexture2D("../images/texture_io.jpg",
			planets[4].orbiting_objects[2].TexID, true);
	LoadImages.loadTexture2D("../images/texture_callisto.jpg",
			planets[4].orbiting_objects[3].TexID, true);
	//LoadImages.loadTexture2D("../images/texture_earth.jpg",sun.TexID,true);
	//LoadImages.loadTexture2D("../images/texture_venus.jpg",sun.TexID,true);
}

//----------------------------------------------------------------------------

int Index_s = 0;

// CalcTextureCoordinates
vec2 CalcTextureCoordinates(vec4 point) {
	double s, t;
	s = atan2(point[0], point[2]) / (2. * M_PI) + 0.5;
	t = asin(point[1]) / M_PI + .5;
	if (l != 0) {
		if (s < 0.75 && texCoords[Index_s - 1][0] > 0.75)
			s += 1.0;
		else if (s > 0.75 && texCoords[Index_s - 1][0] < 0.75)
			s -= 1.0;
	}
	return vec2(s, 1.0 - t);

}

void triangle(const point4& a, const point4& b, const point4& c) {
	vec3 normal = normalize(cross(b - a, c - b));

	sphereNormals[Index_s] = normal;
	spherePoints[Index_s] = a;
	texCoords[Index_s] = CalcTextureCoordinates(a);
	Index_s++;
	sphereNormals[Index_s] = normal;
	spherePoints[Index_s] = b;
	texCoords[Index_s] = CalcTextureCoordinates(a);
	Index_s++;
	sphereNormals[Index_s] = normal;
	spherePoints[Index_s] = c;
	texCoords[Index_s] = CalcTextureCoordinates(a);
	Index_s++;
}

point4 unit(const point4& p) {
	float len = p.x * p.x + p.y * p.y + p.z * p.z;

	point4 t;
	if (len > DivideByZeroTolerance) {
		t = p / sqrt(len);
		t.w = 1.0;
	}

	return t;
}

void divide_triangle(const point4& a, const point4& b, const point4& c,
		int count) {
	if (count > 0) {
		point4 v1 = unit(a + b);
		point4 v2 = unit(a + c);
		point4 v3 = unit(b + c);
		divide_triangle(a, v1, v2, count - 1);
		divide_triangle(c, v2, v3, count - 1);
		divide_triangle(b, v3, v1, count - 1);
		divide_triangle(v1, v3, v2, count - 1);
	} else {
		triangle(a, b, c);
	}
}

void tetrahedron(int count) {
	point4 v[4] = { vec4(0.0, 0.0, 1.0, 1.0), vec4(0.0, 0.942809, -0.333333,
			1.0), vec4(-0.816497, -0.471405, -0.333333, 1.0), vec4(0.816497,
			-0.471405, -0.333333, 1.0) };

	divide_triangle(v[0], v[1], v[2], count);
	divide_triangle(v[3], v[2], v[1], count);
	divide_triangle(v[0], v[3], v[1], count);
	divide_triangle(v[0], v[2], v[3], count);
}

//----------------------------------------------------------------------------

void initAstronomicalObjects() {
	// real average_orbit_distance and equatorial_radius are not feasible right now.

	sun = AstronomicalObject();
	sun.name = "Sun";
	sun.average_orbit_distance = 0;
	sun.equatorial_radius = 1;	// 696;
	sun.rotation_period = 1.1;
	sun.orbit_period = 0;
	sun.TiltingAngle = RotateY(10) * RotateZ(10);
	planets = new AstronomicalObject[8];
	sun.orbiting_objects = planets;
	sun.num_orbiting_objects = 8;

	// mercury
	planets[0].name = "Mercury";
	planets[0].average_orbit_distance = 4;	//57909;
	planets[0].equatorial_radius = 0.2; //2.4;
	planets[0].rotation_period = 0.02;
	planets[0].orbit_period = 0.2;
	planets[0].orbiting_objects = NULL;
	planets[0].TiltingAngle = RotateY(5) * RotateZ(10);
	// venus
	planets[1].name = "Venus";
	planets[1].average_orbit_distance = 8; //108209;
	planets[1].equatorial_radius = 0.4; //6.1;
	planets[1].rotation_period = 0.2;
	planets[1].orbit_period = 0.6;
	planets[1].orbiting_objects = NULL;
	planets[1].TiltingAngle = RotateY(5) * RotateZ(8);
	// earth
	planets[2].name = "Earth";
	planets[2].average_orbit_distance = 12; //149598;
	planets[2].equatorial_radius = 0.4; //6.4;
	planets[2].rotation_period = 0.003;
	planets[2].orbit_period = 1;
	planets[2].num_orbiting_objects = 1;
	planets[2].TiltingAngle = RotateY(8) * RotateZ(7);
	satellites = new AstronomicalObject[1];
	planets[2].orbiting_objects = satellites;
	satellites[0].name = "Moon";
	satellites[0].average_orbit_distance = 2;
	satellites[0].equatorial_radius = 0.1; //6.4;
	satellites[0].rotation_period = 0.01;
	satellites[0].orbit_period = 0.1;
	satellites[2].TiltingAngle = RotateY(20) * RotateZ(7);
	// mars
	planets[3].name = "Mars";
	planets[3].average_orbit_distance = 16; //227944;
	planets[3].equatorial_radius = 0.4; //3.4;
	planets[3].rotation_period = 0.01;
	planets[3].orbit_period = 1.9;
	planets[3].orbiting_objects = NULL;
	planets[3].TiltingAngle = RotateY(20) * RotateZ(45);
	// jupiter
	planets[4].name = "Jupiter";
	planets[4].average_orbit_distance = 20; //778341;
	planets[4].equatorial_radius = 0.7; //69.9;
	planets[4].rotation_period = 0.04;
	planets[4].orbit_period = 11.9;
	planets[2].TiltingAngle = RotateY(45) * RotateZ(10);

	// before
	planets[4].num_orbiting_objects = 4;
	satellites = new AstronomicalObject[4];
	planets[4].orbiting_objects = satellites;
	satellites[0].name = "Ganymede";
	satellites[0].average_orbit_distance = 1;
	satellites[0].equatorial_radius = 0.3; //6.4;
	satellites[0].rotation_period = 0.01;
	satellites[0].orbit_period = 0.4;
	satellites[0].TiltingAngle = RotateY(10) * RotateZ(12);
	satellites[1].name = "Europa";
	satellites[1].average_orbit_distance = 2;
	satellites[1].equatorial_radius = 0.1; //6.4;
	satellites[1].rotation_period = 0.01;
	satellites[1].orbit_period = 0.5;
	satellites[1].TiltingAngle = RotateY(12) * RotateZ(9);
	satellites[2].name = "Io";
	satellites[2].average_orbit_distance = 3;
	satellites[2].equatorial_radius = 0.15; //6.4;
	satellites[2].rotation_period = 0.01;
	satellites[2].orbit_period = 0.3;
	satellites[2].TiltingAngle = RotateY(50) * RotateZ(15);
	satellites[3].name = "Callisto";
	satellites[3].average_orbit_distance = 4;
	satellites[3].equatorial_radius = 0.2; //6.4;
	satellites[3].rotation_period = 0.01;
	satellites[3].orbit_period = 0.5;
	satellites[3].TiltingAngle = RotateY(20) * RotateZ(10);
	// after
	// saturn
	planets[5].name = "Saturn";
	planets[5].average_orbit_distance = 24; //1426666;
	planets[5].equatorial_radius = 0.6; //58.2;
	planets[5].rotation_period = 0.04;
	planets[5].orbit_period = 29.4;
	planets[5].num_orbiting_objects = 2;
	planets[5].orbiting_objects = NULL;
	planets[5].TiltingAngle = RotateY(28) * RotateZ(7);
	/*satellites = new AstronomicalObject[2];
	planets[5].orbiting_objects = satellites;
	satellites[0].name = "Titan";
	satellites[0].average_orbit_distance = 1;
	satellites[0].equatorial_radius = 0.25; //6.4;
	satellites[0].rotation_period = 0.01;
	satellites[0].orbit_period = 0.3;
	satellites[2].TiltingAngle = RotateY(18) * RotateZ(17);
	satellites[1].name = "Rhea";
	satellites[1].average_orbit_distance = 2;
	satellites[1].equatorial_radius = 0.1; //6.4;
	satellites[1].rotation_period = 0.01;
	satellites[1].orbit_period = 0.5;
	satellites[1].TiltingAngle = RotateY(8) * RotateZ(17);
        */
	// uranus
	planets[6].name = "Uranus";
	planets[6].average_orbit_distance = 28; //2870658;
	planets[6].equatorial_radius = 0.6; //25.4;
	planets[6].rotation_period = 0.07;
	planets[6].orbit_period = 84;
	planets[6].num_orbiting_objects = 2;
	planets[6].TiltingAngle = RotateY(8) * RotateZ(27);
	planets[6].orbiting_objects = NULL;
	/*satellites = new AstronomicalObject[2];
	planets[6].orbiting_objects = satellites;
	satellites[0].name = "Titania";
	satellites[0].average_orbit_distance = 1;
	satellites[0].equatorial_radius = 0.1; //6.4;
	satellites[0].rotation_period = 0.01;
	satellites[0].orbit_period = 0.6;
	satellites[0].TiltingAngle = RotateY(8) * RotateZ(7);
	satellites[1].name = "Oberon";
	satellites[1].average_orbit_distance = 2;
	satellites[1].equatorial_radius = 0.1; //6.4;
	satellites[1].rotation_period = 0.01;
	satellites[1].orbit_period = 0.7;
	satellites[1].TiltingAngle = RotateY(8) * RotateZ(7); */
	
	// neptune
	planets[7].name = "Neptune";
	planets[7].average_orbit_distance = 32; //4498396;
	planets[7].equatorial_radius = 0.6; //24.6;
	planets[7].rotation_period = 0.07;
	planets[7].orbit_period = 164.8;
	planets[7].num_orbiting_objects = 1;
	planets[7].TiltingAngle = RotateY(8) * RotateZ(7);
	planets[7].orbiting_objects = NULL;
	/*satellites = new AstronomicalObject[1];
	planets[7].orbiting_objects = satellites;
	satellites[0].name = "Triton";
	satellites[0].average_orbit_distance = 1;
	satellites[0].equatorial_radius = 0.1; //6.4;
	satellites[0].rotation_period = 0.01;
	satellites[0].orbit_period = 0.2;
	satellites[0].TiltingAngle = RotateY(8) * RotateZ(7); */
}

//----------------------------------------------------------------------------

void computeView(){
	// view
	point4  eye( -sin(camera_theta)*cos(camera_phi), sin(camera_phi), cos(camera_theta)*cos(camera_phi), 1.0 );
   point4  at( 0.0, 0.0, 0.0, 1.0 );
   vec4    up( 0.0, 1.0, 0.0, 0.0 );
   view = camera_translate * LookAt( eye, at, up );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init() {
	// Subdivide a tetrahedron into a sphere
	tetrahedron(NumTimesToSubdivide);

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(spherePoints) + sizeof(sphereNormals) + sizeof(texCoords),
			NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(spherePoints),
			sizeof(sphereNormals), sphereNormals);
	glBufferSubData(GL_ARRAY_BUFFER,
			sizeof(spherePoints) + sizeof(sphereNormals), sizeof(texCoords),
			texCoords);
	// Load shaders and use the resulting shader program
	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));
	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(spherePoints)));
	GLuint vTexCoords = glGetAttribLocation(program, "vTexCoords");
	glEnableVertexAttribArray(vTexCoords);
	glVertexAttribPointer(vTexCoords, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(spherePoints) + sizeof(sphereNormals)));
	glEnable (GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);

	PickingMode = glGetUniformLocation(program, "picking_mode");
	TextureFlag = glGetUniformLocation(program, "textureFlag");
	Color = glGetUniformLocation(program, "color");

	// before
	float  material_shininess = 150.0;

    	ambient_product = light_ambient * material_ambient; // k_a * L_a
    	diffuse_product = light_diffuse * material_diffuse; // k_d * L_d
    	specular_product = light_specular * material_specular; // k_s * L_s

    	AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
    	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
    	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
    	LightPosition = glGetUniformLocation(program, "LightPosition");

	// first point light source is activated by default
    	glUniform4fv( AmbientProduct, 1, ambient_product );
    	glUniform4fv( DiffuseProduct, 1, diffuse_product );
    	glUniform4fv( SpecularProduct, 1, specular_product );
    	glUniform4fv( LightPosition, 1, light_position );

    	Shininess = glGetUniformLocation(program, "Shininess");
    	glUniform1f( Shininess, material_shininess );

    	Shading_mode = glGetUniformLocation(program, "Shading");
	glUniform1i(Shading_mode , 1 );
	// after

	initAstronomicalObjects();
	
	computeView();
	
	// Load Texture images
	LoadTextureImages();

}

//----------------------------------------------------------------------------
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// sun
	GLfloat YRotationAngle = sun.RotationTheta[Yaxis];
	mat4 mv = view * sun.TiltingAngle * RotateY(YRotationAngle);
	glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE,	mv);
	glUniform1i(TextureFlag, 1);
	glBindTexture(GL_TEXTURE_2D, sun.TexID);
	glUniform1i(Shading_mode , 0 );
	glDrawArrays(GL_TRIANGLES, 0, NumVerticesSphere);
	glUniform1i(Shading_mode , 1 );
	// planets
	for (int i = 0; i < sun.num_orbiting_objects; i++) {
		double t = sun.orbiting_objects[i].average_orbit_distance;
		double s = sun.orbiting_objects[i].equatorial_radius / sun.equatorial_radius;
		GLfloat YRevolutionAngle = sun.orbiting_objects[i].RevolutionTheta[Yaxis];
		YRotationAngle = sun.orbiting_objects[i].RotationTheta[Yaxis];
		mat4 planet_model = RotateY(YRevolutionAngle) * Translate(t, 0, 0) * sun.orbiting_objects[i].TiltingAngle * RotateY(YRotationAngle);
		glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE, view * planet_model * Scale(s, s, s));
		glUniform1i(TextureFlag, 1);
		glBindTexture(GL_TEXTURE_2D, sun.orbiting_objects[i].TexID);
		glDrawArrays(GL_TRIANGLES, 0, NumVerticesSphere);
		if (sun.orbiting_objects[i].orbiting_objects) {
		  glUniform1i(TextureFlag, 1);
			for (int j = 0; j < sun.orbiting_objects[i].num_orbiting_objects; j++) {
				AstronomicalObject satellite = sun.orbiting_objects[i].orbiting_objects[j];
				// orbit distance of satallite from the planet that satallite rotates around
				double t_s = satellite.average_orbit_distance;
				// scale value respect to planet that stallite rotates around
				double s_s = satellite.equatorial_radius / sun.orbiting_objects[i].equatorial_radius;
				YRevolutionAngle = satellite.RevolutionTheta[Yaxis];
				YRotationAngle = satellite.RotationTheta[Yaxis];
				mat4 satellite_model = planet_model * RotateY(YRevolutionAngle) * Translate(t_s, 0, 0) * satellite.TiltingAngle * RotateY(YRotationAngle);
				//mat4 satellite_model = Translate(t_s, 0, 0) * Scale(s_s, s_s, s_s) * planet_model;
				glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE, 
						view * satellite_model * Scale(s_s, s_s, s_s));
				//glDrawArrays(GL_TRIANGLES, 0, NumVerticesSphere);
				if (planets[i].orbiting_objects[j].TexID > 0)
				  {glBindTexture(GL_TEXTURE_2D, planets[i].orbiting_objects[j].TexID);}
				   glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
			}
		}
	}
	glutSwapBuffers();
}

//----------------------------------------------------------------------------

/* This function handles idle event so that a face rotation 
 * keeps continuing until 90 degree is completed and
 * in case of randomized rotations, this function will
 * execute continuous face rotations.
 * The skeleton of this function is from the spinCube code
 */
void idle(void) {
	int i;
	sun.RotationTheta[Axis] -= EARTH_REVOLUTION_ANGULAR_SPEED / sun.rotation_period;
	if (sun.RotationTheta[Axis] > 360.0) {
		sun.RotationTheta[Axis] -= 360.0;
	}
	if (sun.RotationTheta[Axis] < -360.0) {
		sun.RotationTheta[Axis] += 360.0;
	}
	for (i = 0; i < NUM_OF_OBJECTS; i++) {
		sun.orbiting_objects[i].RevolutionTheta[Axis] -= EARTH_REVOLUTION_ANGULAR_SPEED / sun.orbiting_objects[i].orbit_period;
		if (sun.orbiting_objects[i].RevolutionTheta[Axis] > 360.0) {
			sun.orbiting_objects[i].RevolutionTheta[Axis] -= 360.0;
		}

		if (sun.orbiting_objects[i].RevolutionTheta[Axis] < -360.0) {
			sun.orbiting_objects[i].RevolutionTheta[Axis] += 360.0;
		}

		sun.orbiting_objects[i].RotationTheta[Axis] -= EARTH_REVOLUTION_ANGULAR_SPEED / sun.orbiting_objects[i].rotation_period;
		if (sun.orbiting_objects[i].RotationTheta[Axis] > 360.0) {
			sun.orbiting_objects[i].RotationTheta[Axis] -= 360.0;
		}

		if (sun.orbiting_objects[i].RotationTheta[Axis] < -360.0) {
			sun.orbiting_objects[i].RotationTheta[Axis] += 360.0;
		}

		if (sun.orbiting_objects[i].orbiting_objects) {
			int j;
			for (j = 0; j < sun.orbiting_objects[i].num_orbiting_objects; j++) {
				//AstronomicalObject * satellite = &(sun.orbiting_objects[i].orbiting_objects[j]);
				sun.orbiting_objects[i].orbiting_objects[j].RevolutionTheta[Axis] -=
						EARTH_REVOLUTION_ANGULAR_SPEED
								/ sun.orbiting_objects[i].orbiting_objects[j].orbit_period;
				//printf("moon is set angular with angular speed %lf\n", sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis]);
				if (sun.orbiting_objects[i].orbiting_objects[j].RevolutionTheta[Axis]
						> 360.0) {
					sun.orbiting_objects[i].orbiting_objects[j].RevolutionTheta[Axis] -=
							360.0;
				}

				if (sun.orbiting_objects[i].orbiting_objects[j].RevolutionTheta[Axis]
						< -360.0) {
					sun.orbiting_objects[i].orbiting_objects[j].RevolutionTheta[Axis] +=
							360.0;
				}
				sun.orbiting_objects[i].orbiting_objects[j].RotationTheta[Axis] -=
						EARTH_REVOLUTION_ANGULAR_SPEED
								/ sun.orbiting_objects[i].orbiting_objects[j].rotation_period;
				if (sun.orbiting_objects[i].orbiting_objects[j].RotationTheta[Axis]
						> 360.0) {
					sun.orbiting_objects[i].orbiting_objects[j].RotationTheta[Axis] -=
							360.0;
				}

				if (sun.orbiting_objects[i].orbiting_objects[j].RotationTheta[Axis]
						< -360.0) {
					sun.orbiting_objects[i].orbiting_objects[j].RotationTheta[Axis] +=
							360.0;
				}
			}
		}
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	GLfloat left = -prejection_coef, right = prejection_coef;
	GLfloat top = prejection_coef, bottom = -prejection_coef;
	GLfloat zNear = -1000*prejection_coef, zFar = 1000*prejection_coef;
	GLfloat aspect = GLfloat(width) / height;
	if (aspect > 1.0) {
		left *= aspect;
		right *= aspect;
	} else {
		top /= aspect;
		bottom /= aspect;
	}
	mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);
	glUniformMatrix4fv(glGetUniformLocation(program, "Projection"), 1, GL_TRUE, projection);
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'h':
		cout << "######################################################" << endl;
		cout << "------Solar System------" << endl;
		cout << "w -- move camera forward" << endl;
		cout << "s -- move camera backward" << endl;
		cout << "a -- move camera left" << endl;
		cout << "d -- move camera right" << endl;
		cout << "f -- increase speed of simulation" << endl;
		cout << "v -- reduce speed of simulation" << endl;
		cout << "up -- rotate camera clockwise around x axis" << endl;
		cout << "down -- rotate camera counter-clockwise around x axis" << endl;
		cout << "left -- rotate camera counter-clockwise around y axis" << endl;
		cout << "right -- rotate camera clockwise around y axis" << endl;
		cout << "h -- help" << endl;
		cout << "q -- quit (exit) the program" << endl;
		cout << "######################################################" << endl;
		break;	
	case 'w':
		prejection_coef -= 0.25;
		reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case 's':
		prejection_coef += 0.25;
		reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case 'a':
		camera_translate = Translate(1,0,0) * camera_translate;
		computeView();
		break;
	case 'd':
		camera_translate = Translate(-1,0,0) * camera_translate;
		computeView();
		break;
	case 'f':
		EARTH_REVOLUTION_ANGULAR_SPEED *= 2;
		break;
	case 'v':
		EARTH_REVOLUTION_ANGULAR_SPEED /= 2;
		break;
	case 'q':
	case 'Q':
	case 033: // Escape key
		exit (EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

void specialKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:		
		camera_phi += 0.01;
		computeView();
		break;
	case GLUT_KEY_DOWN:
		camera_phi -= 0.01;
		computeView();
		break;
	case GLUT_KEY_LEFT:
		camera_theta += 0.01;
		computeView();
		break;
	case GLUT_KEY_RIGHT:
		camera_theta -= 0.01;
		computeView();
		break;
	}
}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y) {

	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {

		// begin
		glUniform1i(PickingMode, 1);
		double red_val = 0.05;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform4f(Color, red_val, 0.0, 0.0, 1.0);
		mat4 mv = view;
		glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE,	mv);
		glDrawArrays(GL_TRIANGLES, 0, NumVerticesSphere);
		// end

		for (int i = 0; i < sun.num_orbiting_objects; i++) {

			// before
			double t = sun.orbiting_objects[i].average_orbit_distance;
			double s = sun.orbiting_objects[i].equatorial_radius / sun.equatorial_radius;
			red_val += 0.05;
			glUniform4f(Color, red_val, 0.0, 0.0, 1.0);
			GLfloat YRevolutionAngle = sun.orbiting_objects[i].RevolutionTheta[Yaxis];
			mat4 planet_model = RotateY(YRevolutionAngle) * Translate(t, 0, 0);
			glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE, view * planet_model * Scale(s, s, s));
			glDrawArrays(GL_TRIANGLES, 0, NumVerticesSphere);
			// after


			if (sun.orbiting_objects[i].orbiting_objects) {
				int j;
				for (j = 0; j < sun.orbiting_objects[i].num_orbiting_objects;
						j++) {
					// begin
					AstronomicalObject satellite = sun.orbiting_objects[i].orbiting_objects[j];
					// orbit distance of satallite from the planet that satallite rotates around
					double t_s = satellite.average_orbit_distance;
					// scale value respect to planet that stallite rotates around
					double s_s = satellite.equatorial_radius / sun.orbiting_objects[i].equatorial_radius;
					red_val += 0.05;
					glUniform4f(Color, red_val, 0.0, 0.0, 1.0);
					YRevolutionAngle = satellite.RevolutionTheta[Yaxis];
					mat4 satellite_model = planet_model * RotateY(YRevolutionAngle) * Translate(t_s, 0, 0);
					//mat4 satellite_model = Translate(t_s, 0, 0) * Scale(s_s, s_s, s_s) * planet_model;
					glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE, 
						view * satellite_model * Scale(s_s, s_s, s_s));
					glDrawArrays(GL_TRIANGLES, 0, NumVerticesSphere);
					// end
				}
			}
		}
		// after
		y = glutGet(GLUT_WINDOW_HEIGHT) - y;
		unsigned char pixel[4];
		glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		glUniform1i(PickingMode, 0);
		glFlush();
		if (pixel[0] == 13 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Sun" << std::endl;
		} else if (pixel[0] == 25 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Mercury" << std::endl;
		} else if (pixel[0] == 38 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Venus" << std::endl;
		} else if (pixel[0] == 51 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Earth" << std::endl;
		} else if (pixel[0] == 64 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Moon" << std::endl;
		} else if (pixel[0] == 76 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Mars" << std::endl;
		} else if (pixel[0] == 89 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Jupiter" << std::endl;
		} else if (pixel[0] == 102 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Ganymede" << std::endl;
		} else if (pixel[0] == 115 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Europa" << std::endl;
		} else if (pixel[0] == 128 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Io" << std::endl;
		} else if (pixel[0] == 140 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Callisto" << std::endl;
		} else if (pixel[0] == 153 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Saturn" << std::endl;
		} else if (pixel[0] == 166 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Titan" << std::endl;
		} else if (pixel[0] == 178 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Rhea" << std::endl;
		} else if (pixel[0] == 191 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Uranus" << std::endl;
		} else if (pixel[0] == 204 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Titania" << std::endl;
		} else if (pixel[0] == 217 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Oberon" << std::endl;
		} else if (pixel[0] == 229 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Neptune" << std::endl;
		} else if (pixel[0] == 242 && pixel[1] == 0 && pixel[2] == 0) {
			std::cout << "Triton" << std::endl;
		} else {
			std::cout << "None" << std::endl;
		}
		std::cout << "R: " << (int) pixel[0] << std::endl;
		std::cout << "G: " << (int) pixel[1] << std::endl;
		std::cout << "B: " << (int) pixel[2] << std::endl;
		std::cout << std::endl;
		glutPostRedisplay(); //needed to avoid display of the content of the back buffer when some portion of the window is obscured
	}
}

//----------------------------------------------------------------------------

int main(int argc, char **argv) {
	// initializing window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile (GLUT_CORE_PROFILE);
	glutCreateWindow("Solar System");

	glewExperimental = GL_TRUE;
	glewInit();

	init();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMainLoop();

	return 0;
}
