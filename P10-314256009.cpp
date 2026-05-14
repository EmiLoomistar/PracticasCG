/*
Practica 10: Animacion por Keyframes
Animacion del objeto (gun) y sus engranes (gun_gear) usando keyframes.
Guarda/carga los frames a/desde keyframes.txt
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

//para iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture pisoTexture;

// Modelos: el objeto (gun) y los engranes (gun_gear)
Model gun_model;
Model gun_gear_model;

Skybox skybox;

Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

void inputKeyframes(bool* keys);
float reproduciranimacion = 0, habilitaranimacion = 0, guardoFrame = 0, reinicioFrame = 0;
float ciclo = 0, ciclo2 = 0;

void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};
	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};
	Mesh* piso = new Mesh();
	piso->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(piso);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

///////////////////////////////KEYFRAMES/////////////////////
// Variables que controla la animacion
float posObj_x = 0.0f, posObj_y = 0.0f, posObj_z = 0.0f;
float giroObj = 0.0f;
float rotEngrane = 0.0f;

#define MAX_FRAMES 100
int i_max_steps = 90;
int i_curr_steps = 0;

typedef struct _frame
{
	float posObj_x;
	float posObj_y;
	float posObj_z;
	float giroObj;
	float rotEngrane;
	float posObj_xInc;
	float posObj_yInc;
	float posObj_zInc;
	float giroObjInc;
	float rotEngraneInc;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	KeyFrame[FrameIndex].posObj_x = posObj_x;
	KeyFrame[FrameIndex].posObj_y = posObj_y;
	KeyFrame[FrameIndex].posObj_z = posObj_z;
	KeyFrame[FrameIndex].giroObj = giroObj;
	KeyFrame[FrameIndex].rotEngrane = rotEngrane;
	FrameIndex++;
}

void resetElements(void)
{
	posObj_x = KeyFrame[0].posObj_x;
	posObj_y = KeyFrame[0].posObj_y;
	posObj_z = KeyFrame[0].posObj_z;
	giroObj = KeyFrame[0].giroObj;
	rotEngrane = KeyFrame[0].rotEngrane;
}

void saveFramesToFile(void)
{
	FILE* f;
	errno_t err = fopen_s(&f, "keyframes.txt", "w");
	if (err != 0 || f == NULL) {
		printf("Error al abrir keyframes.txt para escribir\n");
		return;
	}
	fprintf(f, "%d\n", FrameIndex);
	for (int i = 0; i < FrameIndex; i++)
		fprintf(f, "%f %f %f %f %f\n",
			KeyFrame[i].posObj_x, KeyFrame[i].posObj_y, KeyFrame[i].posObj_z,
			KeyFrame[i].giroObj, KeyFrame[i].rotEngrane);
	fclose(f);
	printf("Frames guardados (%d) en keyframes.txt\n", FrameIndex);
}

void loadFramesFromFile(void)
{
	FILE* f;
	errno_t err = fopen_s(&f, "keyframes.txt", "r");
	if (err != 0 || f == NULL) {
		printf("No se encontro keyframes.txt\n");
		return;
	}
	fscanf_s(f, "%d", &FrameIndex);
	for (int i = 0; i < FrameIndex; i++)
		fscanf_s(f, "%f %f %f %f %f",
			&KeyFrame[i].posObj_x, &KeyFrame[i].posObj_y, &KeyFrame[i].posObj_z,
			&KeyFrame[i].giroObj, &KeyFrame[i].rotEngrane);
	fclose(f);
	printf("Frames cargados (%d) desde keyframes.txt\n", FrameIndex);
}

void interpolation(void)
{
	KeyFrame[playIndex].posObj_xInc = (KeyFrame[playIndex + 1].posObj_x - KeyFrame[playIndex].posObj_x) / i_max_steps;
	KeyFrame[playIndex].posObj_yInc = (KeyFrame[playIndex + 1].posObj_y - KeyFrame[playIndex].posObj_y) / i_max_steps;
	KeyFrame[playIndex].posObj_zInc = (KeyFrame[playIndex + 1].posObj_z - KeyFrame[playIndex].posObj_z) / i_max_steps;
	KeyFrame[playIndex].giroObjInc = (KeyFrame[playIndex + 1].giroObj - KeyFrame[playIndex].giroObj) / i_max_steps;
	KeyFrame[playIndex].rotEngraneInc = (KeyFrame[playIndex + 1].rotEngrane - KeyFrame[playIndex].rotEngrane) / i_max_steps;
}

void animate(void)
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps)
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)
			{
				printf("Termina animacion (Frame index= %d)\n", FrameIndex);
				playIndex = 0;
				play = false;
			}
			else
			{
				i_curr_steps = 0;
				interpolation();
			}
		}
		else
		{
			posObj_x   += KeyFrame[playIndex].posObj_xInc;
			posObj_y   += KeyFrame[playIndex].posObj_yInc;
			posObj_z   += KeyFrame[playIndex].posObj_zInc;
			giroObj    += KeyFrame[playIndex].giroObjInc;
			rotEngrane += KeyFrame[playIndex].rotEngraneInc;
			i_curr_steps++;
		}
	}
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 1.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.5f, 0.5f);

	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	gun_model = Model();
	gun_model.LoadModel("Models/gun.obj");
	gun_gear_model = Model();
	gun_gear_model.LoadModel("Models/gun_gear.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.6f,
		0.0f, -1.0f, -1.0f);

	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
		0.1f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset(0.0f, 0.0f);

	// Keyframes predefinidos: el gun se mueve adelante/atras y gira mientras los engranes rotan
	KeyFrame[0]  = { 0.0f, 0.0f,  0.0f,   0.0f,    0.0f };
	KeyFrame[1]  = { 0.0f, 0.0f, -1.0f,   0.0f,  360.0f };
	KeyFrame[2]  = { 0.0f, 0.5f, -2.0f,   0.0f,  720.0f };
	KeyFrame[3]  = { 0.0f, 0.5f, -2.0f,  90.0f,  900.0f };
	KeyFrame[4]  = { 1.0f, 0.5f, -2.0f, 180.0f, 1080.0f };
	KeyFrame[5]  = { 1.0f, 0.0f, -1.0f, 180.0f, 1440.0f };
	KeyFrame[6]  = { 1.0f, 0.0f,  0.0f, 180.0f, 1800.0f };
	KeyFrame[7]  = { 1.0f, 0.0f,  0.0f, 360.0f, 2160.0f };
	KeyFrame[8]  = { 0.0f, 0.0f,  0.0f, 360.0f, 2520.0f };
	FrameIndex = 9;

	printf("\n==========================================\n");
	printf("Practica 10: Animacion por Keyframes\n");
	printf("==========================================\n");
	printf("Teclas:\n");
	printf("  ESPACIO : reproducir animacion\n");
	printf("  0       : habilitar reproduccion de nuevo\n");
	printf("  1       : habilitar movimiento (presionar antes de mover)\n");
	printf("  2/3     : mover objeto X+ / X-\n");
	printf("  4/5     : mover objeto Y+ / Y-\n");
	printf("  8/9     : mover objeto Z+ / Z-\n");
	printf("  6/7     : girar objeto +90 / -90 grados\n");
	printf("  Z/X     : rotar engranes -/+ (keyframe)\n");
	printf("  L       : guardar frame  |  P: habilitar guardar otro\n");
	printf("  O       : escribir frames a keyframes.txt\n");
	printf("  R       : leer frames de keyframes.txt\n");
	printf("==========================================\n\n");

	lastTime = glfwGetTime();
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime * 5);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		inputKeyframes(mainWindow.getsKeys());
		animate();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);

		// Piso
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();

		// gun (objeto que se mueve)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(posObj_x, posObj_y, posObj_z));
		model = glm::rotate(model, giroObj * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		gun_model.RenderModel();

		// engrane izquierdo (animado por keyframe + ajuste manual con Z/X)
		model = modelaux;
		model = glm::translate(model, glm::vec3(-0.3f, 0.25f, 0.6f));
		model = glm::rotate(model, (rotEngrane + mainWindow.get_engranemovadicionalX()) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		gun_gear_model.RenderModel();

		// engrane derecho (rota en sentido contrario)
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.3f, 0.25f, 0.6f));
		model = glm::rotate(model, (rotEngrane + mainWindow.get_engranemovadicionalX()) * toRadians, glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		gun_gear_model.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}

void inputKeyframes(bool* keys)
{
	// ESPACIO: reproducir
	if (keys[GLFW_KEY_SPACE])
	{
		if (reproduciranimacion < 1)
		{
			if (!play && FrameIndex > 1)
			{
				resetElements();
				playIndex = 0;
				i_curr_steps = 0;
				interpolation();
				play = true;
				reproduciranimacion++;
				habilitaranimacion = 0;
				printf("Reproduciendo animacion... (presiona 0 para habilitar de nuevo)\n");
			}
		}
	}
	// 0: habilitar reproducir de nuevo
	if (keys[GLFW_KEY_0])
	{
		if (habilitaranimacion < 1)
		{
			reproduciranimacion = 0;
			habilitaranimacion++;
			printf("Listo: presiona ESPACIO para reproducir\n");
		}
	}

	// L: guardar frame
	if (keys[GLFW_KEY_L])
	{
		if (guardoFrame < 1)
		{
			saveFrame();
			printf("Frame %d guardado | x=%.2f y=%.2f z=%.2f giro=%.1f engrane=%.1f | (P para guardar otro)\n",
				FrameIndex - 1, posObj_x, posObj_y, posObj_z, giroObj, rotEngrane);
			guardoFrame++;
			reinicioFrame = 0;
		}
	}
	if (keys[GLFW_KEY_P])
	{
		if (reinicioFrame < 1)
		{
			guardoFrame = 0;
			reinicioFrame++;
			printf("Listo: presiona L para guardar otro frame\n");
		}
	}

	// O: escribir a archivo
	if (keys[GLFW_KEY_O])
	{
		if (guardoFrame < 1)
		{
			saveFramesToFile();
			guardoFrame++;
			reinicioFrame = 0;
		}
	}
	// R: leer de archivo
	if (keys[GLFW_KEY_R])
	{
		if (reinicioFrame < 1)
		{
			loadFramesFromFile();
			reinicioFrame++;
			guardoFrame = 0;
		}
	}

	// 1: habilitar nuevo movimiento
	if (keys[GLFW_KEY_1])
	{
		if (ciclo2 < 1)
		{
			ciclo = 0;
			ciclo2++;
			printf("Mueve: 2/3(X) 4/5(Y) 8/9(Z) 6/7(giro) Z/X(engrane)\n");
		}
	}
	if (keys[GLFW_KEY_2]) { if (ciclo < 1) { posObj_x += 0.5f; printf("posObj_x=%.2f\n", posObj_x); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_3]) { if (ciclo < 1) { posObj_x -= 0.5f; printf("posObj_x=%.2f\n", posObj_x); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_4]) { if (ciclo < 1) { posObj_y += 0.5f; printf("posObj_y=%.2f\n", posObj_y); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_5]) { if (ciclo < 1) { posObj_y -= 0.5f; printf("posObj_y=%.2f\n", posObj_y); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_8]) { if (ciclo < 1) { posObj_z += 0.5f; printf("posObj_z=%.2f\n", posObj_z); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_9]) { if (ciclo < 1) { posObj_z -= 0.5f; printf("posObj_z=%.2f\n", posObj_z); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_6]) { if (ciclo < 1) { giroObj += 90.0f; printf("giroObj=%.1f\n", giroObj); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_7]) { if (ciclo < 1) { giroObj -= 90.0f; printf("giroObj=%.1f\n", giroObj); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_Z]) { if (ciclo < 1) { rotEngrane -= 30.0f; printf("rotEngrane=%.1f\n", rotEngrane); ciclo++; ciclo2 = 0; } }
	if (keys[GLFW_KEY_X]) { if (ciclo < 1) { rotEngrane += 30.0f; printf("rotEngrane=%.1f\n", rotEngrane); ciclo++; ciclo2 = 0; } }
}
