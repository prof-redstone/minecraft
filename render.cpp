#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "render.hpp"

std::vector<std::string> SkyBoxFaces{
    "sources/skybox/right.jpg",
    "sources/skybox/left.jpg",
    "sources/skybox/top.jpg",
    "sources/skybox/bottom.jpg",
    "sources/skybox/front.jpg",
    "sources/skybox/back.jpg"
};

const char* shaderPathSkyBoxVert = "sources/shaders/SkyboxVertex.glsl";
const char* shaderPathSkyBoxFrag = "sources/shaders/SkyboxFrag.glsl";
const char* shaderPathLightVert = "sources/shaders/LightVertex.glsl";
const char* shaderPathLightFrag = "sources/shaders/LightFrag.glsl";
const char* shaderPathMainVert = "sources/shaders/MainVertex.glsl";
const char* shaderPathMainFrag = "sources/shaders/MainFrag.glsl";
const char* shaderPathDepthVert = "sources/shaders/DepthShadowVertex.glsl";
const char* shaderPathDepthFrag = "sources/shaders/DepthShadowFrag.glsl";

GLFWwindow* window = nullptr;
extern Camera camera;


static unsigned int SCR_WIDTH = 1000;
static unsigned int SCR_HEIGHT = 800;
static float lastX = SCR_WIDTH / 2.0f;
static float lastY = SCR_HEIGHT / 2.0f;
static bool firstMouse = true;//set mouse coordinate to origine on first click
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static int frameCounter = 0;
static float gamma = 1.0;
static unsigned int shaderSkybox;
static unsigned int shaderLight;
static unsigned int shaderProgram;
static unsigned int shaderProgramDepth;
static unsigned int VAO_SKY;
static unsigned int VAO_LIGHT;
static unsigned int cubemapTexture;
static std::vector<Mesh*> opaqueMeshList;
static std::vector<Mesh*> transpMeshList;
static std::vector<Light*> lightList;

static glm::vec3 blockTargetPos = glm::vec3(0.0,0.0,0.0);


void SetupRender(const char * nom, Camera* cam) {
    if (InitGLFW(window, nom) == -1) std::cout << "Erreur in init GLFW" << std::endl;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(0);


    shaderSkybox = ShaderLoader(shaderPathSkyBoxVert, shaderPathSkyBoxFrag);
    shaderLight = ShaderLoader(shaderPathLightVert, shaderPathLightFrag);
    shaderProgram = ShaderLoader(shaderPathMainVert, shaderPathMainFrag);
    shaderProgramDepth = ShaderLoader(shaderPathDepthVert, shaderPathDepthFrag);

    cubemapTexture = loadCubemap(SkyBoxFaces);

    VAO_SKY = setupSkyboxVAO();
    VAO_LIGHT = setupLightVAO();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    camera = *cam;
}

void terminateRender() {
    for (Mesh* mesh : opaqueMeshList) {
        if (mesh != nullptr) {
            glDeleteVertexArrays(1, &mesh->VAO);
            glDeleteBuffers(1, &mesh->VBO);
            GLuint vboNorm = 0; 
            glDeleteBuffers(1, &vboNorm);
            delete mesh;
            mesh = nullptr; 
        }
    }
    opaqueMeshList.clear();
    for (Mesh* mesh : transpMeshList) {
        if (mesh != nullptr) {
            glDeleteVertexArrays(1, &mesh->VAO);
            glDeleteBuffers(1, &mesh->VBO);
            GLuint vboNorm = 0;
            glDeleteBuffers(1, &vboNorm);
            delete mesh;
            mesh = nullptr;
        }
    }
    transpMeshList.clear();

    glfwTerminate();
}


void setMeshTextureFile(Mesh* mesh, const char* path) {
    const int pixelArtMaxSize = 64;
    glGenTextures(1, &(mesh->texture));
    glBindTexture(GL_TEXTURE_2D, (mesh->texture));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4); //force le 4 channels
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        bool isPixelArt = (width <= pixelArtMaxSize && height <= pixelArtMaxSize);
        if (isPixelArt) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }else {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        mesh->enableTexture = true;
        //std::cout << path << " loaded with " << nrChannels << " channels" << std::endl;
    }
    else {
        std::cout << "Failed to load texture : " << path << std::endl;
    }
    stbi_image_free(data);
}




Mesh* setupOpaqueMeshTexture(std::vector<float> vertices, const glm::vec3& position) {
    Mesh* mesh = new Mesh();
    mesh->vertices = vertices;
    glGenVertexArrays(1, &mesh->VAO);
    glBindVertexArray(mesh->VAO);

    glGenBuffers(1, &mesh->VBO);//generate VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);//VBO actif
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);

    mesh->normales = computeNormalsTexture(vertices);

    glGenBuffers(1, &mesh->VBONorm);//generate VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBONorm);//VBO actif
    glBufferData(GL_ARRAY_BUFFER, mesh->normales.size() * sizeof(float), mesh->normales.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    mesh->model = glm::mat4(glm::translate(glm::mat4(1.0f), position));
    mesh->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    mesh->shininess = 32;
    mesh->ambianteLightMult = 0.8;
    mesh->enableTexture = true;
    opaqueMeshList.push_back(mesh);
    return mesh;
}

Mesh* setupTranspMeshTexture(std::vector<float> vertices, const glm::vec3& position) {
    Mesh* mesh = new Mesh();
    mesh->vertices = vertices;
    glGenVertexArrays(1, &mesh->VAO);
    glBindVertexArray(mesh->VAO);

    glGenBuffers(1, &mesh->VBO);//generate VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);//VBO actif
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    mesh->normales = computeNormalsTexture(vertices);

    glGenBuffers(1, &mesh->VBONorm);//generate VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBONorm);//VBO actif
    glBufferData(GL_ARRAY_BUFFER, mesh->normales.size() * sizeof(float), mesh->normales.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    mesh->model = glm::mat4(glm::translate(glm::mat4(1.0f), position));
    mesh->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    mesh->shininess = 32;
    mesh->ambianteLightMult = 0.8;
    mesh->enableTexture = true;
    transpMeshList.push_back(mesh);
    return mesh;
}

void deleteMesh(Mesh* mesh) {
    if (mesh == nullptr) {
        return;
    }

    auto it = std::find(opaqueMeshList.begin(), opaqueMeshList.end(), mesh);
    if (it != opaqueMeshList.end()) {
        opaqueMeshList.erase(it);
    }
    it = std::find(transpMeshList.begin(), transpMeshList.end(), mesh);
    if (it != transpMeshList.end()) {
        transpMeshList.erase(it);
    }

    glDeleteVertexArrays(1, &(mesh->VAO));
    glDeleteBuffers(1, &(mesh->VBO));
    glDeleteBuffers(1, &(mesh->VBONorm));

    if (mesh->enableTexture) {
        glDeleteTextures(1, &(mesh->texture));
    }

    mesh->vertices.clear();
    mesh->vertices.shrink_to_fit();
    mesh->normales.clear();
    mesh->normales.shrink_to_fit();

    delete mesh;
}



void updateMeshTexture(Mesh* mesh, std::vector<float> vertices) {
    mesh->vertices = vertices;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);//VBO actif
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    mesh->normales = computeNormalsTexture(vertices);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBONorm);
    glBufferData(GL_ARRAY_BUFFER, mesh->normales.size() * sizeof(float), mesh->normales.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void updateMesh(Mesh* mesh, std::vector<float> vertices) {
    mesh->vertices = vertices;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    mesh->normales = computeNormals(vertices);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBONorm);
    glBufferData(GL_ARRAY_BUFFER, mesh->normales.size() * sizeof(float), mesh->normales.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void setMeshPosition(Mesh* mesh,const glm::vec3& position) {
    if (mesh == nullptr) return;
    mesh->model = glm::mat4(glm::translate(glm::mat4(1.0f), position));
}

void setMeshColor(Mesh* mesh, const glm::vec4& color) {
    if (mesh == nullptr) return;
        mesh->color = color;
}

void setMeshShininess(Mesh* mesh, int shininess) {
    if (mesh == nullptr) return;
    mesh->shininess = shininess;
}

void setMeshAmbianteLightMult(Mesh* mesh, float ambianteLightMult) {
    if (mesh == nullptr) return;
    mesh->ambianteLightMult = ambianteLightMult;
}



Light* createLight(
    LightType type,
    bool castShadow,
    const glm::vec3& position,
    const glm::vec3& direction,
    const glm::vec3& color,
    float intensity,
    float distance
) {
    Light* light = new Light();
    light->type = type;
    light->position = position;
    light->direction = direction;
    light->color = color;
    light->intensity = intensity;
    light->ambient = color*intensity;
    light->diffuse = color * intensity;
    light->specular = color * intensity;
    light->castshadow = castShadow;
    light->distance = distance;

    if (light->type == POINT) {
        light->near_plane = 0.1f;
        light->far_plane = 25.0f;
        light->shadowWidth = 2048;
        light->shadowHeight = 2048;
        light->PCFSize = 1;
        light->aspectRatio = 1.0f;
        light->fov = glm::radians(90.0f);
    }
    else {
        light->near_plane = -50.0;
        light->far_plane = 50.0;
        light->shadowWidth = 2048;
        light->shadowHeight = 2048;
        light->PCFSize = 1;
        light->width = 25.0;
    }

    if (light->castshadow) {
        setupLightShadow(light);
    }

    lightList.push_back(light);
    return light;
}

void setLightColor(Light* light, const glm::vec3& color) {
    if (light == nullptr) return;
    light->color = color;
    light->ambient = color * light->intensity;
    light->diffuse = color * light->intensity;
    light->specular = color * light->intensity;
}

void setLightPosition(Light* light, const glm::vec3& position) {
    if (light == nullptr) return;
    light->position = position;
}

void setLightDirection(Light* light, const glm::vec3& direction) {
    if (light == nullptr) return;
    light->direction = direction;
}

void setLightDistance(Light* light, float distance) {
    if (light == nullptr) return;
    light->distance = distance;
}

void setLightCastShadow(Light* light, bool castShadow) {
    if (light == nullptr) return;
    if (castShadow && !light->castshadow) {
        light->castshadow = true;
        setupLightShadow(light);
    }else if (!castShadow && light->castshadow) {
        light->castshadow = false;
    }
}

void setLightIntensity(Light* light, float intensity) {
    if (light == nullptr) return;
    light->intensity = intensity;
    light->ambient = light->color * intensity;
    light->diffuse = light->color * intensity;
    light->specular = light->color * intensity;
}

void setupLightShadow(Light* l) {
    glGenFramebuffers(1, &(l->depthMapFBO));
    glGenTextures(1, &(l->depthMap));
    glBindTexture(GL_TEXTURE_2D, l->depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, l->shadowWidth, l->shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, l->depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, l->depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void renderScene() {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    //calculerEtAfficherMoyenneFPS(1.0 / deltaTime, 60);
    //std::cout <<  deltaTime << std::endl;


    processInput(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(0);


    //calculating shadow
    int parityFrameShadowRender = 1; //min 1, max =~ 2
    glUseProgram(shaderProgramDepth);
    //glCullFace(GL_FRONT);
    for (int i = 0; i < lightList.size(); i++) {
        if (lightList[i]->castshadow && (frameCounter % parityFrameShadowRender == 0)) {
            glUniformMatrix4fv(glGetUniformLocation(shaderProgramDepth, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(getLightSpaceMatrix(lightList[i])));
            glViewport(0, 0, (*lightList[i]).shadowWidth, (*lightList[i]).shadowHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, (*lightList[i]).depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (int i = 0; i < opaqueMeshList.size(); i++) {
                glUniformMatrix4fv(glGetUniformLocation(shaderProgramDepth, "model"), 1, GL_FALSE, glm::value_ptr((*opaqueMeshList[i]).model));
                glBindVertexArray((*opaqueMeshList[i]).VAO);
                glDrawArrays(GL_TRIANGLES, 0, (*opaqueMeshList[i]).vertices.size() / 3);
            }
            for (int i = 0; i < transpMeshList.size(); i++) {
                glUniformMatrix4fv(glGetUniformLocation(shaderProgramDepth, "model"), 1, GL_FALSE, glm::value_ptr((*transpMeshList[i]).model));
                glBindVertexArray((*transpMeshList[i]).VAO);
                glDrawArrays(GL_TRIANGLES, 0, (*transpMeshList[i]).vertices.size() / 3);
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glCullFace(GL_BACK);




    // render scene with shadow mapping
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glDepthMask(GL_FALSE); // Désactive l'écriture dans le depth buffer
    renderSkybox(shaderSkybox, VAO_SKY, cubemapTexture);
    glDepthMask(GL_TRUE);


    glUseProgram(shaderProgram);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 600.0f);
    glm::mat4 view = camera.GetViewMatrix();

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.Position));
    glUniform1f(glGetUniformLocation(shaderProgram, "gamma"), gamma);

    int shadowIndex = 0;
    for (int i = 0; i < lightList.size(); i++) {
        std::string base = "lights[" + std::to_string(i) + "]";
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".position").c_str()), 1, glm::value_ptr((*lightList[i]).position));
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".direction").c_str()), 1, glm::value_ptr((*lightList[i]).direction));
        glUniform1i(glGetUniformLocation(shaderProgram, (base + ".lightType").c_str()), ((*lightList[i]).type == POINT) ? 1 : 0);
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".ambient").c_str()), 1, glm::value_ptr((*lightList[i]).ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".diffuse").c_str()), 1, glm::value_ptr((*lightList[i]).diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".specular").c_str()), 1, glm::value_ptr((*lightList[i]).specular));
        glUniform1f(glGetUniformLocation(shaderProgram, (base + ".distance").c_str()), (*lightList[i]).distance);
        glUniform1i(glGetUniformLocation(shaderProgram, (base + ".castshadow").c_str()), (*lightList[i]).castshadow ? 1 : 0);


        if ((*lightList[i]).castshadow) {
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, (base + ".lightSpaceMatrix").c_str()), 1, GL_FALSE, glm::value_ptr( getLightSpaceMatrix(lightList[i]) ));
            glUniform1i(glGetUniformLocation(shaderProgram, (base + ".PCFSize").c_str()), (*lightList[i]).PCFSize);

            std::string shadowSampler = "shadowMaps[" + std::to_string(shadowIndex) + "]";
            glActiveTexture(GL_TEXTURE0 + shadowIndex);
            glBindTexture(GL_TEXTURE_2D, (*lightList[i]).depthMap);
            glUniform1i(glGetUniformLocation(shaderProgram, shadowSampler.c_str()), shadowIndex);

            glUniform1i(glGetUniformLocation(shaderProgram, (base + ".shadowIndex").c_str()), shadowIndex);
            shadowIndex++;
        }else {
            glUniform1i(glGetUniformLocation(shaderProgram, (base + ".shadowIndex").c_str()), -1);
        }
    }
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), lightList.size());

    const int TextureIndex = shadowIndex;
    for (int i = 0; i < opaqueMeshList.size(); i++) {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr((*opaqueMeshList[i]).model));
        glUniform4fv(glGetUniformLocation(shaderProgram, "mat.color"), 1, glm::value_ptr((*opaqueMeshList[i]).color));
        glUniform1i(glGetUniformLocation(shaderProgram, "mat.shininess"), (*opaqueMeshList[i]).shininess);
        glUniform1f(glGetUniformLocation(shaderProgram, "mat.ambianteLightMult"), (*opaqueMeshList[i]).ambianteLightMult);
        glUniform1i(glGetUniformLocation(shaderProgram, "mat.enableTexture"), 0);
        if ((*opaqueMeshList[i]).enableTexture) {
            glActiveTexture(GL_TEXTURE0 + TextureIndex);
            glBindTexture(GL_TEXTURE_2D, (*opaqueMeshList[i]).texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "mat.text"), TextureIndex);
            glUniform1i(glGetUniformLocation(shaderProgram, "mat.enableTexture"), 1);
        }
        glBindVertexArray((*opaqueMeshList[i]).VAO);
        glDrawArrays(GL_TRIANGLES, 0, (*opaqueMeshList[i]).vertices.size() / 3);
    }
    //glDepthMask(GL_FALSE); 
    for (int i = 0; i < transpMeshList.size(); i++) {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr((*transpMeshList[i]).model));
        glUniform4fv(glGetUniformLocation(shaderProgram, "mat.color"), 1, glm::value_ptr((*transpMeshList[i]).color));
        glUniform1i(glGetUniformLocation(shaderProgram, "mat.shininess"), (*transpMeshList[i]).shininess);
        glUniform1f(glGetUniformLocation(shaderProgram, "mat.ambianteLightMult"), (*transpMeshList[i]).ambianteLightMult);
        glUniform1i(glGetUniformLocation(shaderProgram, "mat.enableTexture"), 0);
        if ((*transpMeshList[i]).enableTexture) {
            glActiveTexture(GL_TEXTURE0 + TextureIndex);
            glBindTexture(GL_TEXTURE_2D, (*transpMeshList[i]).texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "mat.text"), TextureIndex);
            glUniform1i(glGetUniformLocation(shaderProgram, "mat.enableTexture"), 1);
        }
        glBindVertexArray((*transpMeshList[i]).VAO);
        glDrawArrays(GL_TRIANGLES, 0, (*transpMeshList[i]).vertices.size() / 3);
    }
    //glDepthMask(GL_TRUE);

    


    //---BOX LIGHT---
    glUseProgram(shaderLight);
    for (int i = 0; i < lightList.size(); i++) {
        glm::vec3 boxPos = ((*lightList[i]).type == POINT) ? (*lightList[i]).position : camera.Position + glm::float32(4.0)*glm::normalize((*lightList[i]).direction);
        glm::mat4 modelLight = glm::mat4(glm::translate(glm::mat4(1.0f), boxPos));
        modelLight = glm::scale(modelLight, glm::vec3(0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderLight, "model"), 1, GL_FALSE, glm::value_ptr(modelLight));
        glUniformMatrix4fv(glGetUniformLocation(shaderLight, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderLight, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(shaderLight, "color"), 1, glm::value_ptr(glm::vec4((*lightList[i]).color, 0.1f)));

        glBindVertexArray(VAO_LIGHT);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    //render bock target : //re use same thing as light
    //glUseProgram(shaderLight);
    glUniformMatrix4fv(glGetUniformLocation(shaderLight, "model"), 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(glm::translate(glm::mat4(1.0f), blockTargetPos + glm::vec3(0.5, 0.5, 0.5))), glm::vec3(0.505f))));
    glUniformMatrix4fv(glGetUniformLocation(shaderLight, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderLight, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(shaderLight, "color"), 1, glm::value_ptr(glm::vec4(1.0,1.0,1.0, 0.18f)));
    glBindVertexArray(VAO_LIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    glm::vec3 boxPos = camera.Position + camera.Front * glm::float32(0.1);

    // Version alternative avec lookAt (plus simple)
    glm::mat4 billboardMatrix = glm::inverse(glm::lookAt(boxPos, camera.Position, camera.Up));
    glm::mat4 modelMatrix = billboardMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(0.0005f));


    glUniformMatrix4fv(glGetUniformLocation(shaderLight, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderLight, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderLight, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(shaderLight, "color"), 1, glm::value_ptr(glm::vec4(1.0, 1.0, 1.0, 1.0f)));
    glBindVertexArray(VAO_LIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    glfwSwapBuffers(window);
    glfwPollEvents();
    frameCounter++;
}

glm::mat4 getLightSpaceMatrix(Light* l) {

    glm::mat4 lightProjection;
    glm::mat4 lightView;

    if (l->type == DIRECTIONAL) {
        lightProjection = glm::ortho(-l->width, l->width, -l->width, l->width, l->near_plane, l->far_plane);
        lightView = glm::lookAt(camera.Position, camera.Position - l->direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else {
        lightProjection = glm::perspective(l->fov, l->aspectRatio, l->near_plane, l->far_plane);
        lightView = glm::lookAt(l->position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    return lightProjection * lightView;
}

unsigned int setupLightVAO() {
    float LightVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f
    };

    unsigned int VBO_LIGHT, VAO_LIGHT;
    glGenVertexArrays(1, &VAO_LIGHT);
    glGenBuffers(1, &VBO_LIGHT);

    glBindVertexArray(VAO_LIGHT);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_LIGHT);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LightVertices), LightVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return VAO_LIGHT;
}

unsigned int setupSkyboxVAO() {
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f
    };

    unsigned int VBO_SKY, VAO_SKY;
    glGenVertexArrays(1, &VAO_SKY);
    glGenBuffers(1, &VBO_SKY);

    glBindVertexArray(VAO_SKY);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_SKY);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return VAO_SKY;
}

void renderSkybox(unsigned int shader, unsigned int VAO, unsigned int cubemapTexture) {
    glDepthFunc(GL_LEQUAL);  // change la fonction de profondeur pour que la skybox passe le test quand z=1.0
    glUseProgram(shader);
    glm::mat4 view_skybox = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Supprime la translation pour que la skybox ne bouge pas avec la cam�ra
    glm::mat4 projection_skybox = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view_skybox));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection_skybox));

    // Lier la texture du cubemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS); //fonction de profondeur par defaut

}

std::vector<float> addNormals(const std::vector<float>& verts) {
    std::vector<float> interleaved;
    interleaved.reserve(verts.size() / 3 * 6); // 3 pos + 3 normal per vertex

    for (size_t i = 0; i < verts.size(); i += 9) {
        glm::vec3 v0(verts[i], verts[i + 1], verts[i + 2]);
        glm::vec3 v1(verts[i + 3], verts[i + 4], verts[i + 5]);
        glm::vec3 v2(verts[i + 6], verts[i + 7], verts[i + 8]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // Append each vertex followed by the normal
        for (int j = 0; j < 3; ++j) {
            glm::vec3 v(verts[i + j * 3], verts[i + j * 3 + 1], verts[i + j * 3 + 2]);
            interleaved.push_back(v.x);
            interleaved.push_back(v.y);
            interleaved.push_back(v.z);

            interleaved.push_back(normal.x);
            interleaved.push_back(normal.y);
            interleaved.push_back(normal.z);
        }
    }

    return interleaved;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }else{
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

int InitGLFW(GLFWwindow*& window, const char * nom) {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, nom, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur lors de l'initialisation de GLEW" << std::endl;
        return -1;
    }

    return 0;
}

void processInput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);


    static bool leftWasPressed = false;
    static bool rightWasPressed = false;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!leftWasPressed) {
            leftWasPressed = true;
            camera.leftMousePressed = true;
        }
    }
    else {
        if (leftWasPressed) {
            leftWasPressed = false;
            camera.leftMousePressed = false;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!rightWasPressed) {
            rightWasPressed = true;
            camera.rightMousePressed = true;
        }
    }
    else {
        if (rightWasPressed) {
            rightWasPressed = false;
            camera.rightMousePressed = false;
        }
    }
    camera.key1Pressed = (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);
    camera.key2Pressed = (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);
    camera.key3Pressed = (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS);
    camera.key4Pressed = (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS);
    camera.key5Pressed = (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS);
    camera.key6Pressed = (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS);
    camera.key7Pressed = (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS);
    camera.key8Pressed = (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS);
    camera.key9Pressed = (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

char* charger_shader(const char* filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier shader " << filePath << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    std::string shaderCode = buffer.str();

    char* shaderSource = new char[shaderCode.size() + 1];
    std::copy(shaderCode.begin(), shaderCode.end(), shaderSource);
    shaderSource[shaderCode.size()] = '\0'; // Terminaison avec '\0'

    return shaderSource;
}

unsigned int ShaderLoader(const char* VertexShader, const char* FragmentShader) {
    char infoLog[512];
    int success;

    // --- VERTEX SHADER ---
    char* vertexShaderSource = charger_shader(VertexShader);
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Erreur compilation VERTEX shader (" << VertexShader << "):\n" << infoLog << std::endl;
    }

    // --- FRAGMENT SHADER ---
    char* fragmentShaderSource = charger_shader(FragmentShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Erreur compilation FRAGMENT shader (" << FragmentShader << "):\n" << infoLog << std::endl;
    }

    // --- SHADER PROGRAM ---
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Erreur linkage du shader program:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


bool shouldCloseTheApp() {
    return !glfwWindowShouldClose(window);
}

std::vector<float> computeNormals(const std::vector<float>& verts) {
    std::vector<float> normals(verts.size(), 0.0f);

    for (size_t i = 0; i < verts.size(); i += 9) {
        glm::vec3 v0(verts[i], verts[i + 1], verts[i + 2]);
        glm::vec3 v1(verts[i + 3], verts[i + 4], verts[i + 5]);
        glm::vec3 v2(verts[i + 6], verts[i + 7], verts[i + 8]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        for (int j = 0; j < 3; ++j) {
            normals[i + j * 3 + 0] = normal.x;
            normals[i + j * 3 + 1] = normal.y;
            normals[i + j * 3 + 2] = normal.z;
        }
    }

    return normals;
}

std::vector<float> computeNormalsTexture(const std::vector<float>& verts) {
    std::vector<float> normals(verts.size()*3/5, 0.0f);

    for (size_t i = 0; i < verts.size(); i += 15) {
        glm::vec3 v0(verts[i], verts[i + 1], verts[i + 2]);
        glm::vec3 v1(verts[i + 3 + 2], verts[i + 4 + 2], verts[i + 5 + 2]);
        glm::vec3 v2(verts[i + 6 + 4], verts[i + 7 + 4], verts[i + 8 + 4]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        for (int j = 0; j < 3; ++j) {
            normals[i * 9 / 15 + j * 3 + 0] = normal.x;
            normals[i * 9 / 15 + j * 3 + 1] = normal.y;
            normals[i * 9 / 15 + j * 3 + 2] = normal.z;
        }
    }

    return normals;
}

void calculerEtAfficherMoyenneFPS(float fps, int tailleMax = 60) {
    static std::vector<float> tableauFPS;

    tableauFPS.push_back(fps);

    if (tableauFPS.size() >= tailleMax) {
        float somme = 0.0f;
        for (const auto& valeur : tableauFPS) {
            somme += valeur;
        }
        std::cout << "FPS : " << static_cast<int>(somme / tableauFPS.size()) << std::endl;
        tableauFPS.clear();
    }
}

GLFWwindow* getwindow() {
    return window;
}

Camera getCamera() {
    return camera;
}

void updateBlockTarget(glm::vec3 Pos) {
    blockTargetPos = Pos;
}