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

// assimp include files. These three are usually needed.
#pragma comment(lib, "assimp.lib") 
#include "Importer.hpp"	//OO version Header!
#include "PostProcess.h"
#include "Scene.h"

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
// orange = 3
const int objCount = 4;

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
float alpha = 0.0f; // horizontal angle
float beta = 30.0f; // vertical angle
float r = 15.0f;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

//------------------[ CAR ]------------------//

float carPos[3] = { 30.0f, 0.0f, 0.0f };
float carDim[2] = { 1.0f, 1.0f };
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

//------------------[ LIGHTS ]------------------//

float dirLight[4] = { 0.5f, 1.0f, 0.0f, 0.0f };
float dirLightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

//------------------[ TEXTURES ]------------------//

GLint texMode_UID;
GLint tex_1_loc;

GLuint TextureArray[1];

//------------------[ AUXILIARY FUNCS ]------------------//

//returns float between 0.0f and 1.0f
float RNG() {
	float RNG = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return RNG;
}

//--------------------------------------[ ASSIMP ]--------------------------------------//

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

const aiScene* scene_1 = NULL;
const aiScene* scene_2 = NULL;
float scene_1_sf;
float scene_2_sf;

struct Mesh {

	GLuint vao;
	GLuint texIndex;
	struct Material mat;
	int numFaces;
};

Assimp::Importer importer;

float scaleFactor; // scale factor for the scene;

std::vector<struct Mesh> meshes; // for storing imported .OBJ files

void set_float4(float f[4], float a, float b, float c, float d){
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}
void color4_to_float4(const aiColor4D *c, float f[4]){
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}
void get_bounding_box_for_node(const aiNode* nd, aiVector3D* min, aiVector3D* max, const aiScene* scene) {
	aiMatrix4x4 prev;
	unsigned int n = 0, t;

	for (; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];

			min->x = aisgl_min(min->x, tmp.x);
			min->y = aisgl_min(min->y, tmp.y);
			min->z = aisgl_min(min->z, tmp.z);

			max->x = aisgl_max(max->x, tmp.x);
			max->y = aisgl_max(max->y, tmp.y);
			max->z = aisgl_max(max->z, tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n], min, max, scene);
	}
}
void get_bounding_box(aiVector3D* min, aiVector3D* max, const aiScene* scene) {

	min->x = min->y = min->z = 1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode, min, max, scene);
}
bool Import3DFromFile(const std::string& pFile, const aiScene** scene)
{

	//check if file exists
	std::ifstream fin(pFile.c_str());
	if (!fin.fail()) {
		fin.close();
	}
	else {
		printf("Couldn't open file: %s\n", pFile.c_str());
		printf("%s\n", importer.GetErrorString());
		return false;
	}

	*scene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Quality);

	// If the import failed, report it
	if (!scene)
	{
		printf("%s\n", importer.GetErrorString());
		return false;
	}

	// Now we can access the file's contents.
	printf("Import of scene %s succeeded.", pFile.c_str());

	aiVector3D scene_min, scene_max, scene_center;
	get_bounding_box(&scene_min, &scene_max, *scene);
	float tmp;
	tmp = scene_max.x - scene_min.x;
	tmp = scene_max.y - scene_min.y > tmp ? scene_max.y - scene_min.y : tmp;
	tmp = scene_max.z - scene_min.z > tmp ? scene_max.z - scene_min.z : tmp;
	scaleFactor = 1.f / tmp;

	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}
void genVAOsAndUniformBuffer(const aiScene *sc) {

	struct Mesh aMesh;
	struct Material aMat;
	GLuint buffer;

	// For each mesh
	for (unsigned int n = 0; n < sc->mNumMeshes; ++n)
	{
		const aiMesh* mesh = sc->mMeshes[n];

		// create array with faces
		// have to convert from Assimp format to array
		unsigned int *faceArray;
		faceArray = (unsigned int *)malloc(sizeof(unsigned int) * mesh->mNumFaces * 3);
		unsigned int faceIndex = 0;

		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace* face = &mesh->mFaces[t];

			memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(unsigned int));
			faceIndex += 3;
		}
		aMesh.numFaces = sc->mMeshes[n]->mNumFaces;

		// generate Vertex Array for mesh
		glGenVertexArrays(1, &(aMesh.vao));
		glBindVertexArray(aMesh.vao);

		// buffer for faces
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);

		// buffer for vertex positions
		if (mesh->HasPositions()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(VERTEX_COORD_ATTRIB);
			glVertexAttribPointer(VERTEX_COORD_ATTRIB, 3, GL_FLOAT, 0, 0, 0);
		}

		// buffer for vertex normals
		if (mesh->HasNormals()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
			glEnableVertexAttribArray(NORMAL_ATTRIB);
			glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, 0, 0, 0);
		}

		// buffer for vertex texture coordinates
		if (mesh->HasTextureCoords(0)) {
			float *texCoords = (float *)malloc(sizeof(float) * 2 * mesh->mNumVertices);
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {

				texCoords[k * 2] = mesh->mTextureCoords[0][k].x;
				texCoords[k * 2 + 1] = mesh->mTextureCoords[0][k].y;

			}
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
			glEnableVertexAttribArray(TEXTURE_COORD_ATTRIB);
			glVertexAttribPointer(TEXTURE_COORD_ATTRIB, 2, GL_FLOAT, 0, 0, 0);
		}

		// unbind buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// create material uniform buffer
		aiMaterial *mtl = sc->mMaterials[mesh->mMaterialIndex];
		aMat.texCount = 0;

		float c[4];
		set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
		aiColor4D diffuse;
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
			color4_to_float4(&diffuse, c);
		memcpy(aMat.diffuse, c, sizeof(c));

		set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
		aiColor4D ambient;
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
			color4_to_float4(&ambient, c);
		memcpy(aMat.ambient, c, sizeof(c));

		set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
		aiColor4D specular;
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
			color4_to_float4(&specular, c);
		memcpy(aMat.specular, c, sizeof(c));

		set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
		aiColor4D emission;
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
			color4_to_float4(&emission, c);
		memcpy(aMat.emissive, c, sizeof(c));

		float shininess = 0.0;
		unsigned int max;
		aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
		aMat.shininess = shininess;

		memcpy(aMesh.mat.diffuse, aMat.diffuse, sizeof(aMat.diffuse));
		memcpy(aMesh.mat.ambient, aMat.ambient, sizeof(aMat.ambient));
		memcpy(aMesh.mat.specular, aMat.specular, sizeof(aMat.specular));
		memcpy(aMesh.mat.emissive, aMat.emissive, sizeof(aMat.emissive));
		aMesh.mat.shininess = aMat.shininess;

		meshes.push_back(aMesh);
	}
}

// Render Assimp Model

void renderModel(int model_ID, const aiScene *sc, const aiNode* nd)
{

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(mMatrix[MODEL], aux);

	//send matrices
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);


	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {

		GLint loc;

		// send materials
		loc = glGetUniformLocation(pid, "mat.ambient");
		glUniform4fv(loc, 1, meshes[model_ID].mat.ambient);
		loc = glGetUniformLocation(pid, "mat.diffuse");
		glUniform4fv(loc, 1, meshes[model_ID].mat.diffuse);
		loc = glGetUniformLocation(pid, "mat.specular");
		glUniform4fv(loc, 1, meshes[model_ID].mat.specular);
		loc = glGetUniformLocation(pid, "mat.shininess");
		glUniform1f(loc, meshes[model_ID].mat.shininess);

		// bind texture
		//glBindTexture(GL_TEXTURE_2D, meshes[nd->mMeshes[n]].texIndex);
		// bind VAO
		glBindVertexArray(meshes[nd->mMeshes[n]].vao);
		// draw
		glDrawElements(GL_TRIANGLES, meshes[nd->mMeshes[n]].numFaces * 3, GL_UNSIGNED_INT, 0);

	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		renderModel(0, sc, nd->mChildren[n]);
	}
	popMatrix(MODEL);
}

//--------------------------------------------------------------------------------------//
//------------------------------------[ ASSIMP END ]------------------------------------//
//--------------------------------------------------------------------------------------//

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
		}

		break;
	}
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

	if (carSpeed != 0.0f) {
		if (carIsLeft) {
			rotate(carDir, carAngularSpeed, axisY);
			carAngle += carAngularSpeed;
			alpha += carAngularSpeed;
			updateCamera();
		}

		else if (carIsRight) {
			rotate(carDir, -carAngularSpeed, axisY);
			carAngle -= carAngularSpeed;
			alpha -= carAngularSpeed;
			updateCamera();
		}
	}

}

void animateCheerios() {
	float aux[3];
	for (int i = 0; i < NUMBER_CHEERIOS; i++) {

		if (hasCollided(carPos, carDim, cheerioPos[i], cheerioDim)) {

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
		lookAt(65, 30, 0, 0, 0, 0, 0, 1, 0);
	}

	else if (camera == CHASE) {
		lookAt(camX + carPos[0], camY + carPos[1] + 2.0f, camZ + carPos[2], carPos[0], carPos[1], carPos[2], 0, 1, 0);
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
	animateCheerios();
	updateScore();

	//--------[ Remember: the first transform is the last one coded! ]--------\\

	// Table
	objID = 0;
	translate(MODEL, 0.0f, -50.0f, 0.0f);
	scale(MODEL, 100.0f, 100.0f, 100.0f);
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

	animateOranges();

	// Oranges
	objID = 3;
	for (int i = 0; i < NUMBER_ORANGES; i++) {
		translate(MODEL, orangePos[i][0], orangePos[i][1], orangePos[i][2]);
		render();
	}

	pushMatrix(MODEL);

	translate(MODEL, 0.0f, -0.003f, 0.0f);
	scale(MODEL, scene_1_sf*130.0f, scene_1_sf*130.0f, scene_1_sf*130.0f);
	renderModel(0, scene_1, scene_1->mRootNode);

	popMatrix(MODEL);


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
	updateCamera();

	glGenTextures(1, TextureArray);
	TGA_Texture(TextureArray, "textures/futuristic_grid.tga", 0);

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

	//Orange position init

	for (int i = 0; i < NUMBER_ORANGES; i++) {
		orangePos[i][0] = -50.0f;
		orangePos[i][1] = 1.0f;
		orangePos[i][2] = -30.0f + (15.0f * (float) i);
		orangePos[i][3] = 1.0f;

		orangeSpeed[i] = RNG() * 0.6f;
		orangeAcceleration[i] = RNG() * 0.0001f;
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

	/*----------------- ASSIMP -------------------*/

	/**/
	if (!Import3DFromFile("models/level.obj", &scene_1))
		std::cout << "ERROR LOADING LEVEL OBJ" << std::endl;

	scene_1_sf = scaleFactor;
	genVAOsAndUniformBuffer(scene_1);

	float ambLevel[4] = { 0.01f, 0.01f, 0.01f, 0.5f };
	float diffLevel[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	float specLevel[4] = { 1.0f, 1.0f, 1.0f, 0.5f };

	memcpy(meshes[0].mat.ambient, ambLevel, sizeof(ambLevel));
	memcpy(meshes[0].mat.diffuse, diffLevel, sizeof(diffLevel));
	memcpy(meshes[0].mat.specular, specLevel, sizeof(specLevel));

	/** /
	if (!Import3DFromFile("models/cube_lamp.obj", &scene_2))
		std::cout << "ERROR LOADING LAMP OBJ" << std::endl;

	scene_2_sf = scaleFactor;
	genVAOsAndUniformBuffer(scene_2);

	float ambLamp[4] = { 0.0f, 0.0f, 0.1f, 1.0f };
	float diffLamp[4] = { 0.0f, 0.2f, 0.7f, 1.0f };
	float specLamp[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	memcpy(meshes[0].mat.ambient, ambLamp, sizeof(ambLamp));
	memcpy(meshes[0].mat.diffuse, diffLamp, sizeof(diffLamp));
	memcpy(meshes[0].mat.specular, specLamp, sizeof(specLamp));
	/**/
	
	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

