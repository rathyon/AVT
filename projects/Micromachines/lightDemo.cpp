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

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"

#define CAPTION "AVT Light Demo"
int WindowHandle = 0;
int WinX = 1280, WinY = 800;

unsigned int FrameCount = 0;

VSShaderLib shader;

const int objCount = 5;

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
GLint lPos_uniformId;

#define ORTHOGRAPHIC 1
#define TOP 2
#define CHASE 3

int camera = 3;
	
// Camera Position
float camX, camY, camZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];
float lightPos[4] = {4.0f, 6.0f, 2.0f, 1.0f};

//-------------------[ Movement ]-------------------//

float carX = 2.0f;
const float carY = 0.0f; //assuming no height changes in the track
float carZ = 2.0f;

float carOrientation = 0.0f;

bool carForward = false;
bool carReverse = false;
bool carRotateLeft = false;
bool carRotateRight = false;

float carSpeed = 0.1f;
float carAngularSpeed = 2.0f; //10 degrees



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
		ortho(25, -25, -25, 25, 0.1f, 1000.f);
	else if(camera == CHASE)
		perspective(53.13f, ratio, 0.1f, 1000.0f);
}

// ------------------------------------------------------------
//
// Object movement functions
//

void animateCar() {
	if (carForward) {
		carX += carSpeed;
	}
	else if (carReverse) {
		carX -= carSpeed;
	}
	if (carRotateLeft)
		carOrientation += carAngularSpeed;
	else if (carRotateRight)
		carOrientation -= carAngularSpeed;
}


// ------------------------------------------------------------
//
// Render stufff
//

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

	if (camera == ORTHOGRAPHIC)
		lookAt(0, 10, 0, 0, 0, 0, 0, 0, 1);
	//else if(camera == TOP)
	else if(camera == CHASE)
		lookAt(camX, camY, camZ, carX, carY, carZ, 0, 1, 0);

	// use our shader
	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates

	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 
	// Rafael: in other words the light is fixed in screen coords (effectively speaking)

	float res[4];
	multMatrixPoint(VIEW, lightPos,res);   //lightPos definido em World Coord so is converted to eye space (Rafael: eye = view)
	glUniform4fv(lPos_uniformId, 1, res);


	pushMatrix(MODEL);

	//--------[ Remember: the first transform is the last one coded! ]--------\\

	//red X axis
	objID = 0;
	rotate(MODEL, 90, 0.0f, 0.0f, 1.0f);
	translate(MODEL, 0.0f, -0.5f, 0.0f);
	render();

	//green Y axis
	objID = 1;
	translate(MODEL, 0.0f, 0.5f, 0.0f);
	render();

	//blue Z axis
	objID = 2;
	rotate(MODEL, 90, 1.0f, 0.0f, 0.0f);
	translate(MODEL, 0.0f, 0.5f, 0.0f);
	render();
	
	//table
	objID = 3;
	scale(MODEL, 50.0f, 0.5f, 50.0f);
	translate(MODEL, -0.5f, -1.25f, -0.5f);
	render();

	//car
	objID = 4;

	animateCar();

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, carOrientation, 0, 1, 0);
	scale(MODEL, 1.7f, 1, 1);
	translate(MODEL, -0.5f, 0, -0.5f);
	render();

	popMatrix(MODEL);
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

		case 'c': 
			printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
			break;

		//cameras
		case '1': camera = ORTHOGRAPHIC; loadIdentity(PROJECTION); ortho(25, -25, -25, 25, 0.1f, 1000.f); break;
		case '3': camera = CHASE; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX)/WinY, 0.1f, 1000.0f); break;
		

		//car movement
		case 'w': if (!carReverse) carForward = true; break;
		case 's': if (!carForward) carReverse = true; break;
		case 'd': if (!carRotateLeft) carRotateRight = true; break;
		case 'a': if (!carRotateRight) carRotateLeft = true; break;

		//aliasing settings
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;
	}
}

void keyUp(unsigned char key, int xx, int yy)
{
	switch (key) {
		case 'w': carForward = false; break;
		case 's': carReverse = false; break;
		case 'd': carRotateRight = false; break;
		case 'a': carRotateLeft = false; break;
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
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
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


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += -direction * 1.0f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

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
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	
	printf("InfoLog for Hello World Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	
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
	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

	//create 3 cylinders for gizmo

	//red
	float ambRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float diffRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float specRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 1.0f;
	int texCount = 0;

	objID = 0;
	loadMaterials(ambRed, diffRed, specRed, emissive, shininess, texCount);
	createCylinder(1.0f, 0.1f, 20);

	//green
	float ambGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float diffGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float specGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };

	objID = 1;
	loadMaterials(ambGreen, diffGreen, specGreen, emissive, shininess, texCount);
	createCylinder(1.0f, 0.1f, 20);

	//blue
	float ambBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float diffBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float specBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };

	objID = 2;
	loadMaterials(ambBlue, diffBlue, specBlue, emissive, shininess, texCount);
	createCylinder(1.0f, 0.1f, 20);

	//table
	float ambTable[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffTable[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	float specTable[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	shininess = 500.0f;


	objID = 3;
	loadMaterials(ambTable, diffTable, specTable, emissive, shininess, texCount);
	createCube();

	//car hull
	float ambCar[] = { 0,0,0,1.0f };
	float diffCar[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	float specCar[] = { 1,1,1,1 };
	shininess = 700.0f;

	objID = 4;
	loadMaterials(ambCar, diffCar, specCar, emissive, shininess, texCount);
	createCube();

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

