#include "GL_Testing.h"

GL_Testing::GL_Testing() {

	GLfloat tmp[] = {

		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, static_cast<float>(cos(1.0f)), static_cast<float>(sin(1.0f)), 0.0f,
		0.0f, static_cast<float>(-sin(1.0f)), static_cast<float>(cos(1.0f)), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f

	};

	matrix = new GLfloat[16];
	memcpy(matrix, tmp, sizeof(GLfloat) * 16);

	GLint err = glewInit();

	if (err) {
		std::cout << "error initializing glew" << std::endl;
	}

}

void GL_Testing::compile_shader(std::string file_path, Shader_Type t) {


	// Load in the source and cstring it
	const char* source;
	std::string tmp;

	tmp = read_file(file_path);
	source = tmp.c_str();

	GLint success;
	GLchar log[512];

	if (t == Shader_Type::VERTEX) {
	
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &source, NULL);
		glCompileShader(vertex_shader);
		
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
		
		if (!success) {
			glGetShaderInfoLog(vertex_shader, 512, NULL, log);
			std::cout << "Vertex shader failed compilation: " << log << std::endl;
		}

	} else if (t == Shader_Type::FRAGMENT) {

		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &source, NULL);
		glCompileShader(fragment_shader);

		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	
		if (!success) {
			glGetShaderInfoLog(fragment_shader, 512, NULL, log);
			std::cout << "Vertex shader failed compilation: " << log << std::endl;
		}
	}
}

void GL_Testing::create_program() {

	GLint success;
	GLchar log[512];

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, log);
		std::cout << "Failed to link shaders into program: " << log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void GL_Testing::create_buffers() {
	
	GLfloat vertices[] = {
		0.5f,  0.5f, 0.0f,  // Top Right
		0.5f, -0.5f, 0.0f,  // Bottom Right
		-0.5f, -0.5f, 0.0f,  // Bottom Left
		-0.5f,  0.5f, 0.0f   // Top Left 
	};
	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3  // First Triangle
		  // Second Triangle
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void GL_Testing::transform()
{
	GLuint transformLoc = glGetUniformLocation(shader_program, "transform");

	glUseProgram(shader_program);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, matrix);
}

void GL_Testing::rotate(double delta) {
	
	counter += delta;

	GLfloat tmp[] = {

		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, static_cast<float>(cos(counter)), static_cast<float>(sin(counter)), 0.0f,
		0.0f, static_cast<float>(-sin(counter)), static_cast<float>(cos(counter)), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f

	};

	memcpy(matrix, tmp, sizeof(GLfloat) * 16);

}

void GL_Testing::draw() {

	glUseProgram(shader_program);
	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
//
