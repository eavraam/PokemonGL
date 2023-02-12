#version 330 core

// Layouts
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
//layout (location = 2) in mat4 aInstanceMatrix;

// Outputs
out vec4 particleColor;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    particleColor = aColor;

    // for instancing
    //gl_Position = projection * view * vec4(vec3(aInstanceMatrix * vec4(aPos, 1.0)), 1.0);
    //gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0); 
}