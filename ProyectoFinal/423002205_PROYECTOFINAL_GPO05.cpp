#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Load Models
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Flock.h"
#include "modelAnim.h"

// --- Audio: MiniAudio ---
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();
void animacionCofre();

void MostrarPantallaCarga(GLFWwindow* window, Shader& pantallaShader, GLuint VAO, GLuint texturaFondo, GLuint texturaFill, GLuint texturaBorder, float progreso) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 identidad = glm::mat4(1.0f);
	pantallaShader.Use();

	// 1. DIBUJAR LA IMAGEN DE FONDO
	glUniformMatrix4fv(glGetUniformLocation(pantallaShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(identidad));
	glUniformMatrix4fv(glGetUniformLocation(pantallaShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(identidad));
	glUniformMatrix4fv(glGetUniformLocation(pantallaShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(identidad));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturaFondo);
	glUniform1i(glGetUniformLocation(pantallaShader.Program, "pantallaTextura"), 0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// --- ACTIVAR TRANSPARENCIA PARA LA UI ---
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 2. DIBUJAR EL RELLENO AZUL (Fondo de la barra)
	glBindTexture(GL_TEXTURE_2D, texturaFill);
	glm::mat4 modelFill = glm::mat4(1.0f);
	// Matemáticas para centrar la barra y hacer que crezca desde la izquierda
	modelFill = glm::translate(modelFill, glm::vec3(-0.52f + (0.56f * progreso), -0.8f, 0.0f));
	modelFill = glm::scale(modelFill, glm::vec3(0.6f * progreso, 0.035f, 1.0f));
	glUniformMatrix4fv(glGetUniformLocation(pantallaShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelFill));
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// 3. DIBUJAR EL MARCO METÁLICO (Encima del relleno)
	glBindTexture(GL_TEXTURE_2D, texturaBorder);
	glm::mat4 modelBorder = glm::mat4(1.0f);
	// El marco siempre está centrado en X=0 y tiene su tamaño máximo fijo (0.6f)
	modelBorder = glm::translate(modelBorder, glm::vec3(0.0f, -0.8f, 0.0f));
	modelBorder = glm::scale(modelBorder, glm::vec3(0.6f, 0.05f, 1.0f));
	glUniformMatrix4fv(glGetUniformLocation(pantallaShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelBorder));
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// --- DESACTIVAR TRANSPARENCIA ---
	glDisable(GL_BLEND);
	glBindVertexArray(0);

	// Actualizar la pantalla
	glfwSwapBuffers(window);
	glfwPollEvents();
}

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(24.5f, 5.0f, -8.5f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

bool mostrarLuces = false; // Activar/desactivar cubos de pointLights

float tiempoInicioAnimacion = 0.0f;
// --- Variables de Audio ---
ma_engine engine;
ma_sound musicaFondo;
ma_sound musicaTocadiscos;
float volumenMusica = 0.25f; // Empezamos al 25%
// --- Variables de animacion turntale/personaje ---
bool fiestaActiva = false;
float rotacionDiscos = 0.0f; // Acumulador para rotación fluida
// --- Variables de Entorno ---
bool esDeDia = true; // El escenario iniciará de día

glm::vec3 directionSpotlight(0.0f, 0.0f, 0.0f);
glm::vec3 positionSpotlight(0.0f, 0.0f, 0.0f);

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(-6.420f, 1.6f, 2.0f), //fondo
	glm::vec3(-0.3f, 0.5f, 3.797f), //fogata
	glm::vec3(0.146f, 2.354f,  -3.372f), // chandelier
	glm::vec3(-2.15f, 1.1f, -4.07f), //candelabras_comedor1
	glm::vec3(-0.225f, 1.09f, 0.759f),
	glm::vec3(1.37f, 1.2f, -5.895f)
};

// Estructura para almacenar las propiedades individuales de cada flama de una vela
struct DatosVela {
	glm::vec3 posicion;
	float escala;
};

// Vector que contiene todas las flamas de velas.
std::vector<DatosVela> listaVelas = {
	{ glm::vec3(-2.15f, 1.1f, -4.07f), 0.15f },  // Vela 1
	{ glm::vec3(-2.047f, 1.05f, -4.126f), 0.15f }, // Vela 2
	{ glm::vec3(-2.245f, 1.05f, -4.02f), 0.15f },   // Vela 3
	{ glm::vec3(0.125f, 2.45f, -2.734f), 0.25f },
	{ glm::vec3(-0.425f, 2.45f, -3.066f), 0.25f },
	{ glm::vec3(-0.412f, 2.45f, -3.709f), 0.25f },
	{ glm::vec3(0.151f, 2.45f, -4.019f), 0.25f },
	{ glm::vec3(0.701f, 2.45f, -3.686f), 0.25f },
	{ glm::vec3(0.688f, 2.45f, -3.044f), 0.25f },
	{ glm::vec3(-0.215f, 1.05f, 0.895f), 0.15f }, // Comedor 2
	{ glm::vec3(-0.225f, 1.09f, 0.759f), 0.15f }, // Comedor 2
	{ glm::vec3(-0.235f, 1.05f, 0.639f), 0.15f }, // Comedor 2
	{ glm::vec3(-1.03f, 1.05f, 0.656f), 0.15f }, // Comedor 2
	{ glm::vec3(-1.12f, 1.09f, 0.678f), 0.15f }, // Comedor 2
	{ glm::vec3(-1.25f, 1.05f, 0.75f), 0.15f }, // Comedor 2
	{ glm::vec3(-6.304f, 1.5f, 3.11f), 0.15f }, // Fondo
	{ glm::vec3(-6.270f, 1.55f, 2.994f), 0.15f }, // Fondo
	{ glm::vec3(-6.23f, 1.5f, 2.878f), 0.15f }, // Fondo
	{ glm::vec3(-6.38f, 1.5f, 1.67f), 0.15f }, // Fondo
	{ glm::vec3(-6.418f, 1.55f, 1.55f), 0.15f }, // Fondo
	{ glm::vec3(-6.46f, 1.5f, 1.443f), 0.15f }, // Fondo
	{ glm::vec3(1.47f, 1.0f, -5.82f), 0.15f }, // Mesita
	{ glm::vec3(1.37f, 1.05f, -5.895f), 0.15f }, // Mesita
	{ glm::vec3(1.274f, 1.0f, -5.97f), 0.15f } // Mesita
};

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

float quadFuego[] = {
	// Posiciones (X,Y,Z)   // Coordenadas UV (U,V)
	-0.5f,  0.5f, 0.0f,     0.0f, 1.0f,  // Arriba-Izquierda
	-0.5f, -0.5f, 0.0f,     0.0f, 0.0f,  // Abajo-Izquierda
	 0.5f, -0.5f, 0.0f,     1.0f, 0.0f,  // Abajo-Derecha

	-0.5f,  0.5f, 0.0f,     0.0f, 1.0f,  // Arriba-Izquierda
	 0.5f, -0.5f, 0.0f,     1.0f, 0.0f,  // Abajo-Derecha
	 0.5f,  0.5f, 0.0f,     1.0f, 1.0f   // Arriba-Derecha
};



glm::vec3 Light1 = glm::vec3(0);


// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

float   apertura = 0.0f;

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Goldshire Inn", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// --- INICIALIZAR MOTOR DE AUDIO ---
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
		std::cout << "Error: No se pudo iniciar MiniAudio" << std::endl;
		return -1;
	}
	ma_sound_init_from_file(&engine, "Audio/forest_elwynn_forest.mp3", 0, NULL, NULL, &musicaFondo);
	ma_sound_set_spatialization_enabled(&musicaFondo, MA_FALSE);
	ma_sound_set_volume(&musicaFondo, volumenMusica);
	ma_sound_set_looping(&musicaFondo, MA_TRUE);
	ma_sound_start(&musicaFondo);

	ma_sound_init_from_file(&engine, "Audio/macarena.mp3", 0, NULL, NULL, &musicaTocadiscos);
	ma_sound_set_looping(&musicaTocadiscos, MA_TRUE);
	ma_sound_set_position(&musicaTocadiscos, 3.876f, 1.065f, 2.7f);

	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
	Shader algasShader("Shaders/anim_algas.vs", "Shaders/anim_algas.frag");
	Shader fuego("Shaders/fuego.vs", "Shaders/fuego.frag");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.frag");


	// --- SETUP DE LA PANTALLA DE CARGA ---
	Shader pantallaShader("Shaders/pantalla.vs", "Shaders/pantalla.frag");

	float quadPantalla[] = {
		// Posiciones (X,Y,Z)   // UVs (location 2)
		-1.0f,  1.0f, 0.0f,     0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,     1.0f, 0.0f,

		-1.0f,  1.0f, 0.0f,     0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,     1.0f, 1.0f
	};

	GLuint cargaVAO, cargaVBO;
	glGenVertexArrays(1, &cargaVAO);
	glGenBuffers(1, &cargaVBO);
	glBindVertexArray(cargaVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cargaVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadPantalla), quadPantalla, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Carga del fondo de pantalla
	GLuint texturaCarga = SOIL_load_OGL_texture("Models/ui/loading.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (texturaCarga == 0) {
		std::cout << "¡ERROR: SOIL NO PUDO LEER LA IMAGEN DE CARGA!" << std::endl;
	}
	// Carga de la UI de la barra
	GLuint texturaFill = SOIL_load_OGL_texture("Models/ui/loading-barfill.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texturaBorder = SOIL_load_OGL_texture("Models/ui/loading-barborder.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	
	// --- FIN SETUP DE LA PANTALLA DE CARGA ---
	
	// --- INICIO DE CARGA DE MODELOS ---
	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.1f);

	Model piso_interior((char*)"Models/piso/piso_interior.obj");

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.25f);

	Model piso_exterior((char*)"Models/piso/piso_exterior.obj");

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.35f);

	Model candelabras((char*)("Models/myCandelabras/candelabras.obj"));
	Model specular_barrel_stack((char*)("Models/specularBarrels/specular_barrel_stack.obj"));
	Model woodChair1((char*)("Models/woodChair/woodChair.obj"));
	Model fachada((char*)("Models/fachada/fachada.obj"));

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.45f);

	Model mailbox((char*)("Models/fachada/mailbox.obj"));
	Model fence((char*)("Models/fachada/fence.obj"));
	Model comedor1((char*)("Models/comedor1/comedor1.obj"));
	Model comedor2((char*)("Models/comedor2/comedor2.obj"));
	Model fondo((char*)("Models/fondo/fondo.obj"));

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.5f);

	Model chairs_set1((char*)("Models/woodChair/chairs_set1.obj"));
	Model chairs_set2((char*)("Models/woodChair/chairs_set2.obj"));
	Model chairs_set3((char*)("Models/woodChair/chairs_set3.obj"));
	Model mesita((char*)("Models/woodChair/mesita2.obj"));
	Model candelabras_comedor1((char*)("Models/myCandelabras/candelabras_comedor1.obj"));
	Model barrel_stack((char*)("Models/barrels/barrel_stack.obj"));
	Model chandelier((char*)("Models/chandelier/chandelier.obj"));

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.6f);

	Model fishModelType1((char*)("Models/fish/fish.obj"));
	Model fishModelType2((char*)("Models/fish/fish2.obj"));
	Model fishModelType3((char*)("Models/fish/fish3.obj"));
	Model fishModelType4((char*)("Models/fish/fish4.obj"));

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.7f);

	Model peceraBase((char*)("Models/acuario/pecera_base.obj"));
	Model peceraAlgas((char*)("Models/acuario/pecera_algas.obj"));
	Model peceraCristal((char*)("Models/acuario/pecera_cristal.obj"));
	Model cofreInferior((char*)("Models/acuario/cofre_inferior.obj"));
	Model cofreSuperior((char*)("Models/acuario/cofre_superior.obj"));

	Model fogata((char*)("Models/campfire/campfire.obj"));

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.8f);

	Model turntable_base((char*)("Models/turntable/base.obj"));
	Model turntable_disco_izquierda((char*)("Models/turntable/disco_izquierda.obj"));
	Model turntable_disco_derecha((char*)("Models/turntable/disco_derecha.obj"));
	Model turntable_trompeta_izquierda((char*)("Models/turntable/trompeta_izquierda.obj"));
	Model turntable_trompeta_derecha((char*)("Models/turntable/trompeta_derecha.obj"));

	Model arbol1((char*)("Models/arboles/arbol1.obj"));
	Model arbol2((char*)("Models/arboles/arbol2.obj"));
	Model arbol3((char*)("Models/arboles/arbol3.obj"));
	Model arbol4((char*)("Models/arboles/arbol4.obj"));

	Model cielo((char*)("Models/sky/cielo.obj"));
	Model planeta((char*)("Models/sky/planeta.obj"));
	
	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 0.9f);

	//Modelo de animaci�n
	ModelAnim personajeIdle("Models/personaje/standing_idle.dae");
	ModelAnim personajeBaile("Models/personaje/macarena.dae");
	personajeIdle.initShaders(animShader.Program);
	personajeBaile.initShaders(animShader.Program);

	MostrarPantallaCarga(window, pantallaShader, cargaVAO, texturaCarga, texturaFill, texturaBorder, 1.0f);
	// --- FIN DE CARGA DE MODELOS ---

	// Inicializar el Cardumen usando la clase Flock
	glm::vec3 tankMin(3.6f, 0.802f, -6.9f);
	glm::vec3 tankMax(6.1f, 2.6f, -2.7f);
	Flock myFlock(tankMin, tankMax);
	// Agregando peces de diferentes especies (cantidad, tipo)
	myFlock.addSchool(40, 1);
	myFlock.addSchool(40, 2);
	myFlock.addSchool(15, 3);
	myFlock.addSchool(5, 4);

	
	// Cargar Textura del Sprite Sheet para el fuego
	GLuint texturaFuego = SOIL_load_OGL_texture("Models/fuego/flamelicksmallblue.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

	// Crear VAO y VBO para el Quad
	GLuint fuegoVAO, fuegoVBO;
	glGenVertexArrays(1, &fuegoVAO);
	glGenBuffers(1, &fuegoVBO);
	glBindVertexArray(fuegoVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fuegoVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadFuego), quadFuego, GL_STATIC_DRAW);

	// Atributo 0: Posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Atributo 1: UV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Cargar Textura del Sprite Sheet para el fuego de la fogata
	// 1. Cargamos la textura agregando el flag SOIL_FLAG_TEXTURE_REPEATS
	GLuint texturaFogata = SOIL_load_OGL_texture(
		"Models/fuego/12ph_kultiras_fireplace01_6919403.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS // <-- ¡Este flag es la clave!
	);

	// 2. Nos aseguramos de decirle a OpenGL que repita la textura en ambos ejes (S y T equivalen a X y Y en texturas)
	glBindTexture(GL_TEXTURE_2D, texturaFogata);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0); // Desvinculamos por seguridad
	Shader fogataShader("Shaders/fogata.vs", "Shaders/fogata.frag");

	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();


	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 300.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		animacionCofre();
		// --- ACTUALIZAR LOS "OÍDOS" PARA EL AUDIO 3D ---
		// 1. Le decimos a MiniAudio dónde está parado el jugador (Posición de la cámara)
		ma_engine_listener_set_position(&engine, 0, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// 2. Le decimos a MiniAudio hacia dónde está volteando a ver el jugador (Frente de la cámara)
		ma_engine_listener_set_direction(&engine, 0, camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);

	   
		// OpenGL options
		glEnable(GL_DEPTH_TEST);
		
		//Load Model
	
		// --- PERFILES DE DÍA Y NOCHE ---
		glm::vec3 dirLuz, ambientLuz, diffuseLuz, specularLuz;

		if (esDeDia) {
			// PERFIL TARDE (Golden Hour): Cielo celeste, luz cálida (naranja/amarilla) y sombras alargadas
			glClearColor(0.53f, 0.75f, 0.85f, 1.0f);
			dirLuz = glm::vec3(-0.8f, -0.5f, -0.2f); // Sol bajando en el horizonte
			ambientLuz = glm::vec3(0.4f, 0.4f, 0.4f);
			diffuseLuz = glm::vec3(1.0f, 0.8f, 0.6f); // Tono atardecer
			specularLuz = glm::vec3(1.0f, 0.9f, 0.8f);
		}else {
			// PERFIL NOCHE: Cielo oscuro, luz de luna azulada y sombras tenues
			glClearColor(0.02f, 0.05f, 0.1f, 1.0f);
			dirLuz = glm::vec3(-0.2f, -1.0f, -0.3f); // Luna alta en el cielo
			ambientLuz = glm::vec3(0.1f, 0.1f, 0.15f); // Ambiente muy oscuro
			diffuseLuz = glm::vec3(0.15f, 0.2f, 0.35f); // Tono azul oscuro
			specularLuz = glm::vec3(0.2f, 0.25f, 0.4f);
		}

		// Pintar el fondo con el color elegido
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Usar el shader antes de enviarle variables
		lightingShader.Use();

		// Enviar la dirección y el brillo especular que siempre aplican igual
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), dirLuz.x, dirLuz.y, dirLuz.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), specularLuz.x, specularLuz.y, specularLuz.z);

        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"),1);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		
		glm::vec3 colorChandelier(1.0f, 0.7f, 0.2f);
		// Point light 1 (Fondo)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), colorChandelier.x * 0.1f, colorChandelier.y * 0.1f, colorChandelier.z * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), colorChandelier.x * 0.6f, colorChandelier.y * 0.6f, colorChandelier.z * 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), colorChandelier.x * 0.3f, colorChandelier.y * 0.3f, colorChandelier.z * 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.22f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.20f);


		// Point light 2 (Fogata)
		glm::vec3 colorFogata(1.0f, 0.5f, 0.1f);
		float flicker = 0.8f + (sin(glfwGetTime() * 12.0f) * 0.1f) + (cos(glfwGetTime() * 21.0f) * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), colorFogata.x * 0.1f, colorFogata.y * 0.1f, colorFogata.z * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), colorFogata.x* flicker, colorFogata.y* flicker, colorFogata.z* flicker);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), colorFogata.x * 0.3f, colorFogata.y * 0.3f, colorFogata.z * 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.14f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.07f);
		
		// Point light 3 (Chandelier)
		
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), colorChandelier.x * 0.1f, colorChandelier.y * 0.1f, colorChandelier.z * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), colorChandelier.x * 0.6f, colorChandelier.y * 0.6f, colorChandelier.z * 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), colorChandelier.x * 0.3f, colorChandelier.y * 0.3f, colorChandelier.z * 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.22f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.20f);
		
		// Point light 4
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"), colorChandelier.x * 0.1f, colorChandelier.y * 0.1f, colorChandelier.z * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"), colorChandelier.x * 0.6f, colorChandelier.y * 0.6f, colorChandelier.z * 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"), colorChandelier.x * 0.3f, colorChandelier.y * 0.3f, colorChandelier.z * 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.22f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.20f);

		// Point light 5
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].position"), pointLightPositions[4].x, pointLightPositions[4].y, pointLightPositions[4].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].ambient"), colorChandelier.x * 0.1f, colorChandelier.y * 0.1f, colorChandelier.z * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].diffuse"), colorChandelier.x * 0.6f, colorChandelier.y * 0.6f, colorChandelier.z * 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].specular"), colorChandelier.x * 0.3f, colorChandelier.y * 0.3f, colorChandelier.z * 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].linear"), 0.22f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].quadratic"), 0.20f);

		// Point light 6
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].position"), pointLightPositions[5].x, pointLightPositions[5].y, pointLightPositions[5].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].ambient"), colorChandelier.x * 0.1f, colorChandelier.y * 0.1f, colorChandelier.z * 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].diffuse"), colorChandelier.x * 0.6f, colorChandelier.y * 0.6f, colorChandelier.z * 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].specular"), colorChandelier.x * 0.3f, colorChandelier.y * 0.3f, colorChandelier.z * 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].linear"), 0.22f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].quadratic"), 0.20f);
				
		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		
		// Control de iluminación dentro de la posada
		bool adentroDeLaPosada = (camera.GetPosition().x < 8.0f && camera.GetPosition().x > -16.0f && camera.GetPosition().z < 5.2f && camera.GetPosition().z > -8.2f && camera.GetPosition().y < 8.2f);

		if (adentroDeLaPosada) {
			// Adentro siempre es mas oscuro que afuera
			glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), ambientLuz.x * 0.125, ambientLuz.y * 0.125, ambientLuz.z * 0.125);
			glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), diffuseLuz.x * 0.125, diffuseLuz.y * 0.125, diffuseLuz.z * 0.125);
		}
		else {

			glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), ambientLuz.x, ambientLuz.y, ambientLuz.z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), diffuseLuz.x, diffuseLuz.y, diffuseLuz.z);
		}
		
		glm::mat4 model(1);

		//Carga de modelo 
        view = camera.GetViewMatrix();	
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "activeTrans"), 0.0f);
		glUniform4f(glGetUniformLocation(lightingShader.Program, "activeAlpha"), 1.0f, 1.0f, 1.0f, 1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		piso_interior.Draw(lightingShader);
		piso_exterior.Draw(lightingShader);

		model = glm::mat4(1);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		fachada.Draw(lightingShader);
		mailbox.Draw(lightingShader);
		fence.Draw(lightingShader);

		comedor1.Draw(lightingShader);
		comedor2.Draw(lightingShader);
		fondo.Draw(lightingShader);
		chairs_set1.Draw(lightingShader);
		chairs_set2.Draw(lightingShader);
		chairs_set3.Draw(lightingShader);
		mesita.Draw(lightingShader);
		candelabras_comedor1.Draw(lightingShader);

		barrel_stack.Draw(lightingShader);

		chandelier.Draw(lightingShader);

		peceraBase.Draw(lightingShader);

		fogata.Draw(lightingShader);

		// Activamos la bandera de transparencia/discard en el shader
		glUniform1f(glGetUniformLocation(lightingShader.Program, "activeTrans"), 1.0f);

		arbol1.Draw(lightingShader);
		arbol2.Draw(lightingShader);
		arbol3.Draw(lightingShader);
		arbol4.Draw(lightingShader);

		// La desactivamos inmediatamente después para no afectar el cielo ni otros modelos
		glUniform1f(glGetUniformLocation(lightingShader.Program, "activeTrans"), 0.0f);
		// -----------------------

		// --- DIBUJAR EL CIELO ---
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(10.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		cielo.Draw(lightingShader);

		// --- DIBUJAR EL PLANETA ---
		float angle = (float)glfwGetTime() * 0.2f;
		glm::mat4 modelPlaneta = glm::mat4(1.0f);
		modelPlaneta = glm::scale(modelPlaneta, glm::vec3(7.0f));
		glm::vec3 pivotePlaneta = glm::vec3(-5.1421f, 7.7288f, -3.2392f); 
		modelPlaneta = glm::translate(modelPlaneta, pivotePlaneta);
		modelPlaneta = glm::rotate(modelPlaneta, angle, glm::vec3(1.0f, 1.0f, 0.0f));
		modelPlaneta = glm::translate(modelPlaneta, -pivotePlaneta);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPlaneta));
		planeta.Draw(lightingShader);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		turntable_base.Draw(lightingShader);
		float latido = 1.0f;
		if (fiestaActiva) {
			// Aumentamos la rotación con deltaTime para que la velocidad sea constante
			rotacionDiscos += 2.0f * deltaTime;
			// Calculamos el latido de la trompeta solo si hay fiesta
			latido = 1.0f + (sin((float)glfwGetTime() * 5.0f) * 0.05f);
		}
		// --- RENDERIZAR DISCO IZQUIERDO ---
		glm::mat4 modelDisco = glm::mat4(1.0f);
		glm::vec3 pivoteDiscoIzq(4.2522f, 0.9441f, 3.8168f); 

		modelDisco = glm::translate(modelDisco, pivoteDiscoIzq);
		modelDisco = glm::rotate(modelDisco, rotacionDiscos, glm::vec3(0.0f, 1.0f, 0.0f));
		modelDisco = glm::translate(modelDisco, -pivoteDiscoIzq);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelDisco));
		turntable_disco_izquierda.Draw(lightingShader);

		// --- RENDERIZAR DISCO DERECHO ---
		modelDisco = glm::mat4(1.0f);
		glm::vec3 pivoteDiscoDer(3.5606f, 0.9552f, 3.8148f);

		modelDisco = glm::translate(modelDisco, pivoteDiscoDer);
		modelDisco = glm::rotate(modelDisco, rotacionDiscos, glm::vec3(0.0f, 1.0f, 0.0f));
		modelDisco = glm::translate(modelDisco, -pivoteDiscoDer);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelDisco));
		turntable_disco_derecha.Draw(lightingShader);

		// --- RENDERIZAR TROMPETA IZQUIERDA ---
		glm::mat4 modelTrompeta = glm::mat4(1.0f);
		glm::vec3 pivoteTrompetaIzq(4.6125f, 1.3925f, 3.5561f); 
		// Creamos un multiplicador de escala que oscile.
		// glfwGetTime() * 5.0f -> Controla qué tan rápido "late" (el tempo o BPM de la música)
		// * 0.1f -> Controla cuánto crece (10% más grande)
		modelTrompeta = glm::translate(modelTrompeta, pivoteTrompetaIzq);
		// Escalamos uniformemente en X, Y y Z
		modelTrompeta = glm::scale(modelTrompeta, glm::vec3(latido, latido, latido));
		modelTrompeta = glm::translate(modelTrompeta, -pivoteTrompetaIzq);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTrompeta));
		turntable_trompeta_izquierda.Draw(lightingShader);

		// --- RENDERIZAR TROMPETA DERECHA ---
		modelTrompeta = glm::mat4(1.0f);
		glm::vec3 pivoteTrompetaDer(3.2672f, 1.4285f, 3.5060f);
		modelTrompeta = glm::translate(modelTrompeta, pivoteTrompetaDer);
		// Escalamos uniformemente en X, Y y Z
		modelTrompeta = glm::scale(modelTrompeta, glm::vec3(latido, latido, latido));
		modelTrompeta = glm::translate(modelTrompeta, -pivoteTrompetaDer);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTrompeta));
		turntable_trompeta_derecha.Draw(lightingShader);


		glBindVertexArray(0);

		// --- RENDERIZADO DEL PERSONAJE ANIMADO ---
		
		/*_______________________________Personaje Animado___________________________*/
		animShader.Use();
		modelLoc = glGetUniformLocation(animShader.Program, "model");
		viewLoc = glGetUniformLocation(animShader.Program, "view");
		projLoc = glGetUniformLocation(animShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3f(glGetUniformLocation(animShader.Program, "material.specular"), 0.5f, 0.5f, 0.5f);
		glUniform1f(glGetUniformLocation(animShader.Program, "material.shininess"), 32.0f);
		glUniform3f(glGetUniformLocation(animShader.Program, "light.ambient"), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(animShader.Program, "light.diffuse"), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(animShader.Program, "light.specular"), 0.2f, 0.2f, 0.2f);
		glUniform3f(glGetUniformLocation(animShader.Program, "light.direction"), 0.0f, -1.0f, -1.0f);
		view = camera.GetViewMatrix();

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(4.2f, 0.22f, 2.332f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.8f));	// it's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		if (fiestaActiva) {
			personajeBaile.Draw(animShader);
		}
		else {
			personajeIdle.Draw(animShader);
		}
		glBindVertexArray(0);

		// --- ANIMACIÓN DE LAS ALGAS (Shader Dedicado) ---
		algasShader.Use(); // Cambiamos al nuevo shader
		// 1. Enviamos el tiempo actual
		glUniform1f(glGetUniformLocation(algasShader.Program, "time"), glfwGetTime());

		// 2. Enviamos las matrices de cámara (View y Projection)
		glUniformMatrix4fv(glGetUniformLocation(algasShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(algasShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// 3. Matriz de Modelo y dibujado
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(algasShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		peceraAlgas.Draw(algasShader);
		glBindVertexArray(0);

		lightingShader.Use();

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		cofreInferior.Draw(lightingShader);

		glm::mat4 modelCofreSuperior = glm::mat4(1.0f);
		glm::vec3 pivote(4.226f, 1.0595f, -3.258f);
		modelCofreSuperior = glm::translate(modelCofreSuperior, pivote);
		modelCofreSuperior = glm::rotate(modelCofreSuperior, glm::radians(apertura), glm::vec3(0.0f, 0.0f, 1.0f));
		modelCofreSuperior = glm::translate(modelCofreSuperior, -pivote);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelCofreSuperior));
		cofreSuperior.Draw(lightingShader);

		// --- Dibujo y control de los peces ---

		// 1. Actualizar posiciones
		myFlock.updateFlock(deltaTime);

		// 2. Dibujar cada pez accediendo a los boids del flock
		for (auto& boid : myFlock.boids) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, boid.position);

			if (glm::length(boid.velocity) > 0.001f) {
				glm::mat4 rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), boid.velocity, glm::vec3(0.0f, 1.0f, 0.0f)));
				model *= rotation;
			}

			model = glm::scale(model, glm::vec3(0.5f));

			glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

			// Elegir el modelo correcto basado en el tipo del Boid
			if (boid.type == 1) {
				fishModelType1.Draw(lightingShader);
			}
			else if (boid.type == 2) {
				fishModelType2.Draw(lightingShader);
			}
			else if (boid.type == 3) {
				fishModelType3.Draw(lightingShader);
			}
			else if (boid.type == 4) {
				fishModelType4.Draw(lightingShader);
			}
		}

		// --- RENDERIZADO DEL FUEGO VELAS ---

		// 1. Configurar estado de OpenGL para Fuego Brillante (Additive Blending)
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); // GL_ONE suma los colores, haciendo que el negro sea transparente y el color brille
		glDepthMask(GL_FALSE); // EVITA que los cuadros invisibles bloqueen otros objetos detrás

		fuego.Use();

		// 2. Cálculos de animación de tiempo
		int fotogramasTotales = 16;  
		float fpsLlama = 24.0f;      // Velocidad a la que parpadea el fuego
		int frameLlama = int(glfwGetTime() * fpsLlama) % fotogramasTotales;

		// 3. Enviar variables al shader
		glUniform1i(glGetUniformLocation(fuego.Program, "numCols"), 4);
		glUniform1i(glGetUniformLocation(fuego.Program, "numRows"), 4);
		glUniform1i(glGetUniformLocation(fuego.Program, "currentFrame"), frameLlama);

		glUniformMatrix4fv(glGetUniformLocation(fuego.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(fuego.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Bind de la textura
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturaFuego);

		// 4. Dibujar un Quad para cada vela
		glBindVertexArray(fuegoVAO);

		// -------- DIBUJO DE TODAS LAS VELAS --------
		for (const auto& vela : listaVelas) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, vela.posicion);
			model = glm::scale(model, glm::vec3(vela.escala));

			glUniformMatrix4fv(glGetUniformLocation(fuego.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		
		// Desvincular y restaurar estados de OpenGL
		glBindVertexArray(0);
		glDepthMask(GL_TRUE); 
		glDisable(GL_BLEND);
		// --- FIN RENDERIZADO DEL FUEGO VELAS ---

		// --- RENDERIZADO DEL FUEGO FOGATA ---
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);

		fogataShader.Use();

		// Solo envías el tiempo continuo
		glUniform1f(glGetUniformLocation(fogataShader.Program, "time"), glfwGetTime());

		glUniformMatrix4fv(glGetUniformLocation(fogataShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(fogataShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturaFogata);

		glBindVertexArray(fuegoVAO); // Puedes reusar el mismo quad de las velas

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-0.2f, 0.45f, 3.797f));
		model = glm::scale(model, glm::vec3(2.0f, 1.1f, 2.0f));
		glUniformMatrix4fv(glGetUniformLocation(fogataShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		// --- FIN RENDERIZADO DEL FUEGO FOGATA ---
		
		lightingShader.Use();

		// Activar transparencia para el cristal de la pecera
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Modificamos activeAlpha: Los primeros tres 1.0f son el color (blanco/neutro), el último es la opacidad (0.3f = 30% visible)
		glUniform4f(glGetUniformLocation(lightingShader.Program, "activeAlpha"), 1.0f, 1.0f, 1.0f, 0.6f);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// Dibujar el cristal del acuario
		
		peceraCristal.Draw(lightingShader);

		// Desactivar transparencia
		glUniform4f(glGetUniformLocation(lightingShader.Program, "activeAlpha"), 1.0f, 1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);

		
		if (mostrarLuces)
		{
			// Also draw the lamp object, again binding the appropriate shader
			lampShader.Use();
			// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
			modelLoc = glGetUniformLocation(lampShader.Program, "model");
			viewLoc = glGetUniformLocation(lampShader.Program, "view");
			projLoc = glGetUniformLocation(lampShader.Program, "projection");

			// Set matrices
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			model = glm::mat4(1);
			model = glm::translate(model, lightPos);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			// Draw the light object (using light's vertex attributes)

			for (GLuint i = 0; i < 6; i++)
			{
				model = glm::mat4(1);
				model = glm::translate(model, pointLightPositions[i]);
				model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glBindVertexArray(VAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			glBindVertexArray(0);
		}


		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Limpiar el audio al cerrar la ventana
	ma_sound_uninit(&musicaFondo);
	ma_engine_uninit(&engine);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}
void animacionCofre() {
	float duracionPausa = 3.0f;       // Segundos que el cofre se queda cerrado
	float duracionAnimacion = 2.0f;   // Segundos que tarda en abrir y volver a cerrar
	float tiempoTotal = duracionPausa + duracionAnimacion;

	// fmod crea un reloj que va de 0.0 al tiempoTotal y se reinicia
	float t = fmod(glfwGetTime(), tiempoTotal);

	if (t < duracionPausa) {
		// ESTADO 1: Pausa (Cofre cerrado)
		apertura = 0.0f;
	}
	else {
		// ESTADO 2: Animación de apertura y cierre
		// Normalizamos el tiempo para que empiece en 0.0 al iniciar esta fase
		float tiempoActivo = t - duracionPausa;

		// Mapeamos el tiempo activo a media onda seno (de 0 a PI)
		// La fórmula es: sin(tiempo * (PI / duracion))
		float cicloApertura = sin(tiempoActivo * (3.14159265f / duracionAnimacion));
		apertura = cicloApertura * -90.0f;
	}
}

// Moves/alters the camera positions based on user input
void DoMovement()
{

	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);


	}

	// --- Controles de Volumen ---
	float velocidadVolumen = 0.5f * deltaTime; // Qué tan rápido sube/baja

	if (keys[GLFW_KEY_P]) // Subir volumen
	{
		volumenMusica += velocidadVolumen;
		if (volumenMusica > 1.0f) volumenMusica = 1.0f; // Tope máximo
		ma_sound_set_volume(&musicaFondo, volumenMusica);
	}

	if (keys[GLFW_KEY_O]) // Bajar volumen
	{
		volumenMusica -= velocidadVolumen;
		if (volumenMusica < 0.0f) volumenMusica = 0.0f; // Tope mínimo
		ma_sound_set_volume(&musicaFondo, volumenMusica);
	}
	
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	// Activar/Desactivar visualización de los cubos de luz
	if (keys[GLFW_KEY_L])
	{
		mostrarLuces = !mostrarLuces;
		keys[GLFW_KEY_L] = false; // Forzamos a falso inmediatamente para evitar que parpadee rapidísimo por dejar la tecla presionada
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		fiestaActiva = !fiestaActiva; // Cambia de false a true, o de true a false

		if (fiestaActiva) {
			ma_sound_start(&musicaTocadiscos); // Reproducir música
		}
		else {
			ma_sound_stop(&musicaTocadiscos);  // Pausar música
		}
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		esDeDia = !esDeDia; // Cambia entre Día y Noche
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}