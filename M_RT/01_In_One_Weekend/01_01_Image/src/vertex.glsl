#version 460 core
layout(location = 0) in vec3 aPos;
out vec2 screenCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	screenCoord = (vec2(aPos.x, aPos.y) + 1.0) / 2.0;
	gl_Position = vec4(aPos, 1.0);
}