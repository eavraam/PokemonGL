#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Bounds.h"

#include <string>
#include <vector>
using namespace std;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh
{
public:
    // Bounding Region
    BoundingRegion br;
    
    // Mesh Data
    vector<Vertex>          vertices;
    vector<unsigned int>    indices;
    vector<Texture>         textures;
    unsigned int VAO;

    // Constructor
    Mesh
    (
        BoundingRegion br,
        vector<Vertex> vertices,
        vector<unsigned int> indices,
        vector<Texture> textures
    );

    // Public Functions
    void Draw(Shader& shader);
    

private:
    // Render Data
    unsigned int VBO, EBO;

    // Private Functions
    void setupMesh();
    
};


#endif 
