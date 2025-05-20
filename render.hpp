#ifndef RENDER_HPP
#define RENDER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "camera.h"

// Forward declarations des classes utilisées
class Camera;
class Light;



// Structure Mesh
struct Mesh {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int VBONorm;
    //unsigned int VBOText;
    unsigned int texture;
    std::vector<float> vertices;
    std::vector<float> normales;
    //std::vector<float> text;
    glm::mat4 model;

    glm::vec4 color;
    float ambianteLightMult;
    int shininess;
    bool enableTexture;
};

enum LightType {
    DIRECTIONAL, // 0
    POINT        // 1
};

struct Light {
    LightType type;
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float distance;
    float intensity;

    int castshadow;
    unsigned int depthMapFBO;
    unsigned int depthMap;
    unsigned int shadowWidth, shadowHeight;
    float near_plane, far_plane;
    float width;//ortho
    float fov;//perspective
    float aspectRatio;//perspective
    unsigned int PCFSize;
};

// Variables globales externes
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;
extern GLFWwindow* window;
extern Camera camera;
extern Mesh* tempMesh;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Fonctions d'initialisation
int InitGLFW(GLFWwindow*& window, const char* nom);
void SetupRender(const char* nom, Camera* cam);
void terminateRender();

// Fonctions de rendu
void renderScene();
void renderSkybox(unsigned int shader, unsigned int VAO, unsigned int cubemapTexture);

// Gestion des entrées
void processInput(GLFWwindow* window);

// Utilitaires
char* charger_shader(const char* filePath);
unsigned int ShaderLoader(const char* VertexShader, const char* FragmentShader);
unsigned int loadCubemap(std::vector<std::string> faces);
std::vector<float> addNormals(const std::vector<float>& verts);

// Setup des objets
void setMeshTextureFile(Mesh* mesh,const char* path);
Mesh* setupMesh(std::vector<float> vertices, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f));
Mesh* setupMeshTexture(std::vector<float> vertices, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f));
void deleteMesh(Mesh* mesh);
void updateMeshTexture(Mesh* mesh, std::vector<float> vertices);
void updateMesh(Mesh* mesh, std::vector<float> vertices);
void setMeshPosition(Mesh* mesh, const glm::vec3& position);
void setMeshColor(Mesh* mesh, const glm::vec4& color);
void setMeshShininess(Mesh* mesh, int shininess);
void setMeshAmbianteLightMult(Mesh* mesh, float ambianteLightMult);
unsigned int setupSkyboxVAO();
unsigned int setupLightVAO();

//setup des lights
Light* createLight(
    LightType type = POINT,
    bool castShadow = false,
    const glm::vec3& position = glm::vec3(4.0f, 5.0f, 1.0f),
    const glm::vec3& direction = glm::vec3(2.0f, 5.0f, 1.0f),
    const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
    float intensity = 1.0f,
    float distance = -1.0f
);
void setLightColor(Light* light, const glm::vec3& color);
void setLightPosition(Light* light, const glm::vec3& position);
void setLightDirection(Light* light, const glm::vec3& direction);
void setLightDistance(Light* light, float distance);
void setLightCastShadow(Light* light, bool castShadow);
void setLightIntensity(Light* light, float intensity);

void setupLightShadow(Light* l);
glm::mat4 getLightSpaceMatrix(Light* l);


// Gestion de la fenêtre
bool shouldCloseTheApp();

std::vector<float> computeNormals(const std::vector<float>& verts);
std::vector<float> computeNormalsTexture(const std::vector<float>& verts);
void calculerEtAfficherMoyenneFPS(float fps, int tailleMax);

GLFWwindow* getwindow();
Camera getCamera();
#endif // RENDER_HPP