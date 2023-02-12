#ifndef TERRAIN_H
#define TERRAIN_H

#include "Shader.h"
#include "Model.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Terrain
{
public:

	// Values
	
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	// ....
	static const int TERRAIN_VERTEX_STEPS = 12;
	static const int TOTAL_VERTEX_COUNT = TERRAIN_VERTEX_STEPS * TERRAIN_VERTEX_STEPS;

	float terrainVertices[8 * TOTAL_VERTEX_COUNT];
	unsigned int terrainIndices[6 * (TERRAIN_VERTEX_STEPS - 1) * (TERRAIN_VERTEX_STEPS - 1)];
	
	// Height adjustment
	float terrainHeight = 0.2;

	// Props
	glm::mat4* treeModelMatrices;
	glm::mat4* rockModelMatrices;
	
	int treeModelMatricesCount = 0;

	unsigned int rockAmount = 40;
	float radius = 2.0f;
	float offset = 3.5f;

	// VAO, VBO, EBO
	unsigned int terrainVAO, terrainVBO, terrainEBO;
	unsigned int terrainTexture;
	unsigned int pinetreeVBO;
	unsigned int rockVBO;
	
	// Translations for x, y, z needed to place the props
	float transX;
	float transY;
	float transZ;

	// Constructor
	Terrain(Shader shader, Model pinetree, Model rock);

	// Methods
	void setVertices(const int TERRAIN_VERTEX_STEPS, const int TOTAL_VERTEX_COUNT, float terrainVertices[]);
	void setIndices(const int TERRAIN_VERTEX_STEPS, const int TOTAL_VERTEX_COUNT, unsigned int terrainIndices[]);
	void shaderConfiguration(Shader shader, float terrainVertices[], unsigned int terrainIndices[]);

	void setupTrees(Model pinetree);
	void setupRocks(Model rock);
	void configureRocks();

	void render(Shader shader, Camera camera, int screenWidth, int screenHeight);

private:
	

	
};

#endif