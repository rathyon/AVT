#pragma once
#include "GL/glew.h"
#include "GL/freeglut.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
//#include "basic_geometry.h"

// THESE MUST MATCH THE ORDER IN Shader.h
#define VERTICES 0
#define NORMALS 1
#define TEXCOORDS 2
#define COLORS 3

typedef struct {
	GLfloat x, y, z;
} Vertex;

typedef struct {
	GLfloat u, v;
} Texcoord;

typedef struct {
	GLfloat nx, ny, nz;
} Normal;

typedef struct {
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	int texCount;
} Mat;

class Mesh
{
private:
	GLuint VAO;
	GLuint Vertex_VBO;
	GLuint Texcoord_VBO;
	GLuint Normal_VBO;
	GLsizei vertexCount;

	std::vector <Vertex> Vertices, vertexData;
	std::vector <Texcoord> Texcoords, texcoordData;
	std::vector <Normal> Normals, normalData;

	std::vector <unsigned int> vertexIdx, texcoordIdx, normalIdx;

	//aux funcs
	void parseVertex(std::stringstream& sin);
	void parseTexcoord(std::stringstream& sin);
	void parseNormal(std::stringstream& sin);
	void parseFace(std::stringstream& sin);
	void parseLine(std::stringstream& sin);

public:

	Mat mat;

	Mesh();
	Mesh(std::string);
	~Mesh();

	GLuint getVAO();
	GLsizei getVertexCount();

	// real funcs
	bool load(std::string);
};

