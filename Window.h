#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat get_is_pointlight_on() { return is_pointlight_on;  };
	GLfloat get_is_pointlight2_on() { return is_pointlight2_on; };
	GLfloat get_articulation() { return articulation; };
	GLfloat get_articulation_x() { return articulation_x; };
	GLfloat get_articulation_y() { return articulation_y; };
	GLfloat get_articulation_z() { return articulation_z; };
	GLfloat getmuevex() { return muevex; }
	GLfloat get_pos_z() { return pos_z; }
	GLfloat get_pos_x() { return pos_x; }
	GLfloat get_pez_posicion() { return pez_posicion; }
	bool get_avanzando() { return avanzando; };
	bool get_retrocediendo() { return retrocediendo; };
	int get_counter() { return counter; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	
	~Window();
private: 
	GLFWwindow *mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	GLfloat muevex;
	GLfloat pos_z;
	GLfloat pos_x;
	GLfloat articulation;
	GLfloat articulation_x = 0.0f;
	GLfloat articulation_y = 0.0f;
	GLfloat articulation_z = 0.0f;
	GLfloat pez_posicion = 0.0f;
	bool avanzando = false;
	bool retrocediendo = false;
	bool mouseFirstMoved;
	bool is_pointlight_on = false;
	bool is_pointlight2_on = false;
	int counter;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};

