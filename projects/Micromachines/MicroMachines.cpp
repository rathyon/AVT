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
#include <fstream>
#include <string>
#include <map>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Model loading lib
#include "Mesh.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "TGA.h"
#include "l3DBillboard.h"

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
// orange = 3
// particle = 4
// flare element = 5
// billboard = 6
const int objCount = 7;

struct MyMesh mesh[objCount];
int objID = 0;

// level = 0
int meshID = 0;
std::vector<Mesh> objMesh;

float lamp_spin = 0.0f;


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
float alpha = 0.0f; // horizontal angle
float beta = 30.0f; // vertical angle
float r = 15.0f;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

//------------------[ CAR ]------------------//

float const carInitPos[3] = { 30.0f, 0.0f, 0.0f };
float carPos[3] = { 30.0f, 0.0f, 0.0f };
float carDim[2] = { 1.0f, 1.0f };
float const carInitDir[3] = { 0.0f, 0.0f, -1.0f };
float carDir[3] = { 0.0f, 0.0f, -1.0f };

float carSpeed = 0.0f;
float carAcc = 0.002f;
float carTraction = 0.001f;
float carBrake = 0.005f;
float carReverse = 0.001f;
float carAngularSpeed = 1.5f;
float carAngle = 0.0f;
float speedLimit = 0.3f;


bool carIsForward = false;
bool carIsReverse = false;
bool carIsLeft = false;
bool carIsRight = false;

//------------------[ GAME LOGIC ]------------------//

float checkPoint0[3] = { 30.0f, 0.0f, 0.0f };
float checkPoint1[3] = { 0.0f, 0.0f, -30.0f };
float checkPoint2[3] = { -30.0f, 0.0f, 0.0f };
float checkPoint3[3] = { 0.0f, 0.0f, 30.0f };

float checkPointSize0And2[2] = { 10.5f, 1.0f };
float checkPointSize1And3[2] = { 1.0f, 10.5f };

int currentCheckPoint = 0;
int score = 0;

int lives = 5;
float livesPos[5][3];

bool isGameOver = false;
bool isPaused = false;

//------------------[ CHEERIOS ]------------------//

#define NUMBER_INNER_CHEERIOS 20.0f
#define NUMBER_OUTER_CHEERIOS 20.0f
#define NUMBER_CHEERIOS 40

float cheerioPos[NUMBER_CHEERIOS][4];
float cheerioDir[NUMBER_CHEERIOS][3] = { };
float cheerioSpeed[NUMBER_CHEERIOS] = { };
float cheerioTraction = 0.003f;
float cheerioDim[2] = { 1.0f, 1.0f };

//------------------[ ORANGES ]------------------//

#define NUMBER_ORANGES 5

float orangePos[NUMBER_ORANGES][4];
float orangeSpeed[NUMBER_ORANGES];
float orangeAcceleration[NUMBER_ORANGES];
float orangeDim[2] = { 2.0f, 2.0f };

//------------------[ BILLBOARD ]------------------//

float billboardAngle = alpha;

//------------------[ LIGHTS ]------------------//

float dirLight[4] = { 0.5f, 1.0f, 0.0f, 0.0f };
float dirLightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
bool dirLightOn = true;

bool headlightsOn = true;
float const spotLightInitPos[4] = { carPos[0], carPos[1], carPos[2], carPos[3] };
float const spotLightInitDir_1[3] = { -1.0f, 0.5f, -1.0f };
float const spotLightInitDir_2[3] = { 1.0f, 0.5f, -1.0f };
float spotLightPos[4] = { carPos[0], carPos[1], carPos[2], carPos[3] };
float spotLightDir_1[3] = { -1.0f, 0.5f, -1.0f };
float spotLightDir_2[3] = { 1.0f, 0.5f, -1.0f };
float spotLightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float spotCosCutoff = 0.97f;
float spotExponent = 0.01f;
float linearAttenuation = 0.01f;

bool candlesOn = true;
float candlePos[6][4] = {
	{-40.0f, 10.0f, -40.0f, 1.0f},
	{ -40.0f, 10.0f, 40.0f, 1.0f },
	{ 40.0f, 10.0f, -40.0f, 1.0f },
	{ 40.0f, 10.0f, 40.0f, 1.0f },
	{ 30.0f, 10.0f, 0.0f, 1.0f },
	{ -30.0f, 10.0f, 0.0f, 1.0f }
};
float candleColor[4] = { 0.0f, 0.5f, 1.0f, 1.0f };

float candleAttenuation = 0.2f;

//------------------[ PARTICLES ]------------------//

#define MAX_PARTICLES 20
#define frand() ((float)rand() / RAND_MAX)

float particlePos[MAX_PARTICLES][3];
float particleLife[MAX_PARTICLES];
float particleFade[MAX_PARTICLES];
float particleRGB[3] = { 0.882f, 0.552f, 0.211f };
float particleSpeed[MAX_PARTICLES][3];
float particleAcc[3] = { 0.1f, -0.15f, 0.0f };

bool drawParticles = false;
int deadParticles = 0;

//------------------[ LENS FLARE ]------------------//
#define FLARE_ELEMENTS 5

float lensPos[FLARE_ELEMENTS] = { 0.0f, 0.3f, 0.4f, 0.8f, 1.0f };
float lensScale[FLARE_ELEMENTS] = { 1.0f, 0.3f, 0.5f, 0.3f, 0.8f };
float lensDefaultSourcePos[2] = { 0.8f, 0.8f };
float lensSourcePos[2] = { 0.8f, 0.8f };

//------------------[ TEXTURES ]------------------//

GLint texMode_UID;
GLint tex_1_loc;
GLint tex_2_loc;

//GLint tex_2_loc;
GLuint TextureArray[5];

//------------------[ AUXILIARY FUNCS ]------------------//

//returns float between 0.0f and 1.0f
float RNG() {
	float RNG = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return RNG;
}


void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << " Score - " << score << " : " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
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

void initParticles()
{
	float v, theta, phi;

	for (int i = 0; i < MAX_PARTICLES; i++)
	{

		v = 0.3f * frand() + 0.2f;
		phi = frand() * PI;
		theta = 2.0f * frand() * PI;


		particlePos[i][0] = carPos[0];
		particlePos[i][1] = carPos[1];
		particlePos[i][2] = carPos[2];

		particleSpeed[i][0] = v * cos(theta) * sin(phi);
		particleSpeed[i][1] = v * cos(phi);
		particleSpeed[i][2] = v * sin(theta) * sin(phi);

		particleLife[i] = 1.0f;
		particleFade[i] = 0.005f;
	}
}

void iterate(int value)
{
	int i;
	float h;

	/* Método de Euler de integração de eq. diferenciais ordinárias
	h representa o step de tempo; dv/dt = a; dx/dt = v; e conhecem-se os valores iniciais de x e v */
	h = 0.125f;
	if (!isPaused) {
		for (i = 0; i < MAX_PARTICLES; i++)
		{
			particlePos[i][0] += h*particleSpeed[i][0];
			particlePos[i][1] += h*particleSpeed[i][1];
			particlePos[i][2] += h*particleSpeed[i][2];
			particleSpeed[i][0] += h*particleAcc[0];
			particleSpeed[i][1] += h*particleAcc[1];
			particleSpeed[i][2] += h*particleAcc[2];
			particleLife[i] -= particleFade[i];
		}
	}

	glutTimerFunc(33, iterate, 0);

}

bool hasCollided(float* obj1Pos, float* obj1Dim, float* obj2Pos, float* obj2Dim) {
	if (abs(obj1Pos[0] - obj2Pos[0]) > (obj1Dim[0] + obj2Dim[0])) return false;
	if (abs(obj1Pos[2] - obj2Pos[2]) > (obj1Dim[1] + obj2Dim[1])) return false;

	return true;
}

void updateScore() {

	switch (currentCheckPoint) {
	case 0:  // if car's last checkpoint was starting line

		if (hasCollided(carPos, carDim, checkPoint1, checkPointSize1And3)) {
			currentCheckPoint++;
		}

		break;
	case 1:

		if (hasCollided(carPos, carDim, checkPoint2, checkPointSize0And2)) {
			currentCheckPoint++;
		}

		break;
	case 2:

		if (hasCollided(carPos, carDim, checkPoint3, checkPointSize1And3)) {
			currentCheckPoint++;
		}

		break;
	case 3:

		if (hasCollided(carPos, carDim, checkPoint0, checkPointSize0And2)) {
			currentCheckPoint = 0;
			score++;
			drawParticles = true;
			initParticles();
		}

		break;
	}
}
void resetCar() {
	carSpeed = 0.0f;
	carAngle = 0.0f;

	carDir[0] = carInitDir[0];
	carDir[1] = carInitDir[1];
	carDir[2] = carInitDir[2];

	carPos[0] = carInitPos[0];
	carPos[1] = carInitPos[1];
	carPos[2] = carInitPos[2];

	spotLightPos[0] = spotLightInitPos[0];
	spotLightPos[1] = spotLightInitPos[1];
	spotLightPos[2] = spotLightInitPos[2];

	spotLightDir_1[0] = spotLightInitDir_1[0];
	spotLightDir_1[1] = spotLightInitDir_1[1];
	spotLightDir_1[2] = spotLightInitDir_1[2];

	spotLightDir_2[0] = spotLightInitDir_2[0];
	spotLightDir_2[1] = spotLightInitDir_2[1];
	spotLightDir_2[2] = spotLightInitDir_2[2];
}

void resetCheerios() {
	float innerStep = 360.0f / NUMBER_INNER_CHEERIOS;
	for (int i = 0; i < NUMBER_INNER_CHEERIOS; i++) {
		float aux[4] = { 20.0f, 0.5f, 0.0f, 1.0f };
		float axisY[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
		rotate(aux, (float)i*innerStep, axisY);
		cheerioPos[i][0] = aux[0];
		cheerioPos[i][1] = aux[1];
		cheerioPos[i][2] = aux[2];
		cheerioPos[i][3] = aux[3];
		cheerioSpeed[i] = 0.0f;
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
		cheerioSpeed[i] = 0.0f;

	}
}

void resetOranges() {
	for (int i = 0; i < NUMBER_ORANGES; i++) {
		orangePos[i][0] = -50.0f;
		orangePos[i][1] = 1.0f;
		orangePos[i][2] = -30.0f + (15.0f * (float)i);
		orangePos[i][3] = 1.0f;

		orangeSpeed[i] = RNG() * 0.6f;
		orangeAcceleration[i] = RNG() * 0.0001f;
	}
}

void setOrthogonalProjectionForHUD(int w, int h) {
	pushMatrix(PROJECTION);
	pushMatrix(MODEL);
	pushMatrix(VIEW);

	loadIdentity(PROJECTION);

	float ratio = (float)w / h;
	if (ratio > 1) {
		ortho(-12.5f * ratio, 12.5f * ratio, -12.5f, 12.5f, -2.0f, 10.0f);
	}
	else {
		ortho(-12.5f, 12.5f, -12.5f / ratio, 12.5f / ratio, -2.0f, 10.0f);
	}

	loadIdentity(MODEL);
	loadIdentity(VIEW);
	lookAt(0.0f, 10.0f, 0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 0.0f);
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

void updateCamera() {
	//setup camera
	camX = r * sin(alpha * PI / 180.0f) * cos(beta * PI / 180.0f);
	camZ = r * cos(alpha * PI / 180.0f) * cos(beta * PI / 180.0f);
	camY = r * sin(beta * PI / 180.0f);
}

void resetCamera() {
	alpha = 0.0f;
	beta = 30.0f;
	r = 15.0f;
	updateCamera();
}

void collisionWithOranges() {
	for (int i = 0; i < NUMBER_ORANGES; i++) {
		if (hasCollided(carPos, carDim, orangePos[i], orangeDim)) {
			if (lives > 1) {
				lives--;
				currentCheckPoint = 0;
				resetCamera();
				resetCar();
				resetOranges();
			}
			else {
				lives--;
				isGameOver = true;
			}
		}
	}
}

void resetGame() {
	isGameOver = false;
	isPaused = false;
	lives = 5;
	score = 0;
	currentCheckPoint = 0;
	resetCamera();
	resetCar();
	resetOranges();
	resetCheerios();
}


void animateCar() {
	float aux[3];

	if (carIsForward) {
		if (carSpeed >= 0.0f)
			carSpeed += carAcc;
		else
			carSpeed += carBrake;

	}

	else if (carIsReverse) {
		if (carSpeed <= 0.0f)
			carSpeed -= carReverse;
		else
			carSpeed -= carBrake;
	}

	else if (!carIsReverse && !carIsForward) {
		if (carSpeed <= carTraction && carSpeed >= -carTraction)
			carSpeed = 0.0f;
		else if (carSpeed > 0.0f)
			carSpeed -= carTraction;
		else
			carSpeed += carTraction;
	}

	if (carSpeed > speedLimit)
		carSpeed = speedLimit;
	else if (carSpeed < -speedLimit)
		carSpeed = -speedLimit;

	aux[0] = carDir[0] * carSpeed;
	aux[1] = carDir[1] * carSpeed;
	aux[2] = carDir[2] * carSpeed;
	add(carPos, aux, carPos);
	add(spotLightPos, aux, spotLightPos);

	if (carSpeed != 0.0f) {
		if ((carIsLeft && carSpeed > 0.0f) || (carIsRight && carSpeed < 0.0f)) {
			rotate(carDir, carAngularSpeed, axisY);
			rotate(spotLightDir_1, carAngularSpeed, axisY);
			rotate(spotLightDir_2, carAngularSpeed, axisY);
			carAngle += carAngularSpeed;
			alpha += carAngularSpeed;
			carAngle = fmod(carAngle, 360.0f);
			alpha = fmod(alpha, 360.0f);
			if (tracking != 1)
				updateCamera();
		}

		else if (carIsRight && carSpeed > 0.0f || (carIsLeft && carSpeed < 0.0f)) {
			rotate(carDir, -carAngularSpeed, axisY);
			rotate(spotLightDir_1, -carAngularSpeed, axisY);
			rotate(spotLightDir_2, -carAngularSpeed, axisY);
			carAngle -= carAngularSpeed;
			alpha -= carAngularSpeed;
			carAngle = fmod(carAngle, 360.0f);
			alpha = fmod(alpha, 360.0f);
			if (tracking != 1)
				updateCamera();
		}
	}

}

void animateCheerios() {
	float aux[3];
	for (int i = 0; i < NUMBER_CHEERIOS; i++) {

		if (hasCollided(carPos, carDim, cheerioPos[i], cheerioDim)) {
			drawParticles = true;
			initParticles();

			if (carSpeed >= 0.0f) {
				cheerioSpeed[i] = carSpeed;
				cheerioDir[i][0] = carDir[0];
				cheerioDir[i][1] = carDir[1];
				cheerioDir[i][2] = carDir[2];
			}

			else {
				cheerioSpeed[i] = -carSpeed;
				cheerioDir[i][0] = -carDir[0];
				cheerioDir[i][1] = -carDir[1];
				cheerioDir[i][2] = -carDir[2];
			}

			carSpeed = 0;
		}

		if (cheerioSpeed[i] > cheerioTraction) {
			cheerioSpeed[i] -= cheerioTraction;
			aux[0] = cheerioDir[i][0] * cheerioSpeed[i];
			aux[1] = cheerioDir[i][1] * cheerioSpeed[i];
			aux[2] = cheerioDir[i][2] * cheerioSpeed[i];
			add(cheerioPos[i], aux, cheerioPos[i]);
		}

		else
			cheerioSpeed[i] = 0.0f;
	}
}

void animateOranges() {

	for (int i = 0; i < NUMBER_ORANGES; i++) {


		if (orangePos[i][0] >= 100.0f) { // respawn
			orangePos[i][0] = -50.0f;
			orangePos[i][1] = 1.0f;
		}
		else if (orangePos[i][0] >= 50.0f) { // hide orange

			if(orangePos[i][1] != -500.0f) // just to avoid too many writes
				orangePos[i][1] = -500.0f;

			orangePos[i][0] += orangeSpeed[i];
		}
		else { // normal movement
			orangeSpeed[i] += orangeAcceleration[i];
			orangePos[i][0] += orangeSpeed[i];
		}

	}

}

void animateBillboard() {
	if (tracking != 1)
		billboardAngle = alpha;
}

void sendLights() {

	GLint loc;
	float res[4];
	float aux[4];

	loc = glGetUniformLocation(pid, "Lights[0].isEnabled");
	glUniform1i(loc, dirLightOn);
	loc = glGetUniformLocation(pid, "Lights[0].isPointLight");
	glUniform1i(loc, false);
	loc = glGetUniformLocation(pid, "Lights[0].position");
	glUniform4fv(loc, 1, dirLight);
	loc = glGetUniformLocation(pid, "Lights[0].color");
	glUniform4fv(loc, 1, dirLightColor);

	/* LEFT SPOTLIGHT */

	loc = glGetUniformLocation(pid, "Lights[1].isEnabled");
	glUniform1i(loc, headlightsOn);
	loc = glGetUniformLocation(pid, "Lights[1].isPointLight");
	glUniform1i(loc, true);
	loc = glGetUniformLocation(pid, "Lights[1].isSpotLight");
	glUniform1i(loc, true);

	aux[0] = spotLightPos[0] + spotLightDir_1[0];
	aux[1] = spotLightPos[1] + spotLightDir_1[1];
	aux[2] = spotLightPos[2] + spotLightDir_1[2];
	aux[3] = spotLightPos[3];
	multMatrixPoint(VIEW, aux, res);
	loc = glGetUniformLocation(pid, "Lights[1].position");
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(pid, "Lights[1].color");
	glUniform4fv(loc, 1, spotLightColor);

	multMatrixPoint(VIEW, carDir, res);
	loc = glGetUniformLocation(pid, "Lights[1].coneDirection");
	glUniform3fv(loc, 1, res);

	loc = glGetUniformLocation(pid, "Lights[1].spotCosCutoff");
	glUniform1f(loc, spotCosCutoff);
	loc = glGetUniformLocation(pid, "Lights[1].spotExponent");
	glUniform1f(loc, spotExponent);
	loc = glGetUniformLocation(pid, "Lights[1].linearAttenuation");
	glUniform1f(loc, linearAttenuation);

	/* RIGHT SPOTLIGHT */

	loc = glGetUniformLocation(pid, "Lights[2].isEnabled");
	glUniform1i(loc, headlightsOn);
	loc = glGetUniformLocation(pid, "Lights[2].isPointLight");
	glUniform1i(loc, true);
	loc = glGetUniformLocation(pid, "Lights[2].isSpotLight");
	glUniform1i(loc, true);

	aux[0] = spotLightPos[0] + spotLightDir_2[0];
	aux[1] = spotLightPos[1] + spotLightDir_2[1];
	aux[2] = spotLightPos[2] + spotLightDir_2[2];
	aux[3] = spotLightPos[3];
	multMatrixPoint(VIEW, aux, res);
	loc = glGetUniformLocation(pid, "Lights[2].position");
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(pid, "Lights[2].color");
	glUniform4fv(loc, 1, spotLightColor);

	multMatrixPoint(VIEW, carDir, res);
	loc = glGetUniformLocation(pid, "Lights[2].coneDirection");
	glUniform3fv(loc, 1, res);

	loc = glGetUniformLocation(pid, "Lights[2].spotCosCutoff");
	glUniform1f(loc, spotCosCutoff);
	loc = glGetUniformLocation(pid, "Lights[2].spotExponent");
	glUniform1f(loc, spotExponent);
	loc = glGetUniformLocation(pid, "Lights[2].linearAttenuation");
	glUniform1f(loc, linearAttenuation);

	/* CANDLE LIGHTS */
	glUniform1i(glGetUniformLocation(pid, "Lights[3].isEnabled"), candlesOn);
	glUniform1i(glGetUniformLocation(pid, "Lights[4].isEnabled"), candlesOn);
	glUniform1i(glGetUniformLocation(pid, "Lights[5].isEnabled"), candlesOn);
	glUniform1i(glGetUniformLocation(pid, "Lights[6].isEnabled"), candlesOn);
	glUniform1i(glGetUniformLocation(pid, "Lights[7].isEnabled"), candlesOn);
	glUniform1i(glGetUniformLocation(pid, "Lights[8].isEnabled"), candlesOn);

	glUniform1i(glGetUniformLocation(pid, "Lights[3].isPointLight"), true);
	glUniform1i(glGetUniformLocation(pid, "Lights[4].isPointLight"), true);
	glUniform1i(glGetUniformLocation(pid, "Lights[5].isPointLight"), true);
	glUniform1i(glGetUniformLocation(pid, "Lights[6].isPointLight"), true);
	glUniform1i(glGetUniformLocation(pid, "Lights[7].isPointLight"), true);
	glUniform1i(glGetUniformLocation(pid, "Lights[8].isPointLight"), true);

	glUniform1i(glGetUniformLocation(pid, "Lights[3].isSpotLight"), false);
	glUniform1i(glGetUniformLocation(pid, "Lights[4].isSpotLight"), false);
	glUniform1i(glGetUniformLocation(pid, "Lights[5].isSpotLight"), false);
	glUniform1i(glGetUniformLocation(pid, "Lights[6].isSpotLight"), false);
	glUniform1i(glGetUniformLocation(pid, "Lights[7].isSpotLight"), false);
	glUniform1i(glGetUniformLocation(pid, "Lights[8].isSpotLight"), false);

	multMatrixPoint(VIEW, candlePos[0], res);
	glUniform4fv(glGetUniformLocation(pid, "Lights[3].position"), 1, res);
	multMatrixPoint(VIEW, candlePos[1], res);
	glUniform4fv(glGetUniformLocation(pid, "Lights[4].position"), 1, res);
	multMatrixPoint(VIEW, candlePos[2], res);
	glUniform4fv(glGetUniformLocation(pid, "Lights[5].position"), 1, res);
	multMatrixPoint(VIEW, candlePos[3], res);
	glUniform4fv(glGetUniformLocation(pid, "Lights[6].position"), 1, res);
	multMatrixPoint(VIEW, candlePos[4], res);
	glUniform4fv(glGetUniformLocation(pid, "Lights[7].position"), 1, res);
	multMatrixPoint(VIEW, candlePos[5], res);
	glUniform4fv(glGetUniformLocation(pid, "Lights[8].position"), 1, res);

	glUniform1f(glGetUniformLocation(pid, "Lights[3].linearAttenuation"), candleAttenuation);
	glUniform1f(glGetUniformLocation(pid, "Lights[4].linearAttenuation"), candleAttenuation);
	glUniform1f(glGetUniformLocation(pid, "Lights[5].linearAttenuation"), candleAttenuation);
	glUniform1f(glGetUniformLocation(pid, "Lights[6].linearAttenuation"), candleAttenuation);
	glUniform1f(glGetUniformLocation(pid, "Lights[7].linearAttenuation"), candleAttenuation);
	glUniform1f(glGetUniformLocation(pid, "Lights[8].linearAttenuation"), candleAttenuation);

	glUniform4fv(glGetUniformLocation(pid, "Lights[3].color"), 1, candleColor);
	glUniform4fv(glGetUniformLocation(pid, "Lights[4].color"), 1, candleColor);
	glUniform4fv(glGetUniformLocation(pid, "Lights[5].color"), 1, candleColor);
	glUniform4fv(glGetUniformLocation(pid, "Lights[6].color"), 1, candleColor);
	glUniform4fv(glGetUniformLocation(pid, "Lights[7].color"), 1, candleColor);
	glUniform4fv(glGetUniformLocation(pid, "Lights[8].color"), 1, candleColor);
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

void renderMesh() {

	// send materials
	glUniform4fv(glGetUniformLocation(pid, "mat.ambient"), 1, objMesh[meshID].mat.ambient);
	glUniform4fv(glGetUniformLocation(pid, "mat.diffuse"), 1, objMesh[meshID].mat.diffuse);
	glUniform4fv(glGetUniformLocation(pid, "mat.specular"), 1, objMesh[meshID].mat.specular);
	glUniform1f(glGetUniformLocation(pid, "mat.shininess"), objMesh[meshID].mat.shininess);

	//send matrices
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	//draw
	glBindVertexArray(objMesh[meshID].getVAO());
	glDrawArrays(GL_TRIANGLES, 0, objMesh[meshID].getVertexCount());
	glBindVertexArray(0);

	loadIdentity(MODEL); //reset model matrix
}

void selectTextureForFlare(int i) {
	switch (i) {
	case 0:
		glUniform1i(tex_2_loc, 1);
		break;
	case 1:
		glUniform1i(tex_2_loc, 2);
		break;
	case 2:
		glUniform1i(tex_2_loc, 3);
		break;
	case 3:
		glUniform1i(tex_2_loc, 2);
		break;
	case 4:
		glUniform1i(tex_2_loc, 1);
		break;
	}
}

void renderLensFlare() {
	float cx, cy; //center of window
	float lx, ly; //relative pos of "source light"
	GLint loc;

	cx = (float)WinX / 2.0f;
	cy = (float)WinY / 2.0f;

	lx = (float)WinX * lensSourcePos[0];
	ly = (float)WinY * lensSourcePos[1];

	float maxdist, dist, ratio;

	maxdist = sqrt((cx*cx) + (cy*cy));
	dist = sqrt((lx - cx)*(lx - cx) + (ly - cy)*(ly - cy));
	ratio = (maxdist - dist) / maxdist;

	float scalefactor = 0.3f + (ratio * 0.8f);
	float alphafactor = 0.2f + (ratio * 0.15f);

	loc = glGetUniformLocation(pid, "lensAlpha");
	glUniform1f(loc, alphafactor);

	// dx and dy are the opposite ends
	float dx, dy;

	dx = cx + (cx - lx);
	dy = cy + (cy - ly);

	pushMatrix(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(PROJECTION);
	loadIdentity(VIEW);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	ortho(0, (float)WinX, 0, (float)WinY, -1, 1);


	// position of element
	float px, py;
	for (int i = 0; i < FLARE_ELEMENTS; i++) {

		px = (1.0f - lensPos[i])*lx + lensPos[i] * dx;
		py = (1.0f - lensPos[i])*ly + lensPos[i] * dy;

		selectTextureForFlare(i);
		
		
		objID = 5;
		translate(MODEL, px, py, 0);
		scale(MODEL, 50 * lensScale[i] * scalefactor, 50 * lensScale[i] * scalefactor, 0);
		glUniform1i(texMode_UID, 2);
		render();
		glUniform1i(texMode_UID, 0);


	}

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

}

void renderBillboard() {
	/** / // Billboard without cheating
	float pos[3] = { 0.0f, 25.0f, 0.0f };
	float cam[3] = { camX, camY, camZ };
	l3dBillboardCylindricalBegin(cam, pos);
	/**/
	computeDerivedMatrix(VIEW_MODEL);
	BillboardCheatCylindricalBegin();
	computeDerivedMatrix_PVM();
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
	glBindVertexArray(mesh[objID].vao);
	glDrawElements(mesh[objID].type, mesh[objID].numIndexes, GL_UNSIGNED_INT, 0);
	loadIdentity(MODEL);
	/**/
}

void displayLives(int w, int h) {
	float x, z, ratio;
	ratio = (float)w / h;

	setOrthogonalProjectionForHUD(w, h);
	if (ratio > 1.0f) {
		x = 10.5f * ratio;
		z = 10.5f;
	}

	else {
		x = 10.5f;
		z = 10.5f / ratio;
	}


	for (int i = 0; i < lives; i++) {
		livesPos[i][0] = z;
		livesPos[i][1] = 1.0f;
		livesPos[i][2] = x - 1.5f*i;
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
		glStencilMask(0xFF);
		meshID = 3;
		translate(MODEL, livesPos[i][0], livesPos[i][1], livesPos[i][2]);
		scale(MODEL, 0.01f, 0.01f, 0.01f);
		rotate(MODEL, 180.0f, 0,1,0);
		renderMesh();
	}

	popMatrix(PROJECTION);
	popMatrix(MODEL);
	popMatrix(VIEW);

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
		lookAt(65, 30, 0, 0, 0, 0, 0, 1, 0);
	}

	else if (camera == CHASE) {
		lookAt(camX + carPos[0], camY + carPos[1] + 2.0f, camZ + carPos[2], carPos[0], carPos[1], carPos[2], 0, 1, 0);
	}

	// use our shader
	glUseProgram(shader.getProgramIndex());

	sendLights();

	// TEXTURES

	// Grid Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);
	// Flare Hex
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);
	// Flare Ring
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);
	// Flare Source (?)
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);
	// Sauron Eye
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);


	pushMatrix(MODEL);

	if (!isPaused && !isGameOver) {
		animateCar();
		animateOranges();
		animateCheerios();
		animateBillboard();
		lamp_spin += 0.2f;
		lamp_spin = fmod(lamp_spin, 360.0f);
	}

	updateScore();
	displayLives(WinX, WinY);


	//--------[ Remember: the first transform is the last one coded! ]--------\\

	// Table
	objID = 0;
	translate(MODEL, 0.0f, -50.0f, 0.0f);
	scale(MODEL, 100.0f, 100.0f, 100.0f);
	translate(MODEL, -0.5f, -0.5f, -0.5f);
	glUniform1i(tex_1_loc, 0);
	glUniform1i(texMode_UID, 1);
	render();
	glUniform1i(texMode_UID, 0);

	// Car
	meshID = 2;
	translate(MODEL, carPos[0], carPos[1] + 0.5f, carPos[2]);
	rotate(MODEL, carAngle - 90.0f, 0.0f, 1.0f, 0.0f);
	scale(MODEL, 0.02f, 0.02f, 0.02f);
	renderMesh();

	// Cheerios
	meshID = 4;
	for (int i = 0; i < NUMBER_CHEERIOS; i++) {
		translate(MODEL, cheerioPos[i][0], cheerioPos[i][1], cheerioPos[i][2]);
		renderMesh();
	}

	collisionWithOranges();

	// Oranges
	meshID = 5;
	for (int i = 0; i < NUMBER_ORANGES; i++) {
		translate(MODEL, orangePos[i][0], orangePos[i][1], orangePos[i][2]);
		renderMesh();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glUniform1i(tex_1_loc, 4);
	glUniform1i(texMode_UID, 2);
	// Particles
	if (drawParticles) {
		objID = 5;
		for (int i = 0; i < MAX_PARTICLES; i++) {
			if (particleLife[i] > 0.0f) {
				translate(MODEL, particlePos[i][0], particlePos[i][1] +3, particlePos[i][2]);
				renderBillboard();
			}

			else deadParticles++;
		}

		if (deadParticles == MAX_PARTICLES) {
			drawParticles = false;
			deadParticles = 0;
		}
	}
	glUniform1i(texMode_UID, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//Old Billboard
	/** /
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);
	glUniform1i(tex_1_loc, 0);

	objID = 6;
	translate(MODEL, 0.0f, 25.0f, 0.0f);
	scale(MODEL, 7.5f, 7.5f, 7.5f);
	rotate(MODEL, billboardAngle, 0.0f, 1.0f, 0.0f);

	glUniform1i(texMode_UID, 3);
	render();
	glUniform1i(texMode_UID, 0);
	/**/

	glUniform1i(tex_1_loc, 4);

	objID = 6;
	translate(MODEL, 0.0f, 35.0f, 0.0f);
	glUniform1i(texMode_UID, 3);
	renderBillboard();
	glUniform1i(texMode_UID, 0);

	loadIdentity(MODEL);

	// OBJ Rendering

	// Level Scenario
	meshID = 0;
	translate(MODEL, 0, -0.5f, 0);
	scale(MODEL, 0.7f, 1, 0.7f);
	renderMesh();

	// Lamps
	meshID = 1;
	for (int i = 0; i < 6; i++) {
		translate(MODEL, candlePos[i][0], candlePos[i][1], candlePos[i][2]);
		rotate(MODEL, lamp_spin, 0, 1, 0);
		scale(MODEL, 0.2f, 0.2f, 0.2f);
		renderMesh();
	}
	/**/

	renderLensFlare();

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
	switch (key) {

		case 27:
			glutLeaveMainLoop();
			break;

		//cameras
		case '1':
			camera = ORTHOGRAPHIC;
			loadIdentity(PROJECTION);
			ortho(-50, 50, -50, 50, 0.1f, 1000.f);
			lensSourcePos[0] = lensDefaultSourcePos[0];
			lensSourcePos[1] = lensDefaultSourcePos[1];
			break;
		case '2':
			camera = TOP;
			loadIdentity(PROJECTION);
			perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f);
			lensSourcePos[0] = lensDefaultSourcePos[0];
			lensSourcePos[1] = lensDefaultSourcePos[1];
			break;
		case '3': camera = CHASE; loadIdentity(PROJECTION); perspective(53.13f, (1.0f*WinX) / WinY, 0.1f, 1000.0f); break;


		//aliasing settings
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;
		case 'r': resetGame();  break;

		//game controls
		case 'p': isPaused = !isPaused; break;

		case 'w': carIsForward = true;  break;
		case 's': carIsReverse = true;  break;
		case 'd': carIsRight = true;  break;
		case 'a': carIsLeft = true;  break;

		//lights
		case 'l': dirLightOn = !dirLightOn; break;
		case 'h': headlightsOn = !headlightsOn; break;
		case 'c': candlesOn = !candlesOn; break;
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

		billboardAngle = alphaAux;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;

		//lens flare source movement

		if (camera == CHASE) {
			float stepX = (float)deltaX * 0.0001f;
			float stepY = (float)deltaY * 0.0001f;

			lensSourcePos[0] += stepX;
			lensSourcePos[1] += stepY;

			if (lensSourcePos[0] > 1.0f)
				lensSourcePos[0] = 1.0f;
			else if (lensSourcePos[0] < 0.0f)
				lensSourcePos[0] = 0.0f;

			if (lensSourcePos[1] > 1.0f)
				lensSourcePos[1] = 1.0f;
			else if (lensSourcePos[1] < 0.0f)
				lensSourcePos[1] = 0.0f;
		}

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

	r -= direction * 1.0f;
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
	tex_2_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	
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

void loadObjMaterials(float* ambient, float* diffuse, float* specular, float* emissive, float shininess, int texCount) {
	memcpy(objMesh[meshID].mat.ambient, ambient, 4 * sizeof(float));
	memcpy(objMesh[meshID].mat.diffuse, diffuse, 4 * sizeof(float));
	memcpy(objMesh[meshID].mat.specular, specular, 4 * sizeof(float));
	memcpy(objMesh[meshID].mat.emissive, emissive, 4 * sizeof(float));
	objMesh[meshID].mat.shininess = shininess;
	objMesh[meshID].mat.texCount = texCount;
}

void init()
{
	updateCamera();

	glGenTextures(4, TextureArray);
	TGA_Texture(TextureArray, "textures/futuristic_grid.tga", 0);
	TGA_Texture(TextureArray, "textures/hexagon.tga", 1);
	TGA_Texture(TextureArray, "textures/ring.tga", 2);
	TGA_Texture(TextureArray, "textures/sourcelight.tga", 3);
	TGA_Texture(TextureArray, "textures/billboard.tga", 4);

	//Model init

	//Cheerio position init
	resetCheerios();

	//Orange position init
	resetOranges();

	// Table
	objID = 0;

	float ambTable[4] = { 0.01f, 0.01f, 0.01f, 1.0f };
	float diffTable[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
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

	float ambCheerio[4] = { 0.01f, 0.02f, 0.0f, 1.0f };
	float diffCheerio[4] = { 0.55f, 0.3f, 0.0f, 1.0f };
	float specCheerio[4] = { 0.55f, 0.3f, 0.0f, 1.0f };

	objID = 2;
	loadMaterials(ambCheerio, diffCheerio, specCheerio, null, shininess, texCount);
	createTorus(cheerioDim[1] / 2.0f, cheerioDim[1], 10, 10);

	// Oranges
	objID = 3;

	float ambOrange[4] = { 0.1f, 0.07f, 0.0f, 1.0f };
	float diffOrange[4] = {1.0f, 0.7f, 0.0f, 1.0f};
	float specOrange[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	loadMaterials(ambOrange, diffOrange, specOrange, null, shininess, texCount);
	createSphere(orangeDim[1] / 2.0f, 30);

	// Particle
	objID = 4;
	float ambParticle[4] = { 0.1f, 0.07f, 0.0f, 1.0f };
	float diffParticle[4] = { 1.0f, 0.7f, 0.0f, 1.0f };
	float specParticle[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	loadMaterials(ambParticle, diffParticle, specParticle, null, shininess, texCount);
	createCube();

	// Lens Flare quad
	objID = 5;
	float ambFlare[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffFlare[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float specFlare[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	loadMaterials(ambFlare, diffFlare, specFlare, null, shininess, texCount);
	createQuad(2, 2);

	// BillBoard
	objID = 6;

	loadMaterials(ambTable, diffTable, specTable, null, shininess, texCount);
	createQuad(7.5f, 7.5f);

	/*----------------- OBJ LOADING -------------------*/

	// Leven Scenario
	meshID = 0;
	Mesh level = Mesh("models/level.obj");
	objMesh.push_back(level);

	float ambLevel[4] = { 0.01f, 0.01f, 0.01f, 0.5f };
	float diffLevel[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	float specLevel[4] = { 1.0f, 1.0f, 1.0f, 0.5f };

	loadObjMaterials(ambLevel, diffLevel, specLevel, null, shininess, texCount);

	// Rotating Lamp
	meshID = 1;
	Mesh lamp = Mesh("models/cube_lamp.obj");
	objMesh.push_back(lamp);

	float ambLamp[4] = { 0.0f, 0.0f, 0.01f, 0.2f };
	float diffLamp[4] = { 0.0f, 0.0f, 0.5f, 0.2f };
	float specLamp[4] = { 1.0f, 1.0f, 1.0f, 0.2f };

	loadObjMaterials(ambLamp, diffLamp, specLamp, null, shininess, texCount);

	// Car
	meshID = 2;
	Mesh car = Mesh("models/car.obj");
	objMesh.push_back(car);

	loadObjMaterials(ambTestCube, diffTestCube, specTestCube, null, shininess, texCount);

	// Life
	meshID = 3;
	Mesh life = Mesh("models/car.obj");
	objMesh.push_back(life);

	float ambLife[4] = { 0.7f, 0.0f, 0.0f, 1.0f };
	float diffLife[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float specLife[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	loadObjMaterials(ambLife, diffLife, specLife, null, shininess, texCount);

	// Obstacle aka cheerio
	meshID = 4;
	Mesh obstacle = Mesh("models/obstacle.obj");
	objMesh.push_back(obstacle);

	loadObjMaterials(ambCheerio, diffCheerio, specCheerio, null, shininess, texCount);

	//Spiked ball aka orange
	meshID = 5;
	Mesh ball = Mesh("models/spiked_ball.obj");
	objMesh.push_back(ball);

	loadObjMaterials(ambOrange, diffOrange, specOrange, null, shininess, texCount);
	
	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

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
	glutTimerFunc(0, iterate, 0);


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

