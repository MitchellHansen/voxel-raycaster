#pragma once
#include <string>
#include <util.hpp>
#include <cstring>

#define GLEW_STATIC
#include <GL/glew.h>

class GL_Testing
{
public:
	GL_Testing();
	~GL_Testing(){};


	enum Shader_Type {VERTEX, FRAGMENT};
	void compile_shader(std::string file_path, Shader_Type t);
	void create_program();
	void create_buffers();
	void transform();
	void rotate(double delta);
	void draw();

private:
	
	GLuint VBO; //raw points
	GLuint EBO; //link triangles
	GLuint VAO;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;

	GLfloat *matrix;

	double counter = 0;
};

