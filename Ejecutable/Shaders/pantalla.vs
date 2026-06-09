#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

// Recibimos las matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Multiplicamos la posición por las matrices para que OpenGL respete la escala y la traslación
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
    TexCoords = aTexCoords;
}