#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec2 OriginalUV; // RECIBIMOS LOS UV ORIGINALES

uniform sampler2D fireTexture;

void main()
{
    vec4 texColor = texture(fireTexture, TexCoords);

    // 1. Calculamos la distancia desde el píxel actual al centro del Quad (0.5, 0.5)
    // Para que la llama sea un poco más alta que ancha, achatamos un poco el cálculo en Y
    vec2 centro = vec2(0.5, 0.5);
    vec2 uvModificado = vec2(OriginalUV.x, OriginalUV.y * 0.8); 
    float distancia = distance(uvModificado, vec2(centro.x, centro.y * 0.8));

    // 2. Usamos smoothstep para crear un degradado suave
    // Todo lo que esté a 0.5 de distancia será 0.0 (negro puro)
    // Todo lo que esté a 0.2 o menos será 1.0 (color puro)
    float mascara = 1.0 - smoothstep(0.2, 0.45, distancia);

    // 3. Multiplicamos el color del fuego por la máscara
    FragColor = vec4(texColor.rgb * mascara, 1.0);
}