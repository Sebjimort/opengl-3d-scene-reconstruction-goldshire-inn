#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec2 OriginalUV; 

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float time; 

void main()
{
    // Billboarding esférico
    vec3 centerPos = vec3(model[3][0], model[3][1], model[3][2]);
    float scaleX = length(vec3(model[0]));
    float scaleY = length(vec3(model[1]));

    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 vertexPos = centerPos 
                     + cameraRight * aPos.x * scaleX 
                     + cameraUp * aPos.y * scaleY;

    gl_Position = projection * view * vec4(vertexPos, 1.0);

    // Mandamos los UV originales sin alterar para la máscara
    OriginalUV = aTexCoords;

    // Animación de textura
    float velocidadY = 1.2;
    float v = aTexCoords.y - (time * velocidadY);
    float u = aTexCoords.x + sin(time * 3.0 + aTexCoords.y * 5.0) * 0.05;

    TexCoords = vec2(u, v);
}