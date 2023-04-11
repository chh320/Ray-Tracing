#version 330 core

in vec2 TexCoord;
out vec4 fragColor;

uniform sampler2D accumTex;

vec3 toneMapping(in vec3 c, float limit) {
    float luminance = 0.3*c.x + 0.6*c.y + 0.1*c.z;
    return c * 1.0 / (1.0 + luminance / limit);
}

void main() {
    vec3 color = texture2D(accumTex, TexCoord.xy).rgb;
    color = toneMapping(color, 1.5);
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0);
}