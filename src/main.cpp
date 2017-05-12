/*
 * Najeeb Ahmad
 * Muhammad Aditya Sasongko
 * Burak Bastem
 *
 * 12 May 2017
 *
 * Acknowledgement: Based on code provided by 
 * 	https://www.cs.unm.edu/~angel/BOOK/INTERACTIVE_COMPUTER_GRAPHICS/SIXTH_EDITION/
 */
  
#include "Angel.h"
#include <math.h>
#include <iostream>
using namespace std;

typedef Angel::vec4  point4;
typedef Angel::vec4  color4;

const int NumTimesToSubdivide = 5;
const int NumTrianglesSphere  = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVerticesSphere   = 3 * NumTrianglesSphere;
point4 spherePoints[NumVerticesSphere];
vec3   sphereNormals[NumVerticesSphere];

GLuint program;

// model-view matrices
mat4 model_views[10];
// 0 - Sun
// 1 - Mercury
// 2 - Venus
// 3 - Earth
// 4 - Mars
// 5 - Jupiter
// 6 - Saturn
// 7 - Uranus
// 8 - Neptune
// 9 - Pluto is it a planet?

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
    glClearColor( 0, 0, 0	, 0 );    
}

//----------------------------------------------------------------------------
void
display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	mat4 mv = Scale(0.2, 0.2, 0.2);
	for(int i = 0; i < 10; i++){
		model_views[i] = mv;
		glUniformMatrix4fv(	glGetUniformLocation( program, "ModelView" ), 1, GL_TRUE, model_views[0] * model_views[i] );
		glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );		
		mv = Translate(0.5, 0, 0) * mv;
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

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc( specialKeyboard );
    glutMainLoop();
    
    return 0;
}
