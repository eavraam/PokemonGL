#include "Terrain.h"
#include<stb/stb_image.h>

Terrain::Terrain(Shader shader, Model pinetree, Model rock)
{
	//const int TERRAIN_VERTEX_STEPS = 12;
	//const int TOTAL_VERTEX_COUNT = TERRAIN_VERTEX_STEPS * TERRAIN_VERTEX_STEPS;

	// Keep the TERRAIN_VERTEX_STEPS in a public int variable for later use
	//m_TERRAIN_VERTEX_STEPS = TERRAIN_VERTEX_STEPS;
	
	//terrainVertices[8 * TOTAL_VERTEX_COUNT];	// Serialize vertices, normals and uvs (x, y, z, nx, ny, nz, uvx, uvy) in an array
	//terrainIndices[6 * (TERRAIN_VERTEX_STEPS - 1) * (TERRAIN_VERTEX_STEPS - 1)]; // We 've got (TVC-1)*(TVC-1) blocks of 3-var vertices, we need 6 indices to draw a square block (2 triangles).


	
	//m_terrainIndices = terrainIndices[6 * (TERRAIN_VERTEX_STEPS - 1) * (TERRAIN_VERTEX_STEPS - 1)];


	// Props Model Matrices
	// Trees
	treeModelMatrices = new glm::mat4[TOTAL_VERTEX_COUNT];
	// Rocks
	rockModelMatrices = new glm::mat4[TOTAL_VERTEX_COUNT];

	// Set/Create Vertices and Indices
	setVertices(TERRAIN_VERTEX_STEPS, TOTAL_VERTEX_COUNT, terrainVertices);
	setIndices(TERRAIN_VERTEX_STEPS, TOTAL_VERTEX_COUNT, terrainIndices);
	// Pass Vertices and Indices to the Shader
	shaderConfiguration(shader, terrainVertices, terrainIndices);

	setupTrees(pinetree);
	setupRocks(rock);

}

void Terrain::setVertices(const int TERRAIN_VERTEX_STEPS, const int TOTAL_VERTEX_COUNT, float terrainVertices[])
{
	int vertexPointer = 0;
	for (unsigned int i = 0; i < TERRAIN_VERTEX_STEPS; i++) {
		for (unsigned int j = 0; j < TERRAIN_VERTEX_STEPS; j++) {
			// vec3 Coordinates
			terrainVertices[vertexPointer * 8] = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
			terrainVertices[vertexPointer * 8 + 1] = (rand() % (int)(2 * terrainHeight * 100)) / 100.0f - terrainHeight; // displace height in range [-terrainHeight, terrainHeight]
			//terrainVertices[vertexPointer * 8 + 1] = 0.0f;
			terrainVertices[vertexPointer * 8 + 2] = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
			// vec3 Normals
			terrainVertices[vertexPointer * 8 + 3] = 0.0f;
			terrainVertices[vertexPointer * 8 + 4] = 1.0f;
			terrainVertices[vertexPointer * 8 + 5] = 0.0f;
			// vec3 UVs
			terrainVertices[vertexPointer * 8 + 6] = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
			terrainVertices[vertexPointer * 8 + 7] = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
			// A counter for the terrain vertices "rows" of data
			vertexPointer++;

			// Check the 4 edges in order to place trees on top of the vertices, surrounding the terrain plane.
			// Top side
			if (i == 0)
			{
				transX = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
				transY = 0.0f;
				transZ = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(transX, transY, transZ + 0.3f));
				model = glm::scale(model, glm::vec3(0.003f, 0.002f, 0.003f));

				treeModelMatrices[treeModelMatricesCount] = model;
				treeModelMatricesCount++;
			}
			// Bottom side
			if (i == TERRAIN_VERTEX_STEPS - 1)
			{
				transX = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
				transY = 0.0f;
				transZ = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(transX, transY, transZ - 0.3f));
				model = glm::scale(model, glm::vec3(0.003f, 0.002f, 0.003f));

				treeModelMatrices[treeModelMatricesCount] = model;
				treeModelMatricesCount++;
			}
			// Left side
			if (j == 0)
			{
				transX = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
				transY = 0.0f;
				transZ = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(transX + 0.3f, transY, transZ));
				model = glm::scale(model, glm::vec3(0.003f, 0.002f, 0.003f));

				treeModelMatrices[treeModelMatricesCount] = model;
				treeModelMatricesCount++;
			}
			// Right side
			if (j == TERRAIN_VERTEX_STEPS - 1)
			{
				transX = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
				transY = 0.0f;
				transZ = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(transX - 0.3f, transY, transZ));
				model = glm::scale(model, glm::vec3(0.003f, 0.002f, 0.003f));

				treeModelMatrices[treeModelMatricesCount] = model;
				treeModelMatricesCount++;
			}
		}
	}
}

void Terrain::setIndices(const int TERRAIN_VERTEX_STEPS, const int TOTAL_VERTEX_COUNT, unsigned int terrainIndices[])
{
	// Set the terrain indices which creates the terrain grid.
	int pointer = 0;
	for (int gridZ = 0; gridZ < TERRAIN_VERTEX_STEPS - 1; gridZ++) {
		for (int gridX = 0; gridX < TERRAIN_VERTEX_STEPS - 1; gridX++) {
			int topLeft = (gridZ * TERRAIN_VERTEX_STEPS) + gridX;
			int topRight = topLeft + 1;
			int bottomLeft = ((gridZ + 1) * TERRAIN_VERTEX_STEPS) + gridX;
			int bottomRight = bottomLeft + 1;
			terrainIndices[pointer++] = topLeft;
			terrainIndices[pointer++] = bottomLeft;
			terrainIndices[pointer++] = topRight;
			terrainIndices[pointer++] = topRight;
			terrainIndices[pointer++] = bottomLeft;
			terrainIndices[pointer++] = bottomRight;
		}
	}
}

void Terrain::shaderConfiguration(Shader shader, float terrainVertices[], unsigned int terrainIndices[])
{
	shader.use();

	// Pass the Vertices and Indices to the Shader
	// -----------
	
	// Generate Buffers
	glGenVertexArrays(1, &terrainVAO);
	glGenBuffers(1, &terrainVBO);
	glGenBuffers(1, &terrainEBO);

	// Bind Buffers
	glBindVertexArray(terrainVAO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), &terrainVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), &terrainIndices, GL_STATIC_DRAW);

	// Set the Vertex Attribute Pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	// Vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	// Vertex UV's
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	// Unbind VAO
	glBindVertexArray(0);


	// Pass the Texture(s) to the Shader
	// -----------

	int imageWidth, imageHeight, nrChannels;
	unsigned char* data = stbi_load("Resources/textures/terrain/aerial_grass_rock/aerial_grass_rock_diff_1k.jpg", &imageWidth, &imageHeight, &nrChannels, 0);

	// Creates the terrain texture object
	
	glGenTextures(1, &terrainTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D
	(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		imageWidth,
		imageHeight,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		data
	);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenerateMipmap(GL_TEXTURE_2D);

	shader.setInt("terrain_diffuse", 0);
}

void Terrain::setupTrees(Model pinetree)
{
	// Configure Pinetree Tree Instanced Array
	// -------------------------
	
	glGenBuffers(1, &pinetreeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pinetreeVBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * TERRAIN_VERTEX_STEPS * sizeof(glm::mat4), &treeModelMatrices[0], GL_STATIC_DRAW);

	// set transformation matrices as an instance vertex attribute (with divisor 1)
	// note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
	// normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
	// -----------------------------------------------------------------------------------------------------------------------------------
	for (unsigned int i = 0; i < pinetree.meshes.size(); i++)
	{
		unsigned int VAO = pinetree.meshes[i].VAO;

		glBindVertexArray(VAO);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

void Terrain::setupRocks(Model rock)
{
	configureRocks();

	// Configure Rock Instanced Array
	// -------------------------

	
	glGenBuffers(1, &rockVBO);
	glBindBuffer(GL_ARRAY_BUFFER, rockVBO);
	glBufferData(GL_ARRAY_BUFFER, rockAmount * sizeof(glm::mat4), &rockModelMatrices[0], GL_STATIC_DRAW);

	// set transformation matrices as an instance vertex attribute (with divisor 1)
	// note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
	// normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
	// -----------------------------------------------------------------------------------------------------------------------------------
	for (unsigned int i = 0; i < rock.meshes.size(); i++)
	{
		unsigned int VAO = rock.meshes[i].VAO;
		glBindVertexArray(VAO);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

void Terrain::configureRocks()
{
	// Rock Generation with random translation, rotation and scale
	for (unsigned int i = 0; i < rockAmount; i++)
	{
		model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)rockAmount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		transX = sin(angle) * radius + displacement;
		transY = 0.0f;

		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		transZ = cos(angle) * radius + displacement;

		model = glm::translate(model, glm::vec3(transX, transY, transZ));

		// 2. scale: Scale between 0.03 and 0.06f
		float scale = static_cast<float>((rand() % 2) / 100.0 + 0.3);
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = static_cast<float>((rand() % 360));
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		rockModelMatrices[i] = model;
	}
}

void Terrain::render(Shader shader, Camera camera, int screenWidth, int screenHeight)
{
	shader.use();

	view = camera.GetViewMatrix();
	projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	shader.setMat4("model", model);

	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	glBindVertexArray(terrainVAO);
	glDrawElements(GL_TRIANGLES, sizeof(terrainIndices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}