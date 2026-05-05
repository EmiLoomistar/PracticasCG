/*
Animación:
Sesión 1: Simple o básica:Por banderas y condicionales (más de 1 transformación geométrica se ve modificada)
Sesión 2: Compleja: Por medio de funciones y algoritmos. Textura Animada
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
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

//variables para animación
float movCoche;
float movOffset;
float rotllanta;
float rotllantaOffset;
float rotCarro;
float rotCarroOffset;
bool avanza = true;
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;



Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Ala2_M;
Model Helice_M;

Model carro_base;
Model carro_cofre;
Model carro_llanta;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat dt = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;


// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";



//cálculo del promedio de las normales para sombreado de Phong
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-4.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		4.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-4.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		4.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};


	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	Mesh* obj5 = new Mesh();
	obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6);
	meshList.push_back(obj5);

	Mesh* obj6 = new Mesh();
	obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);
	meshList.push_back(obj6); // todos los números

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7); // solo un número

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 0.5f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();


	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista.obj");
	Nave_M = Model();
	Nave_M.LoadModel("Models/nave_base.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Ala2_M = Model();
	Ala2_M.LoadModel("Models/ala2.obj");
	Helice_M = Model();
	Helice_M.LoadModel("Models/helice.obj");

	carro_base = Model();
	carro_base.LoadModel("Models/carro.obj");
	carro_cofre = Model();
	carro_cofre.LoadModel("Models/carro_cofre.obj");
	carro_llanta = Model();
	carro_llanta.LoadModel("Models/carro_llanta.obj");

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


	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	// faro izquierdo (spotlight)
	spotLights[1] = SpotLight(1.0f, 1.0f, 0.0f, // rgb
		10.0f, 10.0f,
		0.95f, 0.8f, 2.4f,
		0.0f, -0.10f, 1.0f,
		1.0f, 0.09f, 0.032f, 25.0f);
	spotLightCount++;

	// faro de la nave
	spotLights[2] = SpotLight(1.0f, 1.0f, 0.8f,  // rgb
		10.0f, 10.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.09f, 0.032f, 20.0f);
	spotLightCount++;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);


	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	float timer = 0.0f;

	float carro_x = 0.0f;
	float carro_y = 0.0f;
	float carro_z = 0.0f;

	float carro_degrees_y_objetivo = 0.0f;
	float carro_degrees_x_objetivo = 0.0f;

	float carro_degrees_x = 0.0f;
	float carro_degrees_y = 0.0f;

	float objeto_rotacion = 0.0f;

	float speed = 16.0f;

	bool abriendo = false;
	float ala_angulo = 0.0f;

	float nave_x = 0.0f;
	float nave_y = 0.0f;
	float nave_z = 0.0f;

	float nave_degrees_y_objetivo = 0.0f;
	float nave_degrees_x_objetivo = 0.0f;
	float nave_degrees_y = 0.0f;
	float nave_degrees_x = 0.0f;

	float nave_timer = 0.0f;
	bool nave_termino = false;  

	nave_x = 27.0f; 
	nave_y = 31.0f;
	nave_z = 183.0f;

	lastTime = glfwGetTime();
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		float prev_y = carro_y;
		float prev_z = carro_z;
		float prev_x = carro_x;
		// SEGMENTO 1
		if (timer < 30.0f) {
			carro_degrees_y_objetivo = 0.0f;
			carro_z += dt * speed;
		}
		// SEGMENTO 2: curva izquierda
		else if (timer < 55.0f) {
			carro_degrees_y_objetivo = 15.0f;   
			carro_x += dt * speed * 0.27f;
			carro_y += dt * speed * 0.03f;
			carro_z += dt * speed * 0.95f;
		}
		// SEGMENTO 3: giro fuerte derecha + subida
		else if (timer < 80.0f) {
			carro_degrees_y_objetivo = -35.0f;  
			carro_x -= dt * speed * 0.65f;
			carro_y += dt * speed * 0.22f;
			carro_z += dt * speed * 0.95f;
		}
		// SEGMENTO 4: subida casi recta, leve izquierda
		else if (timer < 120.0f) {
			carro_degrees_y_objetivo = 8.0f;    
			carro_x += dt * speed * 0.12f;
			carro_y += dt * speed * 0.46f;
			carro_z += dt * speed * 1.0f;
		}
		// SEGMENTO 5: curva derecha en lo alto
		else if (timer < 138.0f) {
			carro_degrees_y_objetivo = -18.0f;  
			carro_x -= dt * speed * 0.23f;
			carro_y += dt * speed * 0.19f;
			carro_z += dt * speed * 1.0f;
		}
		// SEGMENTO 6: curva derecha en lo alto
		else if (timer < 160.0f) {
			carro_degrees_y_objetivo = -18.0f;
			carro_x -= dt * speed * 0.23f;
			carro_y += dt * speed * 0.01f;
			carro_z += dt * speed * 1.0f;
		}
		// SEGMENTO 6: curva derecha en lo alto
		else if (timer < 170.0f) {
			carro_degrees_y_objetivo = -18.0f;
			carro_x -= dt * speed * 0.23f;
			carro_y -= dt * speed * 0.02f;
			carro_z += dt * speed * 1.0f;
		}
		// SEGMENTO 7: curva derecha final
		else if (timer < 181.0f) {
			carro_degrees_y_objetivo = -22.0f;  
			carro_x -= dt * speed * 0.32f;
			carro_y -= dt * speed * 0.18f;
			carro_z += dt * speed * 1.0f;
		}
		// SEGMENTO 8: curva derecha final
		else if (timer < 191.0f) {
			carro_degrees_y_objetivo = -22.0f;
			carro_x -= dt * speed * 0.32f;
			carro_y -= dt * speed * 0.18f;
			carro_z += dt * speed * 1.0f;
		}
		// reset
		else {
			timer = 0.0f;

			carro_x = 0.0f;
			carro_y = 0.0f;
			carro_z = 0.0f;

			carro_degrees_y_objetivo = 0.0f;
			carro_degrees_x_objetivo = 0.0f;
			carro_degrees_x = 0.0f;
			carro_degrees_y = 0.0f;

			objeto_rotacion = 0.0f;
		}

		// reset forzado
		if (mainWindow.get_reset()) {
			timer = 0.0f;

			carro_x = 0.0f;
			carro_y = 0.0f;
			carro_z = 0.0f;

			carro_degrees_y_objetivo = 0.0f;
			carro_degrees_x_objetivo = 0.0f;
			carro_degrees_x = 0.0f;
			carro_degrees_y = 0.0f;

			objeto_rotacion = 0.0f;

			nave_x = -37.0f;
			nave_y = 31.0f;
			nave_z = 183.0f;
			nave_degrees_y = 0.0f;
			nave_degrees_x = 0.0f;
			nave_timer = 0.0f;
			nave_termino = false;

			mainWindow.set_reset_false();
		}
	

		if (!nave_termino) {
			float prev_nave_x = nave_x;
			float prev_nave_y = nave_y;
			float prev_nave_z = nave_z;

			
			if (nave_timer < 31.0f) {
				nave_degrees_y_objetivo = -22.0f;  // +180 porque va al revés
				nave_x += dt * speed * 0.32f;
				nave_y += dt * speed * 0.08f;
				nave_z -= dt * speed * 1.05f;
			}
			// SEGMENTO 2 inverso (era SEG 5): cima en descenso
			else if (nave_timer < 71.0f) {
				nave_degrees_y_objetivo = -18.0f;
				nave_x += dt * speed * 0.23f;
				nave_y -= dt * speed * 0.13f;
				nave_z -= dt * speed * 1.0f;
			}
			// SEGMENTO 3 inverso (era SEG 4): bajada empinada
			else if (nave_timer < 106.0f) {
				nave_degrees_y_objetivo = +8.0f;
				nave_x -= dt * speed * 0.12f;
				nave_y -= dt * speed * 0.50f;
				nave_z -= dt * speed * 1.0f;
			}
			// SEGMENTO 4 inverso (era SEG 3): giro + bajada
			else if (nave_timer < 131.0f) {
				nave_degrees_y_objetivo = -35.0f;
				nave_x += dt * speed * 0.65f;
				nave_y -= dt * speed * 0.22f;
				nave_z -= dt * speed * 0.95f;
			}
			// SEGMENTO 5 inverso (era SEG 2): curva
			else if (nave_timer < 156.0f) {
				nave_degrees_y_objetivo = +15.0f;
				nave_x -= dt * speed * 0.27f;
				nave_y -= dt * speed * 0.09f;
				nave_z -= dt * speed * 0.95f;
			}
			// SEGMENTO 6 inverso (era SEG 1): aterrizaje en recta final al piso
			else if (nave_timer < 180.0f) {
				nave_degrees_y_objetivo = 0.0f;
				nave_z -= dt * speed * 1.0f;

			}
			else if (nave_timer < 240.0f) {
				// leve descenso final hasta tocar piso (el -1.5 base + nave_y)
				nave_y -= dt * speed * 0.05f;
			}
			// Aterrizó — se detiene
			else {
				nave_termino = true;
			}

			// Pitch automático para la nave
			float d_y = nave_y - prev_nave_y;
			float d_z = nave_z - prev_nave_z;
			float d_x = nave_x - prev_nave_x;
			float horiz_n = sqrt(d_z * d_z + d_x * d_x);
			if (horiz_n > 0.0001f) {
				nave_degrees_x_objetivo = -atan2(d_y, horiz_n) * 180.0f / 3.14159265f;
				// invertido porque va al revés
				nave_degrees_x_objetivo = -nave_degrees_x_objetivo;
			}

			nave_degrees_y += (nave_degrees_y_objetivo - nave_degrees_y) * dt * 2.0f;
			nave_degrees_x += (nave_degrees_x_objetivo - nave_degrees_x) * dt * 3.0f;

			nave_timer += dt * speed;
		}

		// Pitch automático a partir de la pendiente real del frame
		float delta_y = carro_y - prev_y;
		float delta_z = carro_z - prev_z;
		float delta_x = carro_x - prev_x;
		float horizontal = sqrt(delta_z * delta_z + delta_x * delta_x);
		if (horizontal > 0.0001f) {
			carro_degrees_x_objetivo = -atan2(delta_y, horizontal) * 180.0f / 3.14159265f;
		}

		// Suavizado de ambas rotaciones
		carro_degrees_y += (carro_degrees_y_objetivo - carro_degrees_y) * dt * 2.0f;
		carro_degrees_x += (carro_degrees_x_objetivo - carro_degrees_x) * dt * 3.0f;

		timer += dt * speed;
		objeto_rotacion += dt * speed * 100;

		if (abriendo == true) {
			if (ala_angulo < 45) {
				ala_angulo += dt * speed*40;
			}
			else abriendo = false;
		}
		else {
			if (ala_angulo >= 0.0) {
				ala_angulo -= dt * speed*40;
			}
			else abriendo = true;
		}

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), dt*50);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation(); // para la textura con movimiento

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		//Reinicializando variables cada ciclo de reloj
		model = glm::mat4(1.0);
		modelaux = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		//Pista
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.1f, 2.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Pista_M.RenderModel();

		// carro base
		model = glm::mat4(1.0);
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(carro_x, -1.5f+carro_y, carro_z));

		model = glm::rotate(model, carro_degrees_y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, carro_degrees_x * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		carro_base.RenderModel();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// carro luz
		glm::mat4 faroMatrix = model;
		faroMatrix = glm::translate(faroMatrix, glm::vec3(0.95f, 0.7f, 2.2f));
		spotLights[1].SetFlash(faroMatrix[3], glm::vec3(faroMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
		// carro cofre
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 1.0f));
		model = glm::rotate(model, glm::radians(mainWindow.get_articulation()), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		carro_cofre.RenderModel();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// carro llanta
		model = modelaux;
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.85f));
		model = glm::rotate(model, objeto_rotacion * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		carro_llanta.RenderModel();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// carro llanta
		model = modelaux;
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 1.85f));
		model = glm::rotate(model, objeto_rotacion * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		carro_llanta.RenderModel();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// carro llanta
		model = modelaux;
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -1.45f));
		model = glm::rotate(model, objeto_rotacion * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		carro_llanta.RenderModel();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// carro llanta
		model = modelaux;
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.45f));
		model = glm::rotate(model, objeto_rotacion * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		carro_llanta.RenderModel();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// NAVE 
		model = glm::mat4(1.0);
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(nave_x, -1.5f + nave_y, nave_z));
		model = glm::rotate(model, nave_degrees_y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -nave_degrees_x * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		modelaux = model;  // guardamos la matriz PADRE

		// Base de la nave
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave_M.RenderModel();

		// Faro de la nave (apunta al frente de la nave)
		glm::mat4 faroNaveMatrix = model;
		faroNaveMatrix = glm::translate(faroNaveMatrix, glm::vec3(0.0f, -0.2f, 0.0f));
		spotLights[2].SetFlash(faroNaveMatrix[3], glm::vec3(faroNaveMatrix* glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)));

		// Ala 1 (hereda transformación padre + aleteo propio)
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.23f));
		model = glm::rotate(model, ala_angulo * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala_M.RenderModel();

		// Ala 2 (lado opuesto, aletea contrario)
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.23f));
		model = glm::rotate(model, -ala_angulo * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala2_M.RenderModel();

		// Hélice 1 (giro propio)
		model = modelaux;
		model = glm::translate(model, glm::vec3(-0.36f, -0.29f, 0.35f));
		model = glm::rotate(model, objeto_rotacion * toRadians * 100, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Helice_M.RenderModel();

		// Hélice 2
		model = modelaux;
		model = glm::translate(model, glm::vec3(-0.36f, -0.29f, -0.35f));
		model = glm::rotate(model, objeto_rotacion * toRadians * 100, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Helice_M.RenderModel();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
