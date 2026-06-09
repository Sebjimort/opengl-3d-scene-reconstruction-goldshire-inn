#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D fireTexture;

void main()
{
    vec4 texColor = texture(fireTexture, TexCoords);
    
    if(texColor.r < 0.05 && texColor.g < 0.05 && texColor.b < 0.05)
        discard;
        
    FragColor = texColor;
}