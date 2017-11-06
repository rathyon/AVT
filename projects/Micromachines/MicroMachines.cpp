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

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "TGA.h"

#define PI   3.14159265358979323846f

#define CAPTION "MicroMachines™"
int WindowHandle = 0;
int WinX = 640, WinY = 400;

unsigned int FrameCount = 0;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];

VSShaderLib shader;
GLuint pid;

// table = 0
// test cube = 1
// cheerios = 2
const int objCount = 3;

struct MyMesh mesh[objCount];
int objID = 0;


//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;

float axisY[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	
//------------------[ CAMERAS ]------------------//

#define ORTHOGRAPHIC 1
#define TOP 2
#define CHASE 3

int camera = CHASE;

float top[3];
float camX, camY, camZ;
float alpha = 0.0f;
float beta = 0.0f;
float r = 5.0f;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

//------------------[ CAR ]------------------//

float carPos[3] = { 30.0f, 0.0f, 0.0f };
float carDir[3] = { 0.0f, 0.0f, 1.0f };
float carSpeed = 0.0f;
float carAcc = 0.05f;
float carBrake = 0.5f;
float carReverse = 0.02f;
float carAngularSpeed = 3.0f;
float carAngle = 0.0f;
float speedLimit = 5.0f;
float epsilon = 0.3f;


bool carIsForward = false;
bool carIsReverse = false;;
bool carIsLeft = false;
bool carIsRight = false;

//------------------[ CHEERIOS ]------------------//

#define NUMBER_INNER_CHEERIOS 20.0f
#define NUMBER_OUTER_CHEERIOS 20.0f
#define NUMBER_CHEERIOS 40

float cheerioPos[NUMBER_CHEERIOS][4];
float cheerioSide = 1.0f;

//------------------[ LIGHTS ]------------------//

float dirLight[4] = { 0.5f, 1.0f, 0.0f, 0.0f };
float dirLightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

//------------------[ TEXTURES ]------------------//

GLint texMode_UID;
GLint tex_1_loc;

GLuint TextureArray[1];

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
	WinX = w;
	WinY = h;
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	if(camera == ORTHOGRAPHIC)
		ortho(-50, 50, -50, 50, 0.1f, 1000.f);
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
void animateCar() {
	float aux[3];

	if (carIsForward) {
		if(carSpeed < speedLimit)
			carSpeed += carAcc;
		aux[0] = carDir[0] * carSpeed;
		aux[1] = carDir[1] * carSpeed;
		aux[2] = carDir[2] * carSpeed;
		add(carPos, aux, carPos);
	}

	if (carIsReverse) {
		if (carSpeed > -speedLimit)
			carSpeed -= carReverse;
		aux[0] = carDir[0] * carSpeed;
		aux[1] = carDir[1] * carSpeed;
		aux[2] = carDir[2] * carSpeed;
		add(carPos, aux, carPos);
	}

	if (!carIsReverse && !carIsForward) {
		if (carSpeed < epsilon || carSpeed > -epsilon)
			carSpeed = 0;
		else if (carSpeed > 0)
			carSpeed -= carBrake;
		else
			carSpeed += carBrake;
	}


	if (carIsLeft) {
		rotate(carDir, carAngularSpeed, axisY);
		carAngle += carAngularSpeed;
	}

	else if (carIsRight) {
		rotate(carDir, -carAngularSpeed, axisY);
		carAngle -= carAngularSpeed;
	}


}

void sendLights() {

	GLint loc;
	float res[4];

	loc = glGetUniformLocation(pid, "Lights[0].isEnabled");
	glUniform1i(loc, true);
	loc = glGetUniformLocation(pid, "Lights[0].isPointLight");
	glUniform1i(loc, false);
	loc = glGetUniformLocation(pid, "Lights[0].position");
	glUniform4fv(loc, 1, dirLight);
	loc = glGetUniformLocation(pid, "Lights[0].color");
	glUniform4fv(loc, 1, dirLightColor);

}

void render() {

	GLint loc;

	// send materials
	loc = glGetUniformLocation(pid, "mat.ambient");
	glUniform4fv(loc, 1, mesh[objID].mat.ambient);
	loc = glGetUniformLocation(pid, "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objID].mat.diffuse);
	loc = glGetUniformLocation(pid, "mat.specular");
	glUniform4fv(loc, 1, mesh[objID].mat.specular);
	loc = glGetUniformLocation(pid, "mat.shininess");
	glUniform1f(loc, mesh[objID].mat.shininess);

	//send matrices
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	//draw
	glBindVertexArray(mesh[objID].vao);
	glDrawElements(mesh[objID].type, mesh[objID].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	loadIdentity(MODEL); //reset model matrix

}

void renderScene(void) {

	pid = shader.getProgramIndex();

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt

	if (camera == ORTHOGRAPHIC) {
		lookAt(0, 10, 0, 0, 0, 0, 1, 0, 0);
	}

	else if (camera == TOP) {
		lookAt(65, 10, 0, 0, 0, 0, 0, 1, 0);
	}

	else if (camera == CHASE) {
		lookAt(camX + carPos[0], camY + carPos[1] + 0.0f, camZ + carPos[2], carPos[0], carPos[1], carPos[2], 0, 1, 0);
	}

	// use our shader
	glUseProgram(shader.getProgramIndex());

	sendLights();

	// TEXTURES

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glUniform1i(tex_1_loc, 0);
	//glUniform1i(tex_2_loc, 1);

	pushMatrix(MODEL);

	animateCar();

	//--------[ Remember: the first transform is the last one coded! ]--------\\

	// Table
	objID = 0;
	scale(MODEL, 100.0f, 0.25f, 100.0f);
	translate(MODEL, -0.5f, -0.5f, -0.5f);
	glUniform1i(texMode_UID, 1);
	render();
	glUniform1i(texMode_UID, 0);

	// TestCube
	objID = 1;
	translate(MODEL, carPos[0], carPos[1], carPos[2]);
	rotate(MODEL, carAngle, 0.0f, 1.0f, 0.0f);
	translate(MODEL, -0.5f, 0.0f, -0.5f);
	render();

	// Cheerios
	objID = 2;
	for (int i = 0; i < NUMBER_CHEERIOS; i++) {
		translate(MODEL, cheerioPos[i][0], cheerioPos[i][1], cheerioPos[i][2]);
		render();
	}


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
		case '1': camera = ORTHOGRAPHIC; loadIdentity(PROJECTION); ortho(-50, 50, -50, 50, 0.1f, 1000.f); break;
		case '2': camera = TOP; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;
		case '3': camera = CHASE; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;


		//aliasing settings
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;

		//game controls
		//case 'p': paused = !paused; break;

		case 'w': carIsForward = true;  break;
		case 's': carIsReverse = true;  break;
		case 'd': carIsRight = true;  break;
		case 'a': carIsLeft = true;  break;
	}
}

void keyUp(unsigned char key, int xx, int yy)
{
	switch (key) {
	case 'w': carIsForward = false;  break;
	case 's': carIsReverse = false;  break;
	case 'd': carIsRight = false;  break;
	case 'a': carIsLeft = false;  break;
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
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

	deltaX = -xx + startX;
	deltaY = yy - startY;

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

	camX = rAux * sin(alphaAux * PI / 180.0f) * cos(betaAux * PI / 180.0f);
	camZ = rAux * cos(alphaAux * PI / 180.0f) * cos(betaAux * PI / 180.0f);
	camY = rAux * sin(betaAux * PI / 180.0f);


//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r -= direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * PI / 180.0f) * cos(beta * PI / 180.0f);
	camZ = r * cos(alpha * PI / 180.0f) * cos(beta * PI / 180.0f);
	camY = r * sin(beta * PI / 180.0f);

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
	texMode_UID = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_1_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	//tex_2_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	
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

	//setup camera
	camX = r * sin(alpha * PI / 180.0f) * cos(beta * PI / 180.0f);
	camZ = r * cos(alpha * PI / 180.0f) * cos(beta * PI / 180.0f);
	camY = r * sin(beta * PI / 180.0f);

	glGenTextures(1, TextureArray);
	TGA_Texture(TextureArray, "textures/wall_512_1_05.tga", 0);

	//Model init

	//Cheerio position init
	float innerStep = 360.0f / NUMBER_INNER_CHEERIOS;
	for (int i = 0; i < NUMBER_INNER_CHEERIOS; i++) {
		float aux[4] = { 20.0f, 0.5f, 0.0f, 1.0f };
		float axisY[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
		rotate(aux, (float)i*innerStep, axisY);
		cheerioPos[i][0] = aux[0];
		cheerioPos[i][1] = aux[1];
		cheerioPos[i][2] = aux[2];
		cheerioPos[i][3] = aux[3];
	}

	float outerStep = 360.0f / NUMBER_OUTER_CHEERIOS;
	for (int i = (int)NUMBER_INNER_CHEERIOS; i < NUMBER_CHEERIOS; i++) {
		float aux[4] = { 40.0f, 0.5f, 0.0f, 1.0f };
		float axisY[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
		rotate(aux, (float)i*outerStep, axisY);
		cheerioPos[i][0] = aux[0];
		cheerioPos[i][1] = aux[1];
		cheerioPos[i][2] = aux[2];
		cheerioPos[i][3] = aux[3];
	}




	// Table
	objID = 0;

	float ambTable[4] = { 0.2f, 0.1f, 0.0f, 1.0f };
	float diffTable[4] = { 0.3f, 0.3f, 0.0f, 1.0f };
	float specTable[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float non_emissive[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float shininess = 10.0f;
	int texCount = 0;

	loadMaterials(ambTable, diffTable, specTable, non_emissive, shininess, texCount);
	createCube();

	// test cube
	objID = 1;

	float ambTestCube[4] = { 0.1f, 0.0f, 0.1f, 1.0f };
	float diffTestCube[4] = { 0.5f, 0.0f, 0.5f, 1.0f };
	float specTestCube[4] = { 0.5f, 0.0f, 0.5f, 1.0f };
	float null[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	loadMaterials(ambTestCube, diffTestCube, specTestCube, null, shininess, texCount);
	createCube();

	// Cheerios

	float ambCheerio[4] = { 0.55f, 0.3f, 0.0f, 1.0f };
	float diffCheerio[4] = { 0.55f, 0.3f, 0.0f, 1.0f };
	float specCheerio[4] = { 0.55f, 0.3f, 0.0f, 1.0f };

	objID = 2;
	loadMaterials(ambCheerio, diffCheerio, specCheerio, null, shininess, texCount);
	createTorus(0.5f, 1.0f, 10, 10);

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

