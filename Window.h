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
	GLfloat get_articulation() { return articulation; };
	GLfloat getmuevex() { return muevex; }
	GLfloat get_pos_z() { return pos_z; }
	GLfloat get_pos_x() { return pos_x; }
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
	bool avanzando = false;
	bool retrocediendo = false;
	bool mouseFirstMoved;
	bool is_pointlight_on = false;
	int counter;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};

