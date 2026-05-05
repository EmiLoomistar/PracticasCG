/*
Animación:
Sesión 1:
Simple o básica: Por banderas y condicionales (más de 1 transformación geométrica se ve modificada
Sesión 2
Compleja: Por medio de funciones y algoritmos.
Textura Animada

Práctica 9 - Animación Compleja y Textura Animada
Emiliano Garcia Olvera - 314256009

Escena: Aeolípile (Eolípila) que calienta agua, genera humo en 3 estados,
el humo activa un brazo palanca que lanza una esfera metálica con física
de tiro parabólico y rebote. La esfera entra en un hueco y reaparece en el brazo.
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
bool avanza;
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;

// =============================================
// VARIABLES PARA LA ANIMACIÓN COMPLEJA P09
// =============================================
// Estados de la maquina:
// FUEGO: 0 = apagado, 1 = encendiendo, 2 = ardiendo fuerte, 3 = apagandose
// HUMO:  estado 1 = no sale, estado 2 = saliendo y creciendo, estado 3 = decreciendo hasta no salir
// BRAZO: 0 = reposo, 1 = cargando (humo lleno), 2 = lanzando, 3 = regresando
// ESFERA: 0 = en brazo, 1 = volando (parabolico), 2 = en hueco, 3 = reapareciendo

float ciclo_timer = 0.0f;       // timer maestro del ciclo
int   estado_fuego = 1;        // arranca encendiendose
int   estado_humo = 1;        // arranca sin salir
int   estado_brazo = 0;        // arranca en reposo
int   estado_esfera = 0;        // arranca en el brazo

// Humo: cantidad de particulas activas y escala global
float humo_intensidad = 0.0f;   // 0 = nada, 1 = maximo
float humo_offset_v = 0.0f;   // scroll de la textura del humo (textura animada)
float humo_offset_u = 0.0f;

// Fuego
float fuego_intensidad = 0.0f;  // controla escala del fuego
float fuego_timer = 0.0f;       // para rotacion y oscilacion

// Brazo (palanca)
float brazo_angulo = -45.0f;    // angulo de reposo (recargado hacia atras)
float brazo_angulo_objetivo = -45.0f;
bool  esfera_lanzada = false;

// Esfera (proyectil)
float esfera_x = 0.0f, esfera_y = 0.0f, esfera_z = 0.0f;
float esfera_vx = 0.0f, esfera_vy = 0.0f, esfera_vz = 0.0f;
int   rebotes = 0;
const int MAX_REBOTES = 3;
const float GRAVEDAD = 9.8f;
const float COEF_REBOTE = 0.6f;

// Posicion del extremo del brazo (punto de lanzamiento) y del hueco
const glm::vec3 POS_AEOLIPILE = glm::vec3(0.0f, -0.5f, 1.5f);
const glm::vec3 POS_BRAZO_BASE = glm::vec3(1.8f, 4.5f, 1.5f); // brazo a la altura de la esfera del aeolipile, a un costado
const glm::vec3 POS_HUECO = glm::vec3(-12.0f, 0.0f, 1.5f);
// =============================================

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
Texture fuego_texture;
Texture humo_texture;
Texture metal_texture;       // textura de la esfera (metalica/canica)
Texture brazo_texture;       // textura del brazo
Texture hueco_texture;       // textura del hueco en el piso



Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Aeolipile_base_M;
Model Aeolipile_M;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
Sphere esfera_proyectil = Sphere(0.4f, 20, 20);  // esfera para el proyectil (mas grande para verse)
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
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
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

	unsigned int numeroIndices2[] = {
		0, 1, 2,
		0, 2, 3,
	};

	GLfloat numeroVertices2[] = {
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,

	};

	// =================================================
	// Mesh para el brazo/palanca (paralelepipedo simple)
	// =================================================
	unsigned int brazoIndices[] = {
		// frente
		0, 1, 2,  0, 2, 3,
		// atras
		4, 6, 5,  4, 7, 6,
		// arriba
		8, 9, 10, 8, 10, 11,
		// abajo
		12, 14, 13, 12, 15, 14,
		// derecha
		16, 17, 18, 16, 18, 19,
		// izquierda
		20, 22, 21, 20, 23, 22
	};

	GLfloat brazoVertices[] = {
		// frente   (z+)
		-0.2f, -0.2f, 0.15f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		 2.5f, -0.2f, 0.15f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		 2.5f,  0.2f, 0.15f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		-0.2f,  0.2f, 0.15f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		// atras    (z-)
		-0.2f, -0.2f,-0.15f,   0.0f, 0.0f,   0.0f, 0.0f,-1.0f,
		 2.5f, -0.2f,-0.15f,   1.0f, 0.0f,   0.0f, 0.0f,-1.0f,
		 2.5f,  0.2f,-0.15f,   1.0f, 1.0f,   0.0f, 0.0f,-1.0f,
		-0.2f,  0.2f,-0.15f,   0.0f, 1.0f,   0.0f, 0.0f,-1.0f,
		// arriba   (y+)
		-0.2f,  0.2f, 0.15f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 2.5f,  0.2f, 0.15f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 2.5f,  0.2f,-0.15f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		-0.2f,  0.2f,-0.15f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		// abajo    (y-)
		-0.2f, -0.2f, 0.15f,   0.0f, 0.0f,   0.0f,-1.0f, 0.0f,
		 2.5f, -0.2f, 0.15f,   1.0f, 0.0f,   0.0f,-1.0f, 0.0f,
		 2.5f, -0.2f,-0.15f,   1.0f, 1.0f,   0.0f,-1.0f, 0.0f,
		-0.2f, -0.2f,-0.15f,   0.0f, 1.0f,   0.0f,-1.0f, 0.0f,
		// derecha  (x+)
		 2.5f, -0.2f, 0.15f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		 2.5f, -0.2f,-0.15f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		 2.5f,  0.2f,-0.15f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		 2.5f,  0.2f, 0.15f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		 // izquierda(x-)
		 -0.2f, -0.2f, 0.15f,   0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
		 -0.2f, -0.2f,-0.15f,   1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
		 -0.2f,  0.2f,-0.15f,   1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		 -0.2f,  0.2f, 0.15f,   0.0f, 1.0f,  -1.0f, 0.0f, 0.0f
	};

	// Mesh para el hueco en el piso (un quad)
	unsigned int huecoIndices[] = {
		0, 2, 1,
		0, 3, 2
	};
	GLfloat huecoVertices[] = {
		-0.4f, 0.0f, -0.4f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 0.4f, 0.0f, -0.4f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 0.4f, 0.0f,  0.4f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		-0.4f, 0.0f,  0.4f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f
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
	meshList.push_back(obj6);

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7);

	Mesh* obj8 = new Mesh();
	obj8->CreateMesh(numeroVertices2, numeroIndices2, 32, 6);
	meshList.push_back(obj8);

	// obj9 = brazo (palanca)
	Mesh* obj9 = new Mesh();
	obj9->CreateMesh(brazoVertices, brazoIndices, 192, 36);
	meshList.push_back(obj9);

	// obj10 = hueco en el piso
	Mesh* obj10 = new Mesh();
	obj10->CreateMesh(huecoVertices, huecoIndices, 32, 6);
	meshList.push_back(obj10);
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


// =====================================================================
// Funciones auxiliares para la animacion compleja
// =====================================================================

// Inicializa la esfera en el extremo del brazo
void colocarEsferaEnBrazo() {
	// Posicion aproximada al extremo del brazo en reposo (-45 grados)
	// El extremo local del brazo es (2.5, 0). Con scale -1 en X, queda en (-2.5, 0).
	float ang_reposo = -45.0f * toRadians;
	esfera_x = POS_BRAZO_BASE.x - 2.5f * cos(ang_reposo);
	esfera_y = POS_BRAZO_BASE.y + 2.5f * sin(ang_reposo) + 0.4f;
	esfera_z = POS_BRAZO_BASE.z;
	esfera_vx = 0.0f;
	esfera_vy = 0.0f;
	esfera_vz = 0.0f;
	rebotes = 0;
}

// Algoritmo del ciclo de la maquina. Avanza todos los estados.
void actualizarCiclo(float dt) {
	ciclo_timer += dt;

	// ==========================================
	// FUEGO: ciclo simple (siempre encendido en este escenario)
	// ==========================================
	fuego_timer += dt;
	if (estado_fuego == 1) { // encendiendo
		fuego_intensidad += dt * 0.5f;
		if (fuego_intensidad >= 1.0f) {
			fuego_intensidad = 1.0f;
			estado_fuego = 2; // ardiendo
		}
	}
	else if (estado_fuego == 2) {
		fuego_intensidad = 1.0f;
	}

	// ==========================================
	// HUMO: 3 estados
	// Estado 1: no sale (humo_intensidad = 0)
	// Estado 2: sale y crece (calentamiento del Aeolipile)
	// Estado 3: decrece hasta no salir
	// ==========================================
	if (estado_humo == 1) {
		// Esperando que se caliente el aeolipile
		humo_intensidad = 0.0f;
		if (ciclo_timer > 1.5f) {  // despues de 1.5s comienza a salir humo
			estado_humo = 2;
		}
	}
	else if (estado_humo == 2) {
		// Sale y crece
		humo_intensidad += dt * 0.4f;
		if (humo_intensidad >= 1.0f) {
			humo_intensidad = 1.0f;
			// Cuando el humo esta al maximo, activa el brazo
			if (estado_brazo == 0) {
				estado_brazo = 1; // pasa a "cargando" -> pronto lanzara
			}
		}
	}
	else if (estado_humo == 3) {
		// Decrece hasta apagarse
		humo_intensidad -= dt * 0.5f;
		if (humo_intensidad <= 0.0f) {
			humo_intensidad = 0.0f;
			estado_humo = 1;
			ciclo_timer = 0.0f; // reiniciar ciclo
		}
	}
	// scroll de la textura (textura animada del humo)
	humo_offset_v += dt * 0;//0.4f;
	humo_offset_u += dt * 0;// 0.1f;

	// ==========================================
	// BRAZO: animacion del lanzamiento por funciones
	// ==========================================
	if (estado_brazo == 0) {
		// reposo, esperando humo
		brazo_angulo_objetivo = -45.0f;
	}
	else if (estado_brazo == 1) {
		// cargando: aun mas atras, "tensandose"
		brazo_angulo_objetivo = -65.0f;
		// Cuando ya se cargo (angulo cerca del objetivo), pasa a lanzar
		if (fabs(brazo_angulo - brazo_angulo_objetivo) < 2.0f) {
			estado_brazo = 2; // lanzar!
		}
	}
	else if (estado_brazo == 2) {
		// LANZAR: sube rapido a +60 grados
		brazo_angulo_objetivo = 60.0f;
		// Justo cuando el brazo esta cerca de la vertical, soltar la esfera
		if (brazo_angulo > 30.0f && !esfera_lanzada) {
			esfera_lanzada = true;
			estado_esfera = 1; // volando

			// Posicion inicial de la esfera = extremo del brazo (largo 2.5, con scale -1 en X)
			float ang = brazo_angulo * toRadians;
			float largo_brazo = 2.5f;
			esfera_x = POS_BRAZO_BASE.x - largo_brazo * cos(ang);
			esfera_y = POS_BRAZO_BASE.y + largo_brazo * sin(ang) + 0.4f;
			esfera_z = POS_BRAZO_BASE.z;

			// Estrategia: 2 rebotes naturales + 3er bote cae en el hueco.
			// El primer impacto ocurre a ~1/3 de la distancia al hueco,
			// el segundo a ~2/3, y el tercero coincide con el hueco.
			float piso_y = -1.6f;
			glm::vec3 primer_impacto = glm::vec3(
				esfera_x + (POS_HUECO.x - esfera_x) * 0.33f,
				piso_y,
				esfera_z + (POS_HUECO.z - esfera_z) * 0.33f
			);

			float dx1 = primer_impacto.x - esfera_x;
			float dz1 = primer_impacto.z - esfera_z;
			float dy1 = primer_impacto.y - esfera_y;

			float t1 = 1.1f;
			esfera_vx = dx1 / t1;
			esfera_vz = dz1 / t1;
			esfera_vy = (dy1 + 0.5f * GRAVEDAD * t1 * t1) / t1;
		}
		if (brazo_angulo > 55.0f) {
			estado_brazo = 3; // regresar
		}
	}
	else if (estado_brazo == 3) {
		// Regresar suavemente a reposo
		brazo_angulo_objetivo = -45.0f;
		if (fabs(brazo_angulo - brazo_angulo_objetivo) < 2.0f) {
			estado_brazo = 0;
		}
	}
	// Suavizado del angulo del brazo (interpolacion lineal)
	float velocidad_brazo = (estado_brazo == 2) ? 8.0f : 2.5f;
	brazo_angulo += (brazo_angulo_objetivo - brazo_angulo) * dt * velocidad_brazo;

	// ==========================================
	// ESFERA: tiro parabolico + rebotes
	// ==========================================
	if (estado_esfera == 1) {
		// Volando: aplicar fisica
		esfera_vy -= GRAVEDAD * dt;
		esfera_x += esfera_vx * dt;
		esfera_y += esfera_vy * dt;
		esfera_z += esfera_vz * dt;

		// Rebote contra el piso (y = -1.6 para que la esfera de radio 0.4 toque y=-2)
		float piso = -1.6f;
		if (esfera_y < piso && esfera_vy < 0.0f) {
			esfera_y = piso;
			esfera_vy = -esfera_vy * COEF_REBOTE;
			esfera_vx *= 0.92f;
			esfera_vz *= 0.92f;
			rebotes++;

			// En el segundo rebote, corregir rumbo directo al hueco
			// para que el tercer bote caiga exactamente en el
			if (rebotes == 2) {
				float dx = POS_HUECO.x - esfera_x;
				float dz = POS_HUECO.z - esfera_z;
				float t_corto = 0.6f;
				esfera_vx = dx / t_corto;
				esfera_vz = dz / t_corto;
				esfera_vy = (0.5f * GRAVEDAD * t_corto * t_corto) / t_corto; // arco bajo
			}
		}

		// Entra al hueco solo despues del segundo rebote (tercer bote)
		if (rebotes >= 2) {
			float dx = esfera_x - POS_HUECO.x;
			float dz = esfera_z - POS_HUECO.z;
			float distH = sqrt(dx * dx + dz * dz);
			if (distH < 0.8f && esfera_y <= piso + 0.15f) {
				estado_esfera = 2;
			}
		}
	}
	else if (estado_esfera == 2) {
		// En el hueco: cae al vacio
		esfera_vy -= GRAVEDAD * dt;
		esfera_y += esfera_vy * dt;
		esfera_x = POS_HUECO.x;
		esfera_z = POS_HUECO.z;
		if (esfera_y < -5.0f) {
			estado_esfera = 3; // reaparecer
		}
	}
	else if (estado_esfera == 3) {
		// Reaparecer en el brazo
		colocarEsferaEnBrazo();
		esfera_lanzada = false;
		estado_esfera = 0;

		// Se reinicia el ciclo: humo se apaga, brazo a reposo (ya esta), fuego sigue
		estado_humo = 3; // empieza a bajar
	}
}


int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();
	esfera_proyectil.init();
	esfera_proyectil.load();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

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
	fuego_texture = Texture("Textures/fuego.png");
	fuego_texture.LoadTextureA();
	humo_texture = Texture("Textures/humo.png");
	humo_texture.LoadTextureA();

	metal_texture = Texture("Textures/metal.png");
	metal_texture.LoadTextureA();
	brazo_texture = Texture("Textures/brick.png"); // reusamos brick para el brazo
	brazo_texture.LoadTextureA();
	hueco_texture = Texture("Textures/hueco.png");
	hueco_texture.LoadTextureA();


	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista.obj");
	Nave_M = Model();
	Nave_M.LoadModel("Models/nave.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Aeolipile_base_M = Model();
	Aeolipile_base_M.LoadModel("Models/Aeolipile_base.obj");
	Aeolipile_M = Model();
	Aeolipile_M.LoadModel("Models/Aeolipile.obj");

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

	// Luz puntual del fuego (parpadeante naranja)
	pointLights[1] = PointLight(1.0f, 0.5f, 0.1f,
		0.2f, 1.5f,
		0.0f, -0.5f, 1.5f,
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




	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	movCoche = 0.0f;
	movOffset = 0.01f;
	rotllanta = 0.0f;
	rotllantaOffset = 10.0f;

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	// inicializar esfera en el brazo
	colocarEsferaEnBrazo();

	lastTime = glfwGetTime();
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), dt * 100);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// ============================================
		// Actualizacion del ciclo de la animacion compleja
		// ============================================
		actualizarCiclo(dt);

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
		uniformTextureOffset = shaderList[0].getOffsetLocation();

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

		// Luz del fuego oscila con su intensidad
		float intensidadFuego = fuego_intensidad * (0.8f + 0.2f * sin(fuego_timer * 8.0f));
		// (la luz puntual del fuego se actualiza modificando su ambient/diffuse seria
		//  ideal pero por simplicidad la dejamos fija)

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		//reiniciar variables antes de que sean enviadas al shader

		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);

		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		// PISO
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		// =================================================
		// HUECO en el piso (donde cae la esfera)
		// Lo dibujamos un poquito arriba del piso para evitar z-fighting
		// =================================================
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(POS_HUECO.x, -1.98f, POS_HUECO.z));
		model = glm::scale(model, glm::vec3(2.0f, 1.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		// Color blanco para que la textura del hueco se vea correctamente
		glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		hueco_texture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[9]->RenderMesh();
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));


		// =================================================
		// AEOLIPILE (base + esfera giratoria)
		// =================================================
		model = glm::mat4(1.0);
		model = glm::translate(model, POS_AEOLIPILE);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Aeolipile_base_M.RenderModel();

		model = glm::translate(model, glm::vec3(0.0f, 5.0f, -0.1f));
		// La esfera del aeolipile gira mas rapido conforme hay mas humo
		static float aeo_rot = 0.0f;
		aeo_rot += dt * (60.0f + 240.0f * humo_intensidad);
		model = glm::rotate(model, aeo_rot * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Aeolipile_M.RenderModel();
		modelaux = model; // guardamos matriz padre para los humos hijos

		// =================================================
		// HUMO (3 estados, textura animada, blending) — DIBUJAR AL FINAL
		// IMPORTANTE: el humo NO hereda la rotacion del aeolipile,
		// se posiciona en coordenadas mundo respecto a POS_AEOLIPILE.
		// Tambien desactivamos glDepthMask para que las particulas no se
		// ocluyan entre si y el blending se vea continuo.
		// =================================================
		if (humo_intensidad > 0.01f) {
			int num_particulas = (int)(humo_intensidad * 8) + 4;

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE); // NO escribir al depth buffer

			// Punto de emision: justo arriba de la esfera del aeolipile
			glm::vec3 emisor = POS_AEOLIPILE + glm::vec3(0.0f, 5.7f, -0.1f);

			for (int p = 0; p < num_particulas; p++) {
				float fase = (float)p / (float)num_particulas;
				float altura_p = fmod(humo_offset_v + fase, 1.0f); // 0 a 1
				float escala_p = (0.5f + altura_p * 1.2f) * humo_intensidad;
				// dispersion lateral en zigzag amplio (crece con la altura)
				float disp = 0.1f + altura_p * 0.5f;
				float lateral_x = disp * sin((humo_offset_v + fase) * 6.28f * 1.3f);
				float lateral_z = disp * cos((humo_offset_v + fase) * 6.28f * 1.7f);

				model = glm::mat4(1.0);
				model = glm::translate(model, emisor + glm::vec3(lateral_x, altura_p * 4.0f, lateral_z));
				// rotacion individual de cada particula para variedad
				model = glm::rotate(model, (fase * 360.0f + humo_offset_v * 100.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::scale(model, glm::vec3(escala_p, escala_p, escala_p));

				// offset de textura (textura animada del humo)
				glm::vec2 toffsetHumo = glm::vec2(humo_offset_u + fase * 0.3f, humo_offset_v * 0.5f);
				glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffsetHumo));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				color = glm::vec3(1.0f, 1.0f, 1.0f);
				glUniform3fv(uniformColor, 1, glm::value_ptr(color));
				humo_texture.UseTexture();
				Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshList[7]->RenderMesh();
			}
			glDepthMask(GL_TRUE);  // restaurar
			glDisable(GL_BLEND);
			toffset = glm::vec2(0.0f, 0.0f);
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		}

		// =================================================
		// FUEGO (debajo del Aeolipile, animacion textura+escala)
		// =================================================
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(POS_AEOLIPILE.x, POS_AEOLIPILE.y + 1.0f, POS_AEOLIPILE.z));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		model = glm::rotate(model, fuego_timer * 100.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		float escFuego = fuego_intensidad * (1.0f + 0.15f * sin(fuego_timer * 10.0f));
		model = glm::scale(model, glm::vec3(escFuego, escFuego, escFuego));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		fuego_texture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[7]->RenderMesh();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		// =================================================
		// BRAZO PALANCA
		// =================================================
		// Posicionado a la altura de la esfera del aeolipile, a un costado.
		// Rotacion en eje Z controla el movimiento de lanzamiento (arriba/abajo).
		// El brazo se extiende en -X local (hacia el aeolipile) cuando esta en reposo.
		model = glm::mat4(1.0);
		model = glm::translate(model, POS_BRAZO_BASE);
		// Reflejar en X para que el brazo apunte hacia el aeolipile (hacia X negativo)
		model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
		// Rotacion del lanzamiento (alrededor de Z): -45 reposo, +60 lanzar
		model = glm::rotate(model, brazo_angulo * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		modelaux = model; // matriz padre del brazo (para la esfera cargada)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		brazo_texture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[8]->RenderMesh();

		// =================================================
		// ESFERA METALICA (proyectil)
		// =================================================
		// Si la esfera esta en el brazo (estado 0), su posicion sigue al extremo del brazo
		if (estado_esfera == 0 || (estado_esfera == 1 && !esfera_lanzada)) {
			// Con scale(-1,1,1), el extremo local (2.5,0,0) queda en mundo como (-2.5 rotado)
			// debemos aplicar la matriz del brazo al punto extremo local
			glm::vec4 puntaLocal = glm::vec4(2.5f, 0.0f, 0.0f, 1.0f);
			glm::vec4 puntaMundo = modelaux * puntaLocal;
			esfera_x = puntaMundo.x;
			esfera_y = puntaMundo.y + 0.4f; // un poco encima del brazo
			esfera_z = puntaMundo.z;
		}

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(esfera_x, esfera_y, esfera_z));
		// Rotacion de la esfera para dar sensacion de movimiento
		static float esfera_rot = 0.0f;
		if (estado_esfera == 1) esfera_rot += dt * 720.0f;
		model = glm::rotate(model, esfera_rot * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		metal_texture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		// Dibujar esfera: siempre que no haya caido al vacio del hueco
		if (estado_esfera != 2 || esfera_y > -4.5f) {
			esfera_proyectil.render();
		}

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}