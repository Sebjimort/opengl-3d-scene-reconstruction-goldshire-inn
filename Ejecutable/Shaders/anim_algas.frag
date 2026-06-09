#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// La clase Model que estás usando envía las texturas con este nombre por defecto
uniform sampler2D texture_diffuse1; 

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // Descartamos la transparencia dura de la textura de las algas
    if(texColor.a < 0.1)
        discard;

    // Multiplicamos por un vec4 grisáceo para simular un poco de sombra ambiental 
    // y que no se vean brillantes como un foco en medio del agua
    FragColor = texColor * vec4(0.8, 0.8, 0.8, 1.0);
}