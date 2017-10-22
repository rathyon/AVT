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

// red gizmo
// green gizmo
// blue gizmo
// table
// car
// cheerio
// orange
// butter
// loadtest obj

const int objCount = 9;

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

int camera = CHASE;
	
// Camera Position
float cam[3];

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
//float alpha = 39.0f, beta = 51.0f;
float alpha = -90.0f, beta = 0.0f;
float r = 10.0f;

float camAngle = 0.0f;
float camPitch = 0.0f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];

//------------------[ LIGHTS ]----------------------//

float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float purple[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
float cyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

float dirLightDirection[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
float dirLightColor[4] = { 1.0f, 1.0f , 1.0f , 1.0f};

float candles[6][4];

float redCandle[4] = { -20.0f, 10.0f, 20.0f, 1.0f };
float greenCandle[4] = { 20.0f, 10.0f, 20.0f, 1.0f };
float blueCandle[4] = { -20.0f, 10.0f, -20.0f, 1.0f };
float purpleCandle[4] = { 20.0f, 10.0f, -20.0f, 1.0f };
float cyanCandle[4] = { 0.0f, 10.0f, 0.0f, 1.0f };
float yellowCandle[4] = { 0.0f, 10.0f, 10.0f, 1.0f };

float candleAttenuation = 0.3f;

bool dirLightOn = true;
bool candlesOn = false;

// Game Variables

bool paused = false;

//-------------------[ Movement ]-------------------//

//------ CAR -------//
float carPos[3] = { 2.0f, 0.0f, 2.0f };

float carSpeed[3] = { 0.0f , 0.0f, 0.0f };
float carReverseVec[3] = { 0.03f, 0.0f, 0.0f };

float carAngle = 0.0f;

bool carForward = false;
bool carReverse = false;
bool carRotateLeft = false;
bool carRotateRight = false;

float carAcceleration[3] = { 0.005f, 0.0f, 0.0f };
float carAngularSpeed = 2.0f; //2 degrees
float carRotationAxis[3] = { 0,1,0 }; // rotate "left and right"

float carRadius = 0.85f;

//------ ORANGE ------//

float orangePos[4][3] = { { -20.0f, 1, 0 },{ -10.0f, 1, 0 }, { 10.0f, 1, 0 },{ 20.0f, 1, 0 } };

float orangeSpeed[4][3] = { { 0, 0, -0.005f },{ 0, 0, 0.003f },{ 0, 0, 0.001f },{ 0, 0, -0.004f } };

float orangeAcceleration[4][3] = { { 0, 0, -0.00005f },{ 0, 0, 0.00003f },{ 0, 0, 0.00001f },{ 0, 0, -0.00004f } };

float orangeAngle = 0.0f;
float orangeAngularSpeed = 1.0f;

float orangeRadius = 0.8f;

//------------------[ TEXTURES ]------------------//

GLint texMode_uniformId;
GLint tex_loc1;
GLint tex_loc2;

GLuint TextureArray[2];

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

void animateCar() {

	float res[4];
	float distance = 0.0f;

	bool collided = false;

	if (carForward) {

		for (int i = 0; i < 4; i++) {
			subtract(carPos, orangePos[i], res);
			distance = length(res);
			if (distance <= carRadius + orangeRadius)
				collided = true;
		}

		if (!collided) {
			if (length(carSpeed) < 0.2f)
				add(carSpeed, carAcceleration, carSpeed); // increase speed
			add(carPos, carSpeed, carPos); // move car
			add(cam, carSpeed, cam); // move camera with car
		}
	}
	else if (carReverse) {
		subtract(carReverseVec, carPos, carPos); // move car
		subtract(carReverseVec, cam, cam); // move camera with car
	}

	if (carRotateLeft) {
		carAngle += carAngularSpeed;
		rotate(carSpeed, carAngularSpeed, carRotationAxis);
		rotate(carAcceleration, carAngularSpeed, carRotationAxis);
		rotate(carReverseVec, carAngularSpeed, carRotationAxis);
	}
	else if (carRotateRight) {
		carAngle -= carAngularSpeed;
		rotate(carSpeed, -carAngularSpeed, carRotationAxis);
		rotate(carAcceleration, -carAngularSpeed, carRotationAxis);
		rotate(carReverseVec, -carAngularSpeed, carRotationAxis);
	}
}

void respawnOrange(int i) {

	float RNG = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // between 0.0 and 1.0

	RNG *= 2.0f;

	RNG -= 1.0f;

	RNG *= 25.0f;

	//std::cout << RNG << std::endl;

	orangePos[i][0] = RNG;
	orangePos[i][1] = 1.0f;
	orangePos[i][2] = 0.0f;

}

void animateOranges() {

	for (int i = 0; i < 4; i++) {

		if (orangePos[i][2] > 25 || orangePos[i][2] < -25) {
			orangePos[i][1] = -100.0f;
			//glutTimerFunc(2000, respawnOrange, i);
			respawnOrange(i);
		}
		else {
			add(orangeSpeed[i], orangeAcceleration[i], orangeSpeed[i]);
			add(orangePos[i], orangeSpeed[i], orangePos[i]);
			orangeAngle += orangeAngularSpeed;
		}
	}

}


// ------------------------------------------------------------
//
// Render stufff
//

void sendLights() {

	GLint loc;
	float res[4];

	// Directional Light
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[0].position");
	multMatrixPoint(VIEW, dirLightDirection, res);   //Not doing this = light position is in camera coords
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[0].color");
	glUniform4fv(loc, 1, dirLightColor);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[0].isEnabled");
	glUniform1i(loc, dirLightOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[0].isPointLight");
	glUniform1i(loc, false);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[0].isSpotlight");
	glUniform1i(loc, false);

	// Red Candle
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[1].position");
	multMatrixPoint(VIEW, redCandle, res);   //Not doing this = light position is in camera coords
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[1].color");
	glUniform4fv(loc, 1, red);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[1].isEnabled");
	glUniform1i(loc, candlesOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[1].isPointLight");
	glUniform1i(loc, true); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[1].isSpotlight");
	glUniform1i(loc, false); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[1].linearAttenuation");
	glUniform1f(loc, candleAttenuation);
	/*
	// Green Candle
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[2].position");
	multMatrixPoint(VIEW, greenCandle, res);
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[2].color");
	glUniform4fv(loc, 1, green);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[2].isEnabled");
	glUniform1i(loc, candlesOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[2].isPointLight");
	glUniform1i(loc, true); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[2].isSpotlight");
	glUniform1i(loc, false); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[2].linearAttenuation");
	glUniform1f(loc, candleAttenuation);

	// Blue Candle
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[3].position");
	multMatrixPoint(VIEW, blueCandle, res);
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[3].color");
	glUniform4fv(loc, 1, blue);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[3].isEnabled");
	glUniform1i(loc, candlesOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[3].isPointLight");
	glUniform1i(loc, true); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[3].isSpotlight");
	glUniform1i(loc, false); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[3].linearAttenuation");
	glUniform1f(loc, candleAttenuation);

	// Purple Candle
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[4].position");
	multMatrixPoint(VIEW, purpleCandle, res);
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[4].color");
	glUniform4fv(loc, 1, purple);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[4].isEnabled");
	glUniform1i(loc, candlesOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[4].isPointLight");
	glUniform1i(loc, true); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[4].isSpotlight");
	glUniform1i(loc, false); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[4].linearAttenuation");
	glUniform1f(loc, candleAttenuation);

	// Cyan Candle
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[5].position");
	multMatrixPoint(VIEW, cyanCandle, res);
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[5].color");
	glUniform4fv(loc, 1, cyan);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[5].isEnabled");
	glUniform1i(loc, candlesOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[5].isPointLight");
	glUniform1i(loc, true); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[5].isSpotlight");
	glUniform1i(loc, false); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[5].linearAttenuation");
	glUniform1f(loc, candleAttenuation);

	// Yellow Candle
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[6].position");
	multMatrixPoint(VIEW, yellowCandle, res); 
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[6].color");
	glUniform4fv(loc, 1, yellow);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[6].isEnabled");
	glUniform1i(loc, candlesOn);
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[6].isPointLight");
	glUniform1i(loc, true); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[6].isSpotlight");
	glUniform1i(loc, false); //false
	loc = glGetUniformLocation(shader.getProgramIndex(), "Lights[6].linearAttenuation");
	glUniform1f(loc, candleAttenuation);

	*/
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
		lookAt(0, 10, 0, 0, 0, 0, 1, 0, 0);
	}

	else if (camera == TOP) {
		lookAt(35, 10, 0, 0, 0, 0, 0, 1, 0);
	}

	else if (camera == CHASE) {
		lookAt(cam[0], cam[1], cam[2], carPos[0], carPos[1], carPos[2], 0, 1, 0);
		translate(VIEW, carPos[0], carPos[1], carPos[2]);
		rotate(VIEW, camPitch, 0, 0, 1);
		rotate(VIEW, -carAngle + camAngle, 0.0f, 1.0f, 0.0f);
		translate(VIEW, -carPos[0], -carPos[1], -carPos[2]);
	}

	// use our shader
	glUseProgram(shader.getProgramIndex());

	sendLights();

	// TEXTURES

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glUniform1i(tex_loc1, 0);
	glUniform1i(tex_loc2, 1);

	pushMatrix(MODEL);

	//--------[ Remember: the first transform is the last one coded! ]--------\\

	glUniform1i(texMode_uniformId, 0);

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
	glUniform1i(texMode_uniformId, 1);
	render();

	glUniform1i(texMode_uniformId, 0);

	//car
	objID = 4;

	if(!paused)
		animateCar();

	translate(MODEL, carPos[0], carPos[1], carPos[2]);
	rotate(MODEL, carAngle, 0, 1, 0);
	scale(MODEL, 1.7f, 1, 1);
	translate(MODEL, -0.5f, 0, -0.5f);
	render();

	//cheerios
	objID = 5;

	for (int i = 0; i < 40;i++) {
		rotate(MODEL, i*9.0f, 0, 1, 0);
		translate(MODEL, 20, 0, 0);
		render();
	}

	for (int i = 0; i < 30;i++) {
		rotate(MODEL, i*12.0f, 0, 1, 0);
		translate(MODEL, 10, 0, 0);
		render();
	}

	//oranges
	objID = 6;

	if(!paused)
		animateOranges();

	translate(MODEL, orangePos[0][0], orangePos[0][1], orangePos[0][2]);
	rotate(MODEL, -orangeAngle, 1, 0, 0);
	render();

	translate(MODEL, orangePos[1][0], orangePos[1][1], orangePos[1][2]);
	rotate(MODEL, orangeAngle, 1, 0, 0);
	render();

	translate(MODEL, orangePos[2][0], orangePos[2][1], orangePos[2][2]);
	rotate(MODEL, orangeAngle, 1, 0, 0);
	render();

	translate(MODEL, orangePos[3][0], orangePos[3][1], orangePos[3][2]);
	rotate(MODEL, -orangeAngle, 1, 0, 0);
	render();

	//butter
	objID = 7;

	translate(MODEL, -15, 0, 0);
	scale(MODEL, 2, 0.5, 1);

	render();
	//test object
	/*objID = 8;

	translate(MODEL, 0, 2, 0);
	//scale(MODEL, 10, 10, 10);
	render();*/

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

		case 'o': 
			printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
			break;

		//cameras
		case '1': camera = ORTHOGRAPHIC; loadIdentity(PROJECTION); ortho(-25, 25, -25, 25, 0.1f, 1000.f); break;
		case '2': camera = TOP; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;
		case '3': camera = CHASE; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;

		//lights
		case 'l': 
			dirLightOn = !dirLightOn; break;
		case 'c':
			candlesOn = !candlesOn; break;
		

		//car movement
		case 'w': if (!carReverse) carForward = true; break;
		case 's': if (!carForward) carReverse = true; break;
		case 'd': if (!carRotateLeft) carRotateRight = true; break;
		case 'a': if (!carRotateRight) carRotateLeft = true; break;

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
		case 'w': 
			carForward = false; 
			carSpeed[0] = 0; carSpeed[1] = 0; carSpeed[2] = 0;
			break;
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


		/*alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;*/

		camAngle -= deltaX*0.002f;
		camPitch += deltaY*0.002f;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	/*cam[0] = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f) ;
	cam[1] = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f) ;
	cam[2] = rAux *   						       sin(betaAux * 3.14f / 180.0f) ;*/

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += -direction * 1.0f;
	if (r < 0.1f)
		r = 0.1f;

	cam[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cam[1] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cam[2] = r *   						     sin(beta * 3.14f / 180.0f);

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

	// texMode = on or off
	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	
	printf("InfoLog for Hello World Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	//std::cin.ignore(); //in case of crash
	
	return(shader.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

// Example of a loading of an OBJ using TinyLoader -> will have to setup buffers manually!
bool LoadObj(const char* filename, const char* basepath = NULL, bool triangulate = true) {
	std::cout << "Loading " << filename << std::endl;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename,
		basepath, triangulate);

	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return false;
	}

	//OPENGL stuff

	int faceIndexCount = 0;

	for (size_t i = 0; i < shapes.size(); i++) {
		std::cout << "Shape " << i+1 << ":" << std::endl;
		for (size_t j = 0; j < shapes[i].mesh.indices.size(); j++) {
			std::cout << shapes[i].mesh.indices[j].vertex_index << std::endl;
			faceIndexCount++;
		}
	}

	mesh[objID].numIndexes = faceIndexCount;
	glGenVertexArrays(1, &(mesh[objID].vao));
	glBindVertexArray(mesh[objID].vao);

	GLuint VboId[2];

	float* vertices = &attrib.vertices[0];
	float* normals = &attrib.normals[0];
	float* texcoords = &attrib.texcoords[0];

	unsigned int* faceIndex = new  unsigned int[faceIndexCount];

	int aux = 0;

	std::cout << "Assigning values to faceIndex" << std::endl;
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t j = 0; j < shapes[i].mesh.indices.size(); j++) {
			faceIndex[aux++] = shapes[i].mesh.indices[j].vertex_index;
		}
	}

	for (size_t i = 0; i < faceIndexCount; i++) {
		std::cout << faceIndex[i] << std::endl;
	}


	glGenBuffers(2, VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId[0]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(attrib.vertices) + sizeof(attrib.normals) + sizeof(attrib.texcoords), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), sizeof(texcoords), texcoords);

	glEnableVertexAttribArray(VERTEX_COORD_ATTRIB);
	glVertexAttribPointer(VERTEX_COORD_ATTRIB, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(NORMAL_ATTRIB);
	glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, 0, 0, (void *)sizeof(vertices));
	glEnableVertexAttribArray(TEXTURE_COORD_ATTRIB);
	glVertexAttribPointer(TEXTURE_COORD_ATTRIB, 2, GL_FLOAT, 0, 0, (void *)(sizeof(vertices) + sizeof(normals)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VboId[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh[objID].numIndexes, faceIndex, GL_STATIC_DRAW);

	delete(faceIndex);

	glBindVertexArray(0);

	mesh[objID].type = GL_TRIANGLES;

	std::cout << "Model successfully loaded!" << std::endl;

	return true;
}

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
	/*cam[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f) + carPos[0];
	cam[1] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f) + carPos[1];
	cam[2] = r *   						     sin(beta * 3.14f / 180.0f) + carPos[2];*/

	cam[0] = carPos[0] - 6.0f;
	cam[1] = carPos[1] + 6.0f;
	cam[2] = carPos[2];

	glGenTextures(2, TextureArray);
	TGA_Texture(TextureArray, "textures/kt_rock_1f_shiny.tga", 0);
	TGA_Texture(TextureArray, "textures/wall_512_1_05.tga", 1);

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

	//cheerios
	float ambCheerios[] = { 0,0,0,1 };
	float diffCheerios[] = { 1,0.7f,0,1 };
	float specCheerios[] = { 1,1,1,1 };
	shininess = 5.0f;

	objID = 5;
	loadMaterials(ambCheerios, diffCheerios, specCheerios, emissive, shininess, texCount);
	createTorus(0.15f, 0.4f, 32, 16);

	//oranges
	float diffOranges[] = { 1, 0.5f, 0, 1 };
	objID = 6;
	loadMaterials(ambCheerios, diffOranges, specCheerios, emissive, shininess, texCount);
	createSphere(1, 60);

	//butter
	float diffButter[] = { 1,1,0,1 };
	objID = 7;
	loadMaterials(ambCheerios, diffButter, specCheerios, emissive, shininess, texCount);
	createCube();

	//test obj
	/*objID = 8;
	loadMaterials(ambRed, diffRed, specRed, emissive, shininess, texCount);
	if (!LoadObj("models/cube.obj", "models/", true)) {
		std::cerr << "Error loading model!" << std::endl;
	}*/


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

