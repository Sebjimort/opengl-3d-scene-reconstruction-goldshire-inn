#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Recibimos el tiempo desde C++

void main()
{
    vec3 posModificada = aPos;

    // Factor de altura: max() asegura que la base (y=0) no se mueva, 
    // mientras que los vértices más altos se moverán con mayor amplitud.
    float factorAltura = max(posModificada.y, 0.0);
    float swayAmount = 0.08; // Controla que tanto se mueven

    // Ondulación principal en X y secundaria en Z
    posModificada.x += sin(time * 3.0 + posModificada.y * 3.0) * swayAmount * factorAltura;
    posModificada.z += cos(time * 2.5 + posModificada.y * 2.5) * (swayAmount * 0.5) * factorAltura;

    gl_Position = projection * view * model * vec4(posModificada, 1.0);
    TexCoords = aTexCoords;
}