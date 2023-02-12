#ifndef SKYBOX_H
#define SKYBOX_H

#include<iostream>

#include "Shader.h"
#include "Camera.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Skybox
{
public:

	// Values
	std::string facesCubemap[6] = {};
	// VAO, VBO, EBO
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;

	// Textures
	// Creates the cubemap texture object
	unsigned int cubemapTexture;
	int imageWidth, imageHeight, nrChannels;

	// Constructor
	Skybox(Shader shader);

	// Methods
	void init(Shader shader);
	void setFacesPaths(std::string facePaths[]);
	void createCubemapTexture(std::string facesCubemap[]);
	void render(Shader shader, Camera camera, int screenWidth, int screenHeight);

private:

	float skyboxVertices[24] =
	{
		//   Coordinates
		-1.0f, -1.0f,  1.0f,	//        7--------6
		 1.0f, -1.0f,  1.0f,	//       /|       /|
		 1.0f, -1.0f, -1.0f,	//      4--------5 |
		-1.0f, -1.0f, -1.0f,	//      | |      | |
		-1.0f,  1.0f,  1.0f,	//      | 3------|-2
		 1.0f,  1.0f,  1.0f,	//      |/       |/
		 1.0f,  1.0f, -1.0f,	//      0--------1
		-1.0f,  1.0f, -1.0f
	};

	unsigned int skyboxIndices[36] =
	{
		// Right
		1, 2, 6,
		6, 5, 1,
		// Left
		0, 4, 7,
		7, 3, 0,
		// Top
		4, 5, 6,
		6, 7, 4,
		// Bottom
		0, 3, 2,
		2, 1, 0,
		// Back
		0, 1, 5,
		5, 4, 0,
		// Front
		3, 7, 6,
		6, 2, 3
	};
};



#endif