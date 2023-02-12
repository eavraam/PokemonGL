#define STB_IMAGE_IMPLEMENTATION

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<gl/GL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "Skybox.h"
//#include "Terrain.h" // Terrain.h includes Model as well.

#include<iostream>

GLFWwindow* initializeGeneralSettings();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Model& c, int width, int height);
glm::vec3 mouse_raycast_callback(int mouse_x, int mouse_y);
bool mouseIntersectsAABB(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 boxMin, glm::vec3 boxMax);

// Screen Settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 800;

// Mouse
double mousePosX, mousePosY;

// Model/View/Projection Declarations
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

// Camera
Camera camera(glm::vec3(0.0f, 3.5f, 3.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool firstClick = true;

// Raycast
glm::vec3 raycastMouse(0.0f);

// Character
glm::vec3 characterPosition(0.0f);
glm::vec3 characterRotationAxis(0.0f, 0.1f, 0.0f);
float characterRotationDegrees = 0.0f;
bool canMove = true;
std::string keyPressedNow = "";
std::string keyPressedOnCollision = "";

// Pokeball
bool pokeballThrown = false;
int pokeballCollisionTimes = 0;
RigidBody pokeballTempRB;

// Pokemon
bool key1_wasPressed = false;
bool key2_wasPressed = false;
bool key3_wasPressed = false;
bool drawPokemonArray[3] = {false, false, false};
bool canCatch = false;

BoundingRegion initialModelBr[3]{};
BoundingRegion pokemonToCatchBr;

// Caught Pokemon Arrays
Model caughtModelArray[3]{};
RigidBody caughtRigidbodyArray[3]{};
// Temp models needed for the SWAP when catching Pokemon
Model tempModel;
RigidBody tempRigidbody;
string tempId;

// Timing
// Per-Frame Time Logic + FPS / ms Title
unsigned int counter = 0;	// Keeps track of the amount of frames in timeDiff

float currentFrame = 0.0f;
float lastFrame = 0.0f;
float deltaTime = 0.0f;

float particleCurrentFrame = 0.0f;
float particleLastFrame = 0.0f;
float particleDeltaTime = 0.0f;

std::string FPS;
std::string ms;
std::string newTitle;





// ---------------------------------------


struct Particle {
	float x, y, z;		// position
	float vx, vy, vz;	// velocity
	float r, g, b, a;	// color
	float lifetime;		// lifetime/duration
};

std::vector<Particle> particles;
const int NUM_PARTICLES = 10000;

void initParticles() {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		Particle p;
		p.x = 0.0f;
		p.y = 0.0f;
		p.z = 0.0f;
		p.r = (rand() / (float)RAND_MAX);
		p.g = (rand() / (float)RAND_MAX);
		p.b = (rand() / (float)RAND_MAX);
		//p.a = (rand() / (float)RAND_MAX);
		p.a = 0.0f;
		p.vx = (rand() / (float)RAND_MAX) * 0.1f - 0.05f;
		p.vy = (rand() / (float)RAND_MAX) * 0.1f;
		p.vz = (rand() / (float)RAND_MAX) * 0.1f - 0.05f;

		particles.push_back(p);
	}
}

void updateParticles(float dt) {
	for (Particle& p : particles) {
		p.x += p.vx;
		p.y += p.vy;
		p.z += p.vz;
		p.vy -= 0.001f;
	}
}

void renderParticles(int particleCount, Shader shader) {
	std::vector<float> particleData;
	particleData.reserve(particleCount * 7);
	// Create the model matrices for each particle instance
	std::vector<glm::mat4> particleModelMatrices;

	for (Particle& p : particles) {
		particleData.push_back(p.x);
		particleData.push_back(p.y);
		particleData.push_back(p.z);
		particleData.push_back(p.r);
		particleData.push_back(p.g);
		particleData.push_back(p.b);
		particleData.push_back(p.a);
	}

	model = glm::mat4(1.0f);
	view = camera.GetViewMatrix();
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	shader.use();

	unsigned int VAO, VBO;
	// Gen VAO, VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind VAO, VBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, particleData.size() * sizeof(float), particleData.data(), GL_STATIC_DRAW);
	// Pass the data to the shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set model, view, projection matrices and Draw
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	glDrawArrays(GL_POINTS, 0, particleCount);
	// Delete Buffers
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	// Unbind VAO
	glBindVertexArray(0);
}




// --------------------------------------
int main()
{
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		GENERAL SETTINGS																				//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	GLFWwindow* window;
	window = initializeGeneralSettings();

	// Debug disclaimer for keys
	cout << "PLAYER COMMANDS BELOW.\n" << endl;

	cout << "Press ESC (escape) to close the application." << endl;
	cout << "Use WASD keys to move.\n" << endl;

	cout << "Player starts with 3 Pokemon in slot (Sudowoodo, Espeon, Jirachi)." << endl;
	cout << "Press 1 to use Sudowoodo." << endl;
	cout << "Press 2 to use Espeon." << endl;
	cout << "Press 3 to use Jirachi.\n" << endl;

	cout << "If any Pokemon is used, the respective key will be disabled." << endl;
	cout << "To recatch the Pokemons, target (hover) them with your cursor and press F." << endl;
	cout << "This will send a Pokeball towards them and store them in the first available slot (key 1, key 2, and then key 3).\n" << endl;

	// Shaders
	// Default Shader where I use the lights.
	Shader defaultShader("default.vert", "default.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader modelInstancing("modelInstancing.vert", "modelInstancing.frag");
	Shader shadowMapShader("shadowMap.vert", "shadowMap.frag");
	Shader particleShader("particles.vert", "particles.frag");
	
	// Light Settings
	glm::vec3 lightPos = glm::vec3(15.0f, 15.0f, -5.0f);

	defaultShader.use();
	defaultShader.setVec3("lightPos", glm::vec3(lightPos.x, lightPos.y, lightPos.z));

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// -------------------------------------------------------------------- END OF GENERAL SETTINGS -----------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	
	

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		SKYBOX																							//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	Skybox skybox(skyboxShader);

	// All the faces of the cubemap (make sure they are in this exact order)
	std::string facesCubemap[6] =
	{
		"Resources/skybox/sky/right.jpg",
		"Resources/skybox/sky/left.jpg",
		"Resources/skybox/sky/top.jpg",
		"Resources/skybox/sky/bottom.jpg",
		"Resources/skybox/sky/front.jpg",
		"Resources/skybox/sky/back.jpg"
	};

	skybox.setFacesPaths(facesCubemap);

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// -------------------------------------------------------------------- END OF SKYBOX ---------------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//


	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		MODELS																							//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	// Models' Instances
	// --------------------
	// Some textures must be inverted, so instead of inverting them myself, I just do this.
	stbi_set_flip_vertically_on_load(false);
	
	Model character(BoundTypes::AABB, "Resources/models/torchic/scene.gltf", 0.01f);
	Model borderTree(BoundTypes::AABB, "Resources/models/lowpoly_pinetree/scene.gltf", 0.003f);
	Model rock(BoundTypes::AABB, "Resources/models/lowpoly_rock/scene.gltf", 0.4f);
	Model pokeball(BoundTypes::SPHERE, "Resources/models/pokeball/scene.gltf", 0.001f);
	Model sudowoodo(BoundTypes::AABB, "Resources/models/sudowoodo/scene.gltf", 0.5f);
	Model espeon(BoundTypes::AABB, "Resources/models/espeon/scene.gltf", 0.005f);
	Model jirachi(BoundTypes::AABB, "Resources/models/jirachi/scene.gltf", 0.01f);

	// Set Sudowoodo's pos.y to 0.0f, because the model's y is not set to 0.0f;
	sudowoodo.rb.pos.y = 0.0f;

	// Set Id's to the models for future use
	sudowoodo.id = "sudowoodo";
	espeon.id = "espeon";
	jirachi.id = "jirachi";

	// Store the Models and their Rigidbodies in arrays
	// Model Array
	caughtModelArray[0] = sudowoodo;
	caughtModelArray[1] = espeon;
	caughtModelArray[2] = jirachi;
	// Rigidbody Array
	caughtRigidbodyArray[0] = sudowoodo.rb;
	caughtRigidbodyArray[1] = espeon.rb;
	caughtRigidbodyArray[2] = jirachi.rb;

	// invert textures on load reset
	stbi_set_flip_vertically_on_load(true);

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// -------------------------------------------------------------------- END OF MODELS ---------------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//



	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		PHYSICS																							//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	//Character
	// Make character's bounding region smaller
	//character.meshes[0].br.min /= 10.0f;
	//character.meshes[0].br.max /= 10.0f;

	// Pokeball
	pokeball.rb.applyAcceleration(Environment::gravitationalAcceleration);
	pokeball.rb.pos = glm::vec3(0.0f, -10.0f, 0.0f);			// Initialize Pokeball's position far away, so I don't get errors later on with collisions
	pokeball.meshes[0].br.center = pokeball.rb.pos;	// Initialize pokeball's bounding region "outside" the scene
	pokeballTempRB = pokeball.rb;
	
	// Store initial Models' Bounding Regions
	for (unsigned int i = 0; i < 3; i++)
	{
		for (unsigned int j = 0; j < caughtModelArray[i].meshes.size(); j++)
		{
			initialModelBr[i].min = caughtModelArray[i].meshes[j].br.min;
			initialModelBr[i].max = caughtModelArray[i].meshes[j].br.max;
		}
	}

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// -------------------------------------------------------------------- END OF PHYSICS --------------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		PARTICLES																						//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	// Set particle point size
	glPointSize(5.0f);
	// Initiate Particles
	initParticles();



	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// -------------------------------------------------------------------- END OF PARTICLES ---------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//




	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		TERRAIN																							//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	//Terrain terrain(defaultShader, borderTree, rock);

	// TERRAIN VERTEX STEPS AND TOTAL COUNT
	// CONSTANTS
	const int TERRAIN_VERTEX_STEPS = 15;
	const int TOTAL_VERTEX_COUNT = TERRAIN_VERTEX_STEPS * TERRAIN_VERTEX_STEPS;
	const int BORDER_TREES_COUNT = 4 * TERRAIN_VERTEX_STEPS;	// Amount of "border" trees in the scene
	const unsigned int ROCK_COUNT = 20;							// Amount of rocks in the scene

	// Terrain Vertex and Index
	float terrainVertices[8 * TOTAL_VERTEX_COUNT];	// Serialize vertices, normals and uvs (x, y, z, nx, ny, nz, uvx, uvy) in an array
	unsigned int terrainIndices[6 * (TERRAIN_VERTEX_STEPS - 1) * (TERRAIN_VERTEX_STEPS - 1)]; // We 've got (TVC-1)*(TVC-1) blocks of 3-var vertices, we need 6 indices to draw a square block (2 triangles).
	
	// Model Matrices
	// Tree Model Matrices
	glm::mat4* borderTreeModelMatrices;
	borderTreeModelMatrices = new glm::mat4[BORDER_TREES_COUNT];
	int borderTreeModelMatricesCount = 0;

	// Rock Model Matrices
	glm::mat4* rockModelMatrices;
	rockModelMatrices = new glm::mat4[TOTAL_VERTEX_COUNT];

	//Model Arrays
	// Tree Model list, in order to store their positions and bounding regions
	glm::vec3 borderTreeTranslation(0.0f);
	vector<glm::vec3> borderTreePositions;
	// Rock Model list, in order to store their positions and bounding regions
	glm::vec3 rockTranslation(0.0f);
	vector<glm::vec3> rockPositions;

	// Terrain BoundingRegions
	BoundingRegion tempBr;								// General use temp Br
	BoundingRegion groundBr;							// "Flat" Ground Bounding Region
	BoundingRegion borderTreeBr[BORDER_TREES_COUNT];	// borderTreeBoundingRegion
	BoundingRegion rockBr[ROCK_COUNT];					//RockBoundingRegion
	BoundingRegion borderBr[4];							// Square Scene Border Bounding Region
	
	// Height adjustment
	float terrainHeight = 0.2;

	// Translations for x, y, z needed to place the props
	float transX;
	float transY;
	float transZ;

	// Ground Bounding Region
	groundBr.min = glm::vec3(-((float)TERRAIN_VERTEX_STEPS / 2.0f), -0.5f, -((float)TERRAIN_VERTEX_STEPS / 2.0f));
	groundBr.max = glm::vec3( ((float)TERRAIN_VERTEX_STEPS / 2.0f),  0.0f,  ((float)TERRAIN_VERTEX_STEPS / 2.0f));

	/*cout << "min x: " + std::to_string(groundBr.min.x) + " " << "min y: " + std::to_string(groundBr.min.y) + " " << "min z: " + std::to_string(groundBr.min.z) << endl;
	cout << "max x: " + std::to_string(groundBr.max.x) + " " << "max y: " + std::to_string(groundBr.max.y) + " " << "max z: " + std::to_string(groundBr.max.z) << endl;*/

	// Border Bounding Region
	// Top Border
	borderBr[0].min = glm::vec3(-((float)TERRAIN_VERTEX_STEPS / 2.0f), -0.5f, -((float)TERRAIN_VERTEX_STEPS / 2.0f));
	borderBr[0].max = glm::vec3( ((float)TERRAIN_VERTEX_STEPS / 2.0f),  4.0f, -((float)TERRAIN_VERTEX_STEPS / 2.0f) + 1.0f);
	// Bottom Border
	borderBr[1].min = glm::vec3(-((float)TERRAIN_VERTEX_STEPS / 2.0f), -0.5f, ((float)TERRAIN_VERTEX_STEPS / 2.0f) - 2.0f);
	borderBr[1].max = glm::vec3(((float)TERRAIN_VERTEX_STEPS / 2.0f),   4.0f, ((float)TERRAIN_VERTEX_STEPS / 2.0f));
	// Left Border
	borderBr[2].min = glm::vec3(-((float)TERRAIN_VERTEX_STEPS / 2.0f),			-0.5f, -((float)TERRAIN_VERTEX_STEPS / 2.0f));
	borderBr[2].max = glm::vec3(-((float)TERRAIN_VERTEX_STEPS / 2.0f) + 1.0f,    4.0f,  ((float)TERRAIN_VERTEX_STEPS / 2.0f));
	// Right Border
	borderBr[3].min = glm::vec3(((float)TERRAIN_VERTEX_STEPS / 2.0f) - 2.0f,	-0.5f, -((float)TERRAIN_VERTEX_STEPS / 2.0f));
	borderBr[3].max = glm::vec3(((float)TERRAIN_VERTEX_STEPS / 2.0f),			 4.0f,  ((float)TERRAIN_VERTEX_STEPS / 2.0f));

	// "Grid" Terrain Vertices/Indices Calculation + Border Trees
	int vertexPointer = 0;
	for (unsigned int i = 0; i < TERRAIN_VERTEX_STEPS; i++) {
		for (unsigned int j = 0; j < TERRAIN_VERTEX_STEPS; j++) {
			// vec3 Coordinates
			terrainVertices[vertexPointer * 8] = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
			//terrainVertices[vertexPointer * 8 + 1] = (rand() % (int)(2 * terrainHeight * 100)) / 100.0f - terrainHeight; // displace height in range [-terrainHeight, terrainHeight]
			terrainVertices[vertexPointer * 8 + 1] = 0.0f;
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

			// BorderTree Instance Position Calculation
			transX = (float)j - ((float)TERRAIN_VERTEX_STEPS / 2.0f);
			transY = 0.0f;
			transZ = (float)i - ((float)TERRAIN_VERTEX_STEPS / 2.0f);

			// Check the 4 edges in order to place trees on top of the vertices, surrounding the terrain plane.
			// Top side
			if (i == 0)
			{
				borderTreeTranslation = glm::vec3(transX, transY, transZ + 0.3f);

				// Model setup
				model = glm::mat4(1.0f);
				model = glm::translate(model, borderTreeTranslation);
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

				// Model Matrices 
				borderTreeModelMatrices[borderTreeModelMatricesCount] = model;

				// Store Tree Positions
				borderTreePositions.push_back(borderTreeTranslation);

				// Bounding Region Calculation
				tempBr.min = borderTree.meshes[0].br.min + borderTreeTranslation;
				tempBr.max = borderTree.meshes[0].br.max + borderTreeTranslation;

				// Store Bounding Regions
				borderTreeBr[borderTreeModelMatricesCount] = tempBr;

				// Update counter
				borderTreeModelMatricesCount++;
			}
			// Bottom side
			if (i == TERRAIN_VERTEX_STEPS - 1)
			{
				borderTreeTranslation = glm::vec3(transX, transY, transZ - 0.3f);

				// Model setup
				model = glm::mat4(1.0f);
				model = glm::translate(model, borderTreeTranslation);
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

				// Model Matrices 
				borderTreeModelMatrices[borderTreeModelMatricesCount] = model;

				// Store Tree Positions
				borderTreePositions.push_back(borderTreeTranslation);

				// Bounding Region Calculation
				tempBr.min = borderTree.meshes[0].br.min + borderTreeTranslation;
				tempBr.max = borderTree.meshes[0].br.max + borderTreeTranslation;

				// Store Bounding Regions
				borderTreeBr[borderTreeModelMatricesCount] = tempBr;

				// Update counter
				borderTreeModelMatricesCount++;
			}
			// Left side
			if (j == 0)
			{
				borderTreeTranslation = glm::vec3(transX + 0.3f, transY, transZ);

				// Model setup
				model = glm::mat4(1.0f);
				model = glm::translate(model, borderTreeTranslation);
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

				// Model Matrices
				borderTreeModelMatrices[borderTreeModelMatricesCount] = model;

				// Store Tree Positions
				borderTreePositions.push_back(borderTreeTranslation);

				// Bounding Region Calculation
				tempBr.min = borderTree.meshes[0].br.min + borderTreeTranslation;
				tempBr.max = borderTree.meshes[0].br.max + borderTreeTranslation;

				// Store Bounding Regions
				borderTreeBr[borderTreeModelMatricesCount] = tempBr;

				// Update counter
				borderTreeModelMatricesCount++;
			}
			// Right side
			if (j == TERRAIN_VERTEX_STEPS - 1)
			{
				borderTreeTranslation = glm::vec3(transX - 0.3f, transY, transZ);

				// Model setup
				model = glm::mat4(1.0f);
				model = glm::translate(model, borderTreeTranslation);
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

				// Model Matrices
				borderTreeModelMatrices[borderTreeModelMatricesCount] = model;

				// Store Tree Positions
				borderTreePositions.push_back(borderTreeTranslation);

				// Bounding Region Calculation
				tempBr.min = borderTree.meshes[0].br.min + borderTreeTranslation;
				tempBr.max = borderTree.meshes[0].br.max + borderTreeTranslation;

				// Store Bounding Regions
				borderTreeBr[borderTreeModelMatricesCount] = tempBr;

				// Update counter
				borderTreeModelMatricesCount++;
			}
		}
	}

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

	// Rock Generation with random translation, rotation and scale
	float radius = 4.0f;
	float offset = 3.5f;
	for (unsigned int i = 0; i < ROCK_COUNT; i++)
	{
		// Position Calculation
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)ROCK_COUNT * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		
		transX = sin(angle) * radius + displacement;
		transY = 0.0f;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		transZ = cos(angle) * radius + displacement;

		rockTranslation = glm::vec3(transX, transY, transZ);

		// Model Setup
		model = glm::mat4(1.0f);
		model = glm::translate(model, rockTranslation);
		model = glm::scale(model, glm::vec3(1.0f));
		// rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = static_cast<float>((rand() % 360));
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// Model Matrices
		rockModelMatrices[i] = model;

		// Store rock positions
		rockPositions.push_back(rockTranslation);

		// Bounding Region Calculation
		tempBr.min = rock.meshes[0].br.min + rockTranslation;
		tempBr.max = rock.meshes[0].br.max + rockTranslation;

		// Store Bounding Regions
		rockBr[i] = tempBr;
	}
	
	// CONFIGURE TERRAIN PLANE
	defaultShader.use();

	unsigned int terrainVAO, terrainVBO, terrainEBO;

	// create buffers/arrays
	glGenVertexArrays(1, &terrainVAO);
	glGenBuffers(1, &terrainVBO);
	glGenBuffers(1, &terrainEBO);

	glBindVertexArray(terrainVAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), &terrainVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), &terrainIndices, GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glBindVertexArray(0);

	int widthImg, heightImg, nrChannels;
	unsigned char* data = stbi_load("Resources/textures/terrain/aerial_grass_rock/aerial_grass_rock_diff_1k.jpg", &widthImg, &heightImg, &nrChannels, 0);

	// Creates the terrain texture object
	unsigned int terrainTexture;
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
		widthImg,
		heightImg,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		data
	);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenerateMipmap(GL_TEXTURE_2D);

	defaultShader.setInt("terrain_diffuse", 0);
	

	// Configure Pinetree Tree Instanced Array
	// -------------------------
	unsigned int pinetreeVBO;
	glGenBuffers(1, &pinetreeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pinetreeVBO);
	glBufferData(GL_ARRAY_BUFFER, BORDER_TREES_COUNT * sizeof(glm::mat4), &borderTreeModelMatrices[0], GL_STATIC_DRAW);

	// set transformation matrices as an instance vertex attribute (with divisor 1)
	// note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
	// normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
	// -----------------------------------------------------------------------------------------------------------------------------------
	for (unsigned int i = 0; i < borderTree.meshes.size(); i++)
	{
		unsigned int VAO = borderTree.meshes[i].VAO;
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

	// Configure Rock Instanced Array
	// -------------------------
	
	unsigned int rockVBO;
	glGenBuffers(1, &rockVBO);
	glBindBuffer(GL_ARRAY_BUFFER, rockVBO);
	glBufferData(GL_ARRAY_BUFFER, ROCK_COUNT * sizeof(glm::mat4), &rockModelMatrices[0], GL_STATIC_DRAW);

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

	

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// -------------------------------------------------------------------- END OF TERRAIN --------------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//



	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//																		SHADOW MAP																						//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//


	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// ----------------------------------------------------------------- END OF SHADOW MAP --------------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//



	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	//														SETUP BEFORE INFINITE LOOP																						//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	


	// collision counter
	int collisionCounter = 0;

	// Helping Tools
	float rotationRate = 0.5f;
	float pokemonRotation = 0.0f;

	// Initialize Collision Position
	glm::vec3 collisionPosition(0.0f);

	// Shadow Setup
	float near_plane = 0.1f, far_plane = 45.0f;
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;


	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//
	// ------------------------------------------------------- END OF SETUP BEFORE INFINITE LOOP --------------------------------------------------------------------------	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------	//

	// MAIN WHILE LOOP
	while (!glfwWindowShouldClose(window))
	{
		
		// Per-Frame Time Logic
		// --------------------

		particleCurrentFrame = glfwGetTime();
		particleDeltaTime = particleCurrentFrame - particleLastFrame;
		particleLastFrame = particleCurrentFrame;

		currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		counter++;

		if (deltaTime >= 1.0 / 120.0)
		{
			// Creates new title
			FPS = std::to_string((1.0 / deltaTime) * counter);
			ms = std::to_string((deltaTime / counter) * 1000);
			newTitle = "PokemonGL - " + FPS + "FPS / " + ms + "ms";
			glfwSetWindowTitle(window, newTitle.c_str());

			// Resets times and counter
			lastFrame = currentFrame;
			counter = 0;

			// Inputs (Keyboard, Mouse)
			processInput(window, character, SCR_WIDTH, SCR_HEIGHT);
		}		
		
		// Shadow Matrices
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;

		// Activate ShadowMap Shader and pass the lightSpaceMatrix to the Shader.
		shadowMapShader.use();
		shadowMapShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		// General settings
		pokemonRotation += rotationRate;

		// Shadow General settings and Bind depthMap FBO
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		
		// Shadow Character Render
		// --------------------
		model = glm::mat4(1.0f);
		model = glm::translate(model, characterPosition);
		model = glm::rotate(model, glm::radians(characterRotationDegrees), characterRotationAxis);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		shadowMapShader.setMat4("model", model);

		// Draw the Model
		character.Draw(shadowMapShader);

		// Shadow Terrain Render
		// --------------------

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	
		shadowMapShader.setMat4("model", model);

		glBindTexture(GL_TEXTURE_2D, terrainTexture);
		glBindVertexArray(terrainVAO);
		glDrawElements(GL_TRIANGLES, sizeof(terrainIndices), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);


		// Shadow Pokeball Model Render
		// --------------------
		// Uses the default shader, which is called earlier
		model = glm::mat4(1.0f);
		model = glm::translate(model, pokeball.rb.pos);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		
		shadowMapShader.setMat4("model", model);
		
		if (pokeballThrown == true)
		{
			pokeball.Draw(shadowMapShader);
		}


		// Shadow Pokemon Models Render
		// --------------------
		// Uses the shadow map shader, which is called earlier
		for (unsigned int r = 0; r < 3; r++)
		{
			if (drawPokemonArray[r] == true)
			{
				model = glm::mat4(1.0f);
				model = glm::translate(model, caughtRigidbodyArray[r].pos);
				model = glm::rotate(model, glm::radians(pokemonRotation), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			
				shadowMapShader.setMat4("model", model);

				caughtModelArray[r].Draw(shadowMapShader);
			}
		}
		

		// Render scene
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Color & Buffers
		// --------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		//// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		//// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		// RENDERS
		// --------------------
		//View/Projection Transformations
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		
		// Particle Render
		// Update / Animate particles' positions
		updateParticles(particleDeltaTime);
		// Render particles
		renderParticles(particles.size(), particleShader);


		// Default Shader Use
		defaultShader.use();
		defaultShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		defaultShader.setInt("shadowMap", 2);

		// Character Model Render
		// --------------------
		// Uses the default shader, which is called earlier
		model = glm::mat4(1.0f);
		model = glm::translate(model, characterPosition);
		model = glm::rotate(model, glm::radians(characterRotationDegrees), characterRotationAxis);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		defaultShader.setMat4("view", view);
		defaultShader.setMat4("projection", projection);
		defaultShader.setMat4("model", model);

		// Draw the Model
		character.Draw(defaultShader);


		// Pokeball Model Render
		// --------------------
		// Uses the default shader, which is called earlier
		defaultShader.use();

		if (pokeballThrown == true)
		{

			pokeball.rb.update(deltaTime);
			pokeballTempRB.update(deltaTime);

			pokeballTempRB.acceleration = Environment::gravitationalAcceleration;

			pokeball.rb = pokeballTempRB;
			pokeball.meshes[0].br.center = pokeball.rb.pos;

			model = glm::mat4(1.0f);
			model = glm::translate(model, pokeball.rb.pos);
			model = glm::rotate(model, glm::radians(270.0f), glm::vec3(90.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

			defaultShader.setMat4("view", view);
			defaultShader.setMat4("projection", projection);
			defaultShader.setMat4("model", model);
			
			pokeball.Draw(defaultShader);
			
			// If I get >=3 Collisions, Reset Pokeball
			// If pokeball.pos.y <= -1.0f, it means that it was accidentally thrown out of the bounds of the scene, so Reset Pokeball
			if (pokeballCollisionTimes >= 3 || pokeballTempRB.pos.y <= -1.0f)
			{
				// Store current position where the last collision happened
				collisionPosition = glm::vec3(pokeball.rb.pos.x, 0.0f, pokeball.rb.pos.z);

				// Reset Pokeball Settings
				pokeballThrown = false;
				pokeballCollisionTimes = 0;
				pokeballTempRB.reset();
				pokeball.rb.reset();
				pokeball.meshes[0].br.center = pokeball.rb.pos;

				// Draw Pokemon stored in place/key number 1
				if (key1_wasPressed == true)
				{
					// Make First Pokemon appear
					caughtRigidbodyArray[0].pos = collisionPosition;
					drawPokemonArray[0] = true;

					// Update the key1_wasPressed boolean lock
					key1_wasPressed = false;
				}
				// Draw Pokemon stored in place/key number 2
				else if (key2_wasPressed == true)
				{
					// Make Second Pokemon appear
					caughtRigidbodyArray[1].pos = collisionPosition;
					drawPokemonArray[1] = true;

					// Update the key2_wasPressed boolean lock
					key2_wasPressed = false;
				}
				// Draw Pokemon stored in place/key number 3
				else if (key3_wasPressed == true)
				{
					// Make Third appear
					caughtRigidbodyArray[2].pos = collisionPosition;
					drawPokemonArray[2] = true;

					// Update the key3_wasPressed boolean lock
					key3_wasPressed = false;
				}

				
			}
		}


		// Pokemon Models Render
		// --------------------
		for (unsigned int r = 0; r < 3; r++)
		{
			if (drawPokemonArray[r] == true)
			{
				for (unsigned int i = 0; i < caughtModelArray[r].meshes.size(); i++)
				{
					caughtModelArray[r].meshes[i].br.min = initialModelBr[r].min + caughtRigidbodyArray[r].pos;
					caughtModelArray[r].meshes[i].br.max = initialModelBr[r].max + caughtRigidbodyArray[r].pos;
				}

				defaultShader.use();

				model = glm::mat4(1.0f);
				model = glm::translate(model, caughtRigidbodyArray[r].pos);
				model = glm::rotate(model, glm::radians(pokemonRotation), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

				defaultShader.setMat4("view", view);
				defaultShader.setMat4("projection", projection);
				defaultShader.setMat4("model", model);

				caughtModelArray[r].Draw(defaultShader);
			}
		}


		// Terrain Render
		// --------------------
		// Uses the default shader, which is called earlier
		
		//terrain.render(defaultShader, camera, SCR_WIDTH, SCR_HEIGHT);

		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		defaultShader.setMat4("view", view);
		defaultShader.setMat4("projection", projection);
		defaultShader.setMat4("model", model);

		glBindTexture(GL_TEXTURE_2D, terrainTexture);
		glBindVertexArray(terrainVAO);
		glDrawElements(GL_TRIANGLES, sizeof(terrainIndices), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		


		// Instanced Pinetree Tree Model Render
		// --------------------
		// Uses the instancing shader, used for the multiple one-draw-call instanced models
		modelInstancing.use();

		modelInstancing.setMat4("view", view);
		modelInstancing.setMat4("projection", projection);
		modelInstancing.setInt("texture_diffuse1", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, borderTree.textures_loaded[0].id);

		// Draw the Model
		for (unsigned int i = 0; i < borderTree.meshes.size(); i++)
		{
			glBindVertexArray(borderTree.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(borderTree.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, BORDER_TREES_COUNT);
			glBindVertexArray(0);		
		}

		// Instanced Rock Model Render
		// --------------------
		// Uses the instancing shader, used for the multiple one-draw-call instanced models, called earlier.
		modelInstancing.setMat4("view", view);
		modelInstancing.setMat4("projection", projection);

		modelInstancing.setInt("texture_diffuse1", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);

		// Draw the Model
		for (unsigned int i = 0; i < rock.meshes.size(); i++)
		{
			glBindVertexArray(rock.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, ROCK_COUNT);
			glBindVertexArray(0);
		}




		// COLLISIONS
		// ------------------------------------------

		// Check if pokeball has already collided 3 times. If not, play
		if (pokeballCollisionTimes <= 3)
		{
			// Pokeball Collision with Ground
			if (pokeball.meshes[0].br.intersectsWith(groundBr))
			{
				// Update the pokeballTempRB (which controls the pokeball Rigidbody)
				pokeballTempRB.velocity = glm::vec3(pokeballTempRB.velocity.x, 0.7f * abs(pokeballTempRB.velocity.y), pokeballTempRB.velocity.z);

				// Count how many times the Pokeball has Collided
				pokeballCollisionTimes += 1;
			}

			// Pokeball Collision with the Borders of the scene
			for (unsigned int j = 0; j < 4; j++)
			{
				if (pokeball.meshes[0].br.intersectsWith(borderBr[j]))
				{
					// Count how many times the Pokeball has Collided
					pokeballCollisionTimes += 1;

					// Check which border the pokeball hit
					// Top/Bottom Borders
					if (j == 0 || j == 1)
					{
						// Update the pokeballTempRB (which controls the pokeball Rigidbody)
						// Reverse z-axis
						pokeballTempRB.velocity = glm::vec3(pokeballTempRB.velocity.x, 0.7f * abs(pokeballTempRB.velocity.y), - pokeballTempRB.velocity.z);
					}
					
					// Left/Right Borders
					else if (j == 2 || j == 3)
					{
						// Update the pokeballTempRB (which controls the pokeball Rigidbody)
						// Reverse x-axis
						pokeballTempRB.velocity = glm::vec3( - pokeballTempRB.velocity.x, 0.7f * abs(pokeballTempRB.velocity.y), pokeballTempRB.velocity.z);
					}
					
				}
			}

			// Pokeball Collision with Rocks
			for (unsigned int j = 0; j < rockPositions.size(); j++)
			{
				if (pokeball.meshes[0].br.intersectsWith(rockBr[j]))
				{
					// Count how many times the Pokeball has Collided
					pokeballCollisionTimes += 1;

					// Pokeball is on the right||left of the rock, so the Pokeball should bounce "right"||"left"
					if (pokeball.meshes[0].br.calculateCenter().x >= rockBr[j].calculateCenter().x
						|| pokeball.meshes[0].br.calculateCenter().x <= rockBr[j].calculateCenter().x)
					{
						//pokeballTempRB.velocity = glm::vec3(-pokeballTempRB.velocity.x, 0.7f * abs(pokeballTempRB.velocity.y), pokeballTempRB.velocity.z);
						pokeballTempRB.velocity *= glm::vec3(-1.0f, 0.7f, 1.0f);
					}
					// Pokeball is on the front||back of the rock, so the Pokeball should bounce "down"
					else if (pokeball.meshes[0].br.calculateCenter().z >= rockBr[j].calculateCenter().z
							 || pokeball.meshes[0].br.calculateCenter().z <= rockBr[j].calculateCenter().z)
					{
						//pokeballTempRB.velocity = glm::vec3(pokeballTempRB.velocity.x, 0.7f * abs(pokeballTempRB.velocity.y), -pokeballTempRB.velocity.z);
						pokeballTempRB.velocity *= glm::vec3(1.0f, 0.7f, -1.0f);
					}
				}
			}
		}

		// Check Pokeball collisions with Pokemons, and Store
		for (unsigned int i = 0; i < 3; i++)
		{
			for (unsigned int j = 0; j < caughtModelArray[i].meshes.size(); j++)
			{
				if (pokeball.meshes[0].br.intersectsWith(caughtModelArray[i].meshes[j].br))
				{
					cout << "pokeball collision with Pokemon: " + std::to_string(i) << endl;

					// SWAP between the active and stored models for each key in an increasing manner
					// Check if Pokemon in key1 is drawn. If yes, its place is available.
					// If not, this means there is a Pokemon already stored in key1, so check key 2.
					// etc.

					// If first Pokemon is drawn, its place is available for storing a "caught" Pokemon
					if (drawPokemonArray[0] == true)
					{
						// Store the first Pokemon
						tempModel = caughtModelArray[0];
						tempRigidbody = caughtRigidbodyArray[0];
						
						// Put the "caught" Pokemon in the first place
						caughtModelArray[0] = caughtModelArray[i];
						caughtRigidbodyArray[0] = caughtRigidbodyArray[i];

						// Put the stored Pokemon in the "caught" Pokemon's place
						caughtModelArray[i] = tempModel;
						caughtRigidbodyArray[i] = tempRigidbody;

						// Pokemon on key 1 should not be drawn
						drawPokemonArray[0] = false;

						break;
					}
					// If second Pokemon is drawn, its place is available for storing a "caught" Pokemon
					else if (drawPokemonArray[1] == true)
					{
						// Store the second Pokemon
						tempModel = caughtModelArray[1];
						tempRigidbody = caughtRigidbodyArray[1];

						// Put the "caught" Pokemon in the second place
						caughtModelArray[1] = caughtModelArray[i];
						caughtRigidbodyArray[1] = caughtRigidbodyArray[i];

						// Put the stored Pokemon in the "caught" Pokemon's place
						caughtModelArray[i] = tempModel;
						caughtRigidbodyArray[i] = tempRigidbody;

						// Pokemon on key 2 should not be drawn
						drawPokemonArray[1] = false;

						break;
					}
					// If third Pokemon is drawn, its place is available for storing a "caught" Pokemon
					else if (drawPokemonArray[2] == true)
					{
						// Store the third Pokemon
						tempModel = caughtModelArray[2];
						tempRigidbody = caughtRigidbodyArray[2];

						// Put the "caught" Pokemon in the third place
						caughtModelArray[2] = caughtModelArray[i];
						caughtRigidbodyArray[2] = caughtRigidbodyArray[i];

						// Put the stored Pokemon in the "caught" Pokemon's place
						caughtModelArray[i] = tempModel;
						caughtRigidbodyArray[i] = tempRigidbody;

						// Pokemon on key 3 should not be drawn
						drawPokemonArray[2] = false;

						break;
					}
				}
			}

		}
		

		// ------------------------------
		// RAY - CASTING
		// ------------------------------
		
		// Get the mouse raycast direction
		raycastMouse = mouse_raycast_callback(mousePosX, mousePosY);

		

		// Check if mouse is raycasting towards any Pokemon
		for (unsigned int j = 0; j < 3; j++)
		{
			for (unsigned int i = 0; i < caughtModelArray[j].meshes.size(); i++)
			{
				if (mouseIntersectsAABB(camera.Position, raycastMouse, caughtModelArray[j].meshes[i].br.min, caughtModelArray[j].meshes[i].br.max) == true)
				{
					// Enable canCatch lock to capture Pokemon
					canCatch = true;

					// Define the Bounding Region to which we shoot the "capture" Pokeball
					// Here, I set it to the target Pokemon's *first* mesh (for these models, mesh[0] is the main body)
					pokemonToCatchBr.min = caughtModelArray[j].meshes[i].br.min;
					pokemonToCatchBr.max = caughtModelArray[j].meshes[i].br.max;

					break;
				}
				else
				{
					canCatch = false;
				}
			}
			if (canCatch == true)
			{
				break;
			}
		}

		
		
		// Character Collision with Border Trees
		if (canMove == true)
		{
			for (unsigned int i = 0; i < character.meshes.size(); i++)
			{
				for (unsigned int j = 0; j < 4; j++)
				{
					if (character.meshes[i].br.intersectsWith(borderBr[j]))
					{
						cout << "Hello Border Tree " + std::to_string(collisionCounter) << endl;
						collisionCounter += 1;

						// Lock movement after collision
						canMove = false;
						keyPressedOnCollision = keyPressedNow;
					}
				}
			}
		}
		
		// Character Collision with Rocks
		if (canMove == true)
		{
			for (unsigned int i = 0; i < character.meshes.size(); i++)
			{
				for (unsigned int j = 0; j < rockPositions.size(); j++)
				{
					if (character.meshes[i].br.intersectsWith(rockBr[j]))
					{
						cout << "Hello Rock: " + std::to_string(collisionCounter) << endl;
						collisionCounter += 1;

						// Lock movement after collision
						canMove = false;
						keyPressedOnCollision = keyPressedNow;
					}
				}
			}
		}


		// Unlock movement after collisions
		if (keyPressedNow != keyPressedOnCollision)
		{
			canMove = true;

		}

			
		// Skybox Render (MAKE IT THE LAST DRAWING FOR PERFORMANCE)
		// -------------------- 
		skybox.render(skyboxShader, camera, SCR_WIDTH, SCR_HEIGHT);


		// -----	REST OF THE STUFF	----- //
		// --------------------
		glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);	// Adjust the viewport when the window is resized
		framebuffer_size_callback(window, SCR_WIDTH, SCR_HEIGHT);	// Make sure the viewport matches the new window dimensions 
		glfwSwapBuffers(window);									// Swap the back buffer with the front buffer
		glfwPollEvents();											// Take care of all GLFW events
		glfwGetCursorPos(window, &mousePosX, &mousePosY);			// Get and update the cursor's position

		//mouse_callback(window, mousePosX, mousePosY);				// Move camera according to mouse movement
	}

	// -----	CLEAN-UP CREW	----- //
	// Delete Shader Program(s)
	defaultShader.deleteProgram();
	skyboxShader.deleteProgram();
	modelInstancing.deleteProgram();
	shadowMapShader.deleteProgram();
	particleShader.deleteProgram();
	// Delete Textures
	glDeleteTextures(1, &terrainTexture);
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();

	return 0;
}

// Initialize the OpenGL configurations
GLFWwindow* initializeGeneralSettings()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "PokemonGL"
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PokemonGL", NULL, NULL);
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);
	// Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case, the viewport goes from x=0, y=0 to x=800, y=800
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Locks the mouse inside the application window and makes it invisible
	// If you want it to function normally, but hide when inside the window
	// then change GLFW_CURSOR_DISABLED to GLFW_CURSOR_HIDDEN
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Enable the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Enable VSync to lock FPS to 60.
	// To disable VSync, use glfwSwapInterval(0);
	glfwSwapInterval(1);

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Model& c, int width, int height)
{
	float vel = camera.MovementSpeed * deltaTime;
	// Keyboard Inputs
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Move UP
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		keyPressedNow = "W";

		if (canMove == true)
		{

			camera.ProcessKeyboard(FORWARD, deltaTime);

			glm::vec3 movement = glm::vec3(camera.Front.x, 0, camera.Front.z) * vel;

			characterPosition += movement;
			characterRotationDegrees = 180.0f;
			characterRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

			for (int i = 0; i < c.meshes.size(); i++)
			{
				c.meshes[i].br.min += movement;
				c.meshes[i].br.max += movement;
			}
		}
	}
	// Move DOWN
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		keyPressedNow = "S";

		if (canMove == true)
		{

			camera.ProcessKeyboard(BACKWARD, deltaTime);

			glm::vec3 movement = glm::vec3(camera.Front.x, 0, camera.Front.z) * vel;

			characterPosition -= movement;
			characterRotationDegrees = 0.0f;
			characterRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

			for (int i = 0; i < c.meshes.size(); i++)
			{
				c.meshes[i].br.min -= glm::vec3(camera.Front.x, 0, camera.Front.z) * vel;
				c.meshes[i].br.max -= glm::vec3(camera.Front.x, 0, camera.Front.z) * vel;
			}
		}
	}
	// Move LEFT
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		keyPressedNow = "A";

		if (canMove == true)
		{

			camera.ProcessKeyboard(LEFT, deltaTime);

			glm::vec3 movement = camera.Right * vel;

			characterPosition -= movement;
			characterRotationDegrees = -90.0f;
			characterRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

			for (int i = 0; i < c.meshes.size(); i++)
			{
				c.meshes[i].br.min -= camera.Right * vel;
				c.meshes[i].br.max -= camera.Right * vel;
			}
		}
	}
	// Move RIGHT
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		keyPressedNow = "D";

		if (canMove == true)
		{

			camera.ProcessKeyboard(RIGHT, deltaTime);
			glm::vec3 movement = camera.Right * vel;

			characterPosition += movement;
			characterRotationDegrees = 90.0f;
			characterRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

			for (int i = 0; i < c.meshes.size(); i++)
			{
				c.meshes[i].br.min += movement;
				c.meshes[i].br.max += movement;
			}
		}
	}
	// Use POKEMON 1
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && pokeballThrown == false)
	{
		if (drawPokemonArray[0] == false)
		{
			// Update the temp pokeball rigidbody pos
			pokeballTempRB.pos = glm::vec3(characterPosition.x, characterPosition.y + 0.3f, characterPosition.z);
			// Apply Impulse to the temp pokeball rigidbody
			pokeballTempRB.applyImpulse(normalize(glm::vec3(mousePosX - SCR_WIDTH / 2.0f, 100.0f, mousePosY - SCR_HEIGHT / 2.0f)), 200.0f, deltaTime);
			
			// Boolean Locks
			// -------------
			// Update the respective keypress bool
			key1_wasPressed = true;
			// Pokeball was thrown, so update the boolean lock
			pokeballThrown = true;

			// Debug output
			cout << "Key 1 was pressed. Draw Pokemon Number 1." << endl;
		}
	}
	// Use POKEMON 2
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && pokeballThrown == false)
	{
		if (drawPokemonArray[1] == false)
		{
			// Update the temp pokeball rigidbody pos
			pokeballTempRB.pos = glm::vec3(characterPosition.x, characterPosition.y + 0.3f, characterPosition.z);
			// Apply Impulse to the temp pokeball rigidbody
			pokeballTempRB.applyImpulse(normalize(glm::vec3(mousePosX - SCR_WIDTH / 2.0f, 100.0f, mousePosY - SCR_HEIGHT / 2.0f)), 200.0f, deltaTime);

			// Boolean Locks
			// -------------
			// Update the respective keypress bool
			key2_wasPressed = true;
			// Pokeball was thrown, so update the boolean lock
			pokeballThrown = true;

			// Debug output
			cout << "Key 2 was pressed. Draw Pokemon Number 2." << endl;
		}
	}
	// Use POKEMON 3
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && pokeballThrown == false)
	{
		if (drawPokemonArray[2] == false)
		{
			// Update the temp pokeball rigidbody pos
			pokeballTempRB.pos = glm::vec3(characterPosition.x, characterPosition.y + 0.3f, characterPosition.z);
			// Apply Impulse to the temp pokeball rigidbody
			pokeballTempRB.applyImpulse(normalize(glm::vec3(mousePosX - SCR_WIDTH / 2.0f, 100.0f, mousePosY - SCR_HEIGHT / 2.0f)), 200.0f, deltaTime);

			// Boolean Locks
			// -------------
			// Update the respective keypress bool
			key3_wasPressed = true;
			// Pokeball was thrown, so update the boolean lock
			pokeballThrown = true;

			// Debug output
			cout << "Key 3 was pressed. Draw Pokemon Number 3." << endl;
		}
	}
	// Catch POKEMON (currently not working, just throwing the ball straight with much force)
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && pokeballThrown == false)
	{
		if (canCatch == true)
		{
			// Update the temp pokeball rigidbody pos
			pokeballTempRB.pos = glm::vec3(characterPosition.x, characterPosition.y + 0.3f, characterPosition.z);
			// Apply Impulse to the temp pokeball rigidbody
			//pokeballTempRB.applyImpulse(normalize(glm::vec3(mousePosX - SCR_WIDTH / 2.0f, 0.0f, mousePosY - SCR_HEIGHT / 2.0f)), 800.0f, deltaTime);
			pokeballTempRB.applyImpulse(
				normalize(glm::vec3(pokemonToCatchBr.calculateCenter().x - c.meshes[0].br.calculateCenter().x,
									0.0f,
									pokemonToCatchBr.calculateCenter().z - c.meshes[0].br.calculateCenter().z)
						 ),
				800.0f,
				deltaTime);

			// Boolean Locks
			// -------------
			// Pokeball was thrown, so update the boolean lock
			pokeballThrown = true;

			// Reset canCatch lock
			canCatch = false;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Note: that width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// mouse raycast callback
glm::vec3 mouse_raycast_callback(int mouse_x, int mouse_y)
{
	float x = (2.0f * mouse_x) / SCR_WIDTH - 1.0f;
	float y = 1.0f - (2.0f * mouse_y) / SCR_HEIGHT;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.0, 1.0);

	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	glm::vec3 ray_wor((glm::inverse(view) * ray_eye).x, (glm::inverse(view) * ray_eye).y, (glm::inverse(view) * ray_eye).z);	 
	
	return ray_wor;
}

bool mouseIntersectsAABB(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 boxMin, glm::vec3 boxMax) {
	glm::vec3 tMin = (boxMin - rayOrigin) / rayDir;
	glm::vec3 tMax = (boxMax - rayOrigin) / rayDir;

	glm::vec3 t1 = min(tMin, tMax);
	glm::vec3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	if (tNear > tFar)
	{
		//cout << "Ray not intersecting AABB" << endl;
		return false;
	}

	//cout << "CONGRATULATIONS! YOU HAVE AN INTERSECTION!" << endl;
	return true;
};