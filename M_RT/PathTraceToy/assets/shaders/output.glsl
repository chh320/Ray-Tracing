#version 460 core
in vec2 TexCoord;

out vec4 FragColor;
uniform sampler2D accumTex;

void main()
{	
	FragColor = vec4(texture(accumTex, TexCoord).rgb, 1.0);
}