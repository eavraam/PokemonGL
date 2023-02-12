#version 330 core

in vec4 particleColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(particleColor.r, particleColor.g, particleColor.b, 0.0f);
}