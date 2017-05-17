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
#include <math.h>
#include <iostream>
#define NUM_OF_OBJECTS 8
#define EARTH_REVOLUTION_ANGULAR_SPEED 0.02

using namespace std;

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };

int  Axis = Zaxis;

class AstronomicalObject {
   public:
   	string name;									// name of the object
   	double average_orbit_distance;			// in 10^3 km
	double equatorial_radius;					// in 10^3 km 
	double rotation_period;						// in earth days
	double orbit_period;							// in earth years
	AstronomicalObject* orbiting_objects;	// for example Earth and other planets orbit Sun.
	int num_orbiting_objects;
	mat4  object_model_view;
	GLfloat Theta[NumAxes];
};

typedef Angel::vec4  point4;
typedef Angel::vec4  color4;

const int NumTimesToSubdivide = 5;
const int NumTrianglesSphere  = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVerticesSphere   = 3 * NumTrianglesSphere;
point4 spherePoints[NumVerticesSphere];
vec3   sphereNormals[NumVerticesSphere];

GLuint program;

AstronomicalObject sun;
// model-view matrices
mat4 model_views[NUM_OF_OBJECTS];
// 0 - Sun
// 1 - Mercury
// 2 - Venus
// 3 - Earth
// 4 - Mars
// 5 - Jupiter
// 6 - Saturn
// 7 - Uranus
// 8 - Neptune

//----------------------------------------------------------------------------

void initAstronomicalObjects(){
	// real average_orbit_distance and equatorial_radius are not feasible right now.
	sun = AstronomicalObject();
	sun.name = "Sun";
	sun.average_orbit_distance = 0;
   sun.equatorial_radius = 1;// 696;
   sun.rotation_period = 1.1;
   sun.orbit_period = 0;
   AstronomicalObject* planets = new AstronomicalObject[8];
   AstronomicalObject* satellites;
   sun.orbiting_objects = planets;
   sun.num_orbiting_objects = 8;
   // mercury
   planets[0].name = "Mercury";
	planets[0].average_orbit_distance = 4;//57909;
   planets[0].equatorial_radius = 0.2; //2.4;
   planets[0].rotation_period = 58.6;
   planets[0].orbit_period = 0.2;
	planets[0].orbiting_objects = NULL;
   // venus
   planets[1].name = "Venus";
	planets[1].average_orbit_distance = 8; //108209;
   planets[1].equatorial_radius = 0.4; //6.1;
   planets[1].rotation_period = -243;
   planets[1].orbit_period = 0.6;
	planets[1].orbiting_objects = NULL;
   // earth
   planets[2].name = "Earth";
	planets[2].average_orbit_distance = 12; //149598;
   planets[2].equatorial_radius = 0.4; //6.4;
   planets[2].rotation_period = 1;
   planets[2].orbit_period = 1;
	planets[2].num_orbiting_objects = 1;
	satellites = new AstronomicalObject[1];
   	planets[2].orbiting_objects = satellites;
	satellites[0].name = "Moon";
	satellites[0].average_orbit_distance = 2;
	satellites[0].equatorial_radius = 0.2; //6.4;
   	satellites[0].rotation_period = 0.1;
   	satellites[0].orbit_period = 0.1;
   // mars
   planets[3].name = "Mars";
	planets[3].average_orbit_distance = 16; //227944;
   planets[3].equatorial_radius = 0.4; //3.4;
   planets[3].rotation_period = 1;
   planets[3].orbit_period = 1.9;
	planets[3].orbiting_objects = NULL;
   // jupiter
   planets[4].name = "Jupiter";
	planets[4].average_orbit_distance = 20; //778341;
   planets[4].equatorial_radius = 0.7; //69.9;
   planets[4].rotation_period = 0.4;
   planets[4].orbit_period = 11.9;
	planets[4].orbiting_objects = NULL;
   // saturn
   planets[5].name = "Saturn";
	planets[5].average_orbit_distance = 24; //1426666;
   planets[5].equatorial_radius = 0.6; //58.2;
   planets[5].rotation_period = 0.4;
   planets[5].orbit_period = 29.4;
	planets[5].orbiting_objects = NULL;
   // uranus
   planets[6].name = "Uranus";
	planets[6].average_orbit_distance = 28;//2870658;
   planets[6].equatorial_radius = 0.6; //25.4;
   planets[6].rotation_period = -0.7;
   planets[6].orbit_period = 84;
	planets[6].orbiting_objects = NULL;	
   // neptune
   planets[7].name = "Neptune";
	planets[7].average_orbit_distance = 32; //4498396;
   planets[7].equatorial_radius = 0.6; //24.6;
   planets[7].rotation_period = 0.7;
   planets[7].orbit_period = 164.8;
	planets[7].orbiting_objects = NULL;
}

/* This function handles idle event so that a face rotation 
 * keeps continuing until 90 degree is completed and
 * in case of randomized rotations, this function will
 * execute continuous face rotations.
 * The skeleton of this function is from the spinCube code
 */
void
idle( void )
{
	int i;
	for(i = 0; i < NUM_OF_OBJECTS; i++) {
		sun.orbiting_objects[i].Theta[Axis] -= EARTH_REVOLUTION_ANGULAR_SPEED / sun.orbiting_objects[i].orbit_period;
    		if ( sun.orbiting_objects[i].Theta[Axis] > 360.0 ) {
        		sun.orbiting_objects[i].Theta[Axis] -= 360.0;
    		}

    		if ( sun.orbiting_objects[i].Theta[Axis] < -360.0 ) {
        		sun.orbiting_objects[i].Theta[Axis] += 360.0;
    		}
		if(sun.orbiting_objects[i].orbiting_objects) {
			int j;
			for(j = 0; j < sun.orbiting_objects[i].num_orbiting_objects; j++) {
				//AstronomicalObject * satellite = &(sun.orbiting_objects[i].orbiting_objects[j]);
				sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis] -= EARTH_REVOLUTION_ANGULAR_SPEED / sun.orbiting_objects[i].orbiting_objects[j].orbit_period;
				//printf("moon is set angular with angular speed %lf\n", sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis]);
    				if ( sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis] > 360.0 ) {
        				sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis] -= 360.0;
    				}

    				if ( sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis] < -360.0 ) {
        				sun.orbiting_objects[i].orbiting_objects[j].Theta[Axis] += 360.0;
    				}
			}
		}	
	}
    	glutPostRedisplay();
}

//----------------------------------------------------------------------------


int Index_s = 0;

void
triangle( const point4& a, const point4& b, const point4& c )
{
    vec3  normal = normalize( cross(b - a, c - b) );

    sphereNormals[Index_s] = normal;  spherePoints[Index_s] = a;  Index_s++;
    sphereNormals[Index_s] = normal;  spherePoints[Index_s] = b;  Index_s++;
    sphereNormals[Index_s] = normal;  spherePoints[Index_s] = c;  Index_s++;
}

point4
unit( const point4& p )
{
    float len = p.x*p.x + p.y*p.y + p.z*p.z;
    
    point4 t;
    if ( len > DivideByZeroTolerance ) {
	t = p / sqrt(len);
	t.w = 1.0;
    }

    return t;
}

void
divide_triangle( const point4& a, const point4& b,
		 const point4& c, int count )
{
    if ( count > 0 ) {
        point4 v1 = unit( a + b );
        point4 v2 = unit( a + c );
        point4 v3 = unit( b + c );
        divide_triangle(  a, v1, v2, count - 1 );
        divide_triangle(  c, v2, v3, count - 1 );
        divide_triangle(  b, v3, v1, count - 1 );
        divide_triangle( v1, v3, v2, count - 1 );
    }
    else {
        triangle( a, b, c );
    }
}

void
tetrahedron( int count )
{
    point4 v[4] = {
	vec4( 0.0, 0.0, 1.0, 1.0 ),
	vec4( 0.0, 0.942809, -0.333333, 1.0 ),
	vec4( -0.816497, -0.471405, -0.333333, 1.0 ),
	vec4( 0.816497, -0.471405, -0.333333, 1.0 )
    };

    divide_triangle( v[0], v[1], v[2], count );
    divide_triangle( v[3], v[2], v[1], count );
    divide_triangle( v[0], v[3], v[1], count );
    divide_triangle( v[0], v[2], v[3], count );
}

//----------------------------------------------------------------------------
// OpenGL initialization
void
init()
{   
    // Subdivide a tetrahedron into a sphere
    tetrahedron( NumTimesToSubdivide );
        
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereNormals), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereNormals), sphereNormals );

    // Load shaders and use the resulting shader program
    program = InitShader( "vshader.glsl", "fshader.glsl" ); 
    glUseProgram( program );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	 GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(spherePoints)) );

    glEnable( GL_DEPTH_TEST );
    glClearColor( 0, 0, 0, 0 );   
    
    initAstronomicalObjects(); 
}

//----------------------------------------------------------------------------
void
display( void )
{	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	double scaleby = 0.04;
	mat4 mv = Scale(scaleby, scaleby, scaleby);
	glUniformMatrix4fv(	glGetUniformLocation( program, "ModelView" ), 1, GL_TRUE, mv );
	glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
	for(int i = 0; i < sun.num_orbiting_objects; i++){
		double t = scaleby * sun.orbiting_objects[i].average_orbit_distance /*/ sun.orbiting_objects[i].equatorial_radius*/;
		double s = scaleby * sun.orbiting_objects[i].equatorial_radius / sun.equatorial_radius;
		GLfloat ZRotationAngle = sun.orbiting_objects[i].Theta[Zaxis];
		mat4 planet_model_view = Translate(t*cos(ZRotationAngle), t*sin(ZRotationAngle), 0); 
		glUniformMatrix4fv(	glGetUniformLocation( program, "ModelView" ), 1, GL_TRUE, planet_model_view * Scale(s, s, s));
		glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
		if(sun.orbiting_objects[i].orbiting_objects) {
			printf("planet %d has satellite(s).\n", i);
			int j;
			for(j = 0; j < sun.orbiting_objects[i].num_orbiting_objects; j++) {
				printf("moon is being created\n");
				AstronomicalObject satellite = sun.orbiting_objects[i].orbiting_objects[j];
				double t_s = scaleby * satellite.average_orbit_distance /*/ sun.orbiting_objects[i].equatorial_radius*/;
				double s_s = scaleby * satellite.equatorial_radius / sun.equatorial_radius;
				ZRotationAngle = satellite.Theta[Zaxis];
				mat4 satellite_model_view = Translate(t_s*cos(ZRotationAngle), t_s*sin(ZRotationAngle), 0);
				glUniformMatrix4fv(	glGetUniformLocation( program, "ModelView" ), 1, GL_TRUE, planet_model_view * satellite_model_view *  Scale(s_s, s_s, s_s));
				glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
			}
		}
	}
	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
	 glViewport( 0, 0, width, height );
    GLfloat left = -1.0, right = 1.0;
    GLfloat top = 1.0, bottom = -1.0;
    GLfloat zNear = -1.0, zFar = 1.0;
    GLfloat aspect = GLfloat(width)/height;
    if ( aspect > 1.0 ) {
		left *= aspect;
		right *= aspect;
    } else {
		top /= aspect;
		bottom /= aspect;
    }
    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( glGetUniformLocation( program, "Projection" ), 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
	switch( key ) {
		case 'h':
			cout << "------Solar System------" << endl;
			cout << "h -- help" <<endl;
			cout << "q -- quit (exit) the program" << endl;
			break;
		case 'q': case 'Q': case 033: // Escape key
			exit( EXIT_SUCCESS );
			break;		
	}
}

//----------------------------------------------------------------------------

void
specialKeyboard( int key, int x, int y )
{
	switch( key ) {
		case GLUT_KEY_UP:
			break;
		case GLUT_KEY_DOWN:
			break;
	}
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
	 // initializing window
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutInitContextVersion( 3, 2 );
    glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow( "Solar System" );
    	 
	 glewExperimental = GL_TRUE;
    glewInit();

    init();

	glutIdleFunc( idle );
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc( specialKeyboard );
    glutMainLoop();
    
    return 0;
}
