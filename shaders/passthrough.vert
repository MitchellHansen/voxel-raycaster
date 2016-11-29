#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 transform;
out vec4 vertexColor;

void main() {
	gl_Position = transform * vec4(position, 1.0f);
	
	//gl_Position = vec4(position, 1.0);
    vertexColor = vec4(0.5f, 0.0f, 0.0f, 1.0f); 

}