#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Parámetros del Sprite Sheet
uniform int numCols;
uniform int numRows;
uniform int currentFrame;

void main()
{
    // 1. BILLBOARDING CON ESCALA
    mat4 modelView = view * model;
    
    // Extraemos el tamaño real (escala) de cada columna del modelView antes de modificarlo
    float scaleX = length(vec3(modelView[0]));
    float scaleY = length(vec3(modelView[1]));
    float scaleZ = length(vec3(modelView[2]));
    
    // Anulamos la rotación (para que mire a la cámara) pero le aplicamos la escala original
    modelView[0][0] = scaleX; modelView[0][1] = 0.0;    modelView[0][2] = 0.0;
    modelView[1][0] = 0.0;    modelView[1][1] = scaleY; modelView[1][2] = 0.0;
    modelView[2][0] = 0.0;    modelView[2][1] = 0.0;    modelView[2][2] = scaleZ;

    gl_Position = projection * modelView * vec4(aPos, 1.0);

    // 2. ANIMACIÓN UV: Calculamos el tamaño de cada fotograma
    float frameWidth = 1.0 / float(numCols);
    float frameHeight = 1.0 / float(numRows);

    // Encontramos en qué columna y fila estamos actualmente
    int col = currentFrame % numCols;
    int row = currentFrame / numCols;

    // Desplazamos las coordenadas UV base
    float u = (aTexCoords.x * frameWidth) + (float(col) * frameWidth);

    float v = (aTexCoords.y * frameHeight) + (float(numRows - 1 - row) * frameHeight);

    TexCoords = vec2(u, v);
}