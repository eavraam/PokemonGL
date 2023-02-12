#version 330 core

out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D terrain_diffuse;

void main()
{    
    FragColor = texture(terrain_diffuse, v_uv);
    //FragColor = vec4(1.0f);
}