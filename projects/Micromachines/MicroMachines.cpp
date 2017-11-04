//
// AVT demo light 
// based on demos from GLSL Core Tutorial in Lighthouse3D.com   
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//

#include <math.h>
#include <iostream>
#include <sstream>

#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// TinyLoader
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "TGA.h"

#define CAPTION "MicroMachines™"
int WindowHandle = 0;
int WinX = 640, WinY = 400;

unsigned int FrameCount = 0;

VSShaderLib shader;

const int objCount = 1;

struct MyMesh mesh[objCount];
int objID=0;


//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;

#define ORTHOGRAPHIC 1
#define TOP 2
#define CHASE 3

int camera = TOP;
	
// Camera Position
float cam[3];

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];

//------------------[ TEXTURES ]------------------//

GLint texMode_UID;
GLint tex_1_loc;
GLint tex_2_loc;

GLuint TextureArray[2];

//-----------------------------------------------//

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000/60, refresh, 0); // 60 fps
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if(h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	if(camera == ORTHOGRAPHIC)
		ortho(-25, 25, -25, 25, 0.1f, 1000.f);
	else if (camera == TOP)
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	else if(camera == CHASE)
		perspective(53.13f, ratio, 0.1f, 1000.0f);
}

// ------------------------------------------------------------
//
// Object movement functions
//


// ------------------------------------------------------------
//
// Render stufff
//

void sendLights() {

	GLint loc;
	float res[4];
	
}

void sendMaterials() {

	GLint loc;

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objID].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objID].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objID].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objID].mat.shininess);
}

void sendMatrices() {
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
}

void render() {

	sendMaterials();
	sendMatrices();

	glBindVertexArray(mesh[objID].vao);
	glDrawElements(mesh[objID].type, mesh[objID].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	loadIdentity(MODEL); //reset model matrix

}

void renderScene(void) {

	//GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt

	if (camera == ORTHOGRAPHIC) {
		/*lookAt(0, 10, 0, 0, 0, 0, 1, 0, 0);*/
	}

	else if (camera == TOP) {
		lookAt(35, 10, 0, 0, 0, 0, 0, 1, 0);
	}

	else if (camera == CHASE) {
		/*lookAt(cam[0], cam[1], cam[2], carPos[0], carPos[1], carPos[2], 0, 1, 0);
		translate(VIEW, carPos[0], carPos[1], carPos[2]);
		rotate(VIEW, camPitch, 0, 0, 1);
		rotate(VIEW, -carAngle + camAngle, 0.0f, 1.0f, 0.0f);
		translate(VIEW, -carPos[0], -carPos[1], -carPos[2]);*/
	}

	// use our shader
	glUseProgram(shader.getProgramIndex());

	sendLights();

	// TEXTURES

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glUniform1i(tex_1_loc, 0);
	glUniform1i(tex_2_loc, 1);

	pushMatrix(MODEL);

	//--------[ Remember: the first transform is the last one coded! ]--------\\



	popMatrix(MODEL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void keyDown(unsigned char key, int xx, int yy)
{
	switch(key) {

		case 27:
			glutLeaveMainLoop();
			break;

		//cameras
		//case '1': camera = ORTHOGRAPHIC; loadIdentity(PROJECTION); ortho(-25, 25, -25, 25, 0.1f, 1000.f); break;
		case '2': camera = TOP; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;
		//case '3': camera = CHASE; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;


		//aliasing settings
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;

		//game controls
		case 'p': paused = !paused; break;
	}
}

void keyUp(unsigned char key, int xx, int yy)
{
	switch (key) {
		case 'w': break;
		case 's': break;
		case 'd': break;
		case 'a': break;
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN)  {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
		}
		else if (tracking == 2) {
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {
	}
	// right mouse button: zoom
	else if (tracking == 2) {
	}


//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/main_shader.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/main_shader.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");

	// texMode = on or off
	/*texMode_UID = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_1_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_2_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap2");*/
	
	printf("InfoLog for Hello World Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	//std::cin.ignore(); //in case of crash
	
	return(shader.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//


void loadMaterials(float* ambient, float* diffuse, float* specular, float* emissive, float shininess, int texCount) {
	memcpy(mesh[objID].mat.ambient, ambient, 4 * sizeof(float));
	memcpy(mesh[objID].mat.diffuse, diffuse, 4 * sizeof(float));
	memcpy(mesh[objID].mat.specular, specular, 4 * sizeof(float));
	memcpy(mesh[objID].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objID].mat.shininess = shininess;
	mesh[objID].mat.texCount = texCount;
}

void init()
{

	/*glGenTextures(2, TextureArray);
	TGA_Texture(TextureArray, "textures/kt_rock_1f_shiny.tga", 0);
	TGA_Texture(TextureArray, "textures/wall_512_1_05.tga", 1);*/


	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char **argv) {

//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

	glutInitContextVersion (3, 3);
	glutInitContextProfile (GLUT_CORE_PROFILE );
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100,100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	//glutIdleFunc(renderScene);

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc ( mouseWheel ) ;
	glutTimerFunc(0,timer,0);
	glutTimerFunc(0, refresh, 0);


//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);

}

