#include <iostream>
#include <random>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>
#include <gtc/type_ptr.hpp>

#include "Camera.h"
#include "Sphere.h"
#include "shader_c.h"
#include "shader_t.h"
#include "LoadTGA.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

GLfloat* createMasterHairs(const Sphere& object);
GLuint generateTextureFromHairData(GLfloat* hairData);

// Window dimensions
const GLuint WIDTH = 2000, HEIGHT = 1100;

// camera variables - give pretty starting point
Camera camera(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 180, 0);
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// time variables
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// Light variables
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos(0.f, 0.f, 0.f);

// Hair variables
int verticesPerStrand = 15; // if this changed , it should be changed in compute shader and in geometry shader too
float hairStrandLength = 0.005f;
const int dataVariablesPerMasterHair = 1; // position
int noOfMasterHairs;

float damping = 0;
float timeStep = 0.03;

float windAmount = 0.f;
float minWindAmount = 0.f;
float maxWindAmount = 1000.f;
float windMagnitude = 1.f;
glm::vec4 windDirection = {0.f, -1.f, 1.f, 0.f};
glm::vec4 windPosition = {0.f, 0.f, 0.f, 1.f};

glm::mat4 model=glm::mat4(1.0f);



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw  creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hair simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // set callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    glEnable(GL_MULTISAMPLE);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;

    // Initialize GLEW to setup the OpenGL Function pointers
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // create model
    // -----------------------------
    Sphere sphere(2.0, 40);

    TextureData mainTexture;
    LoadTGATexture("../textures/brown.tga", &mainTexture);
    // Hair
    // ---------------------------------------------------------------------------------

    // Creation of the master hairs
    GLfloat* hairData = createMasterHairs(sphere);

    // Generation of textures for hair data
    GLuint hairDataTextureID_rest = generateTextureFromHairData(hairData);
    GLuint hairDataTextureID_last = generateTextureFromHairData(hairData);
    GLuint hairDataTextureID_current = generateTextureFromHairData(hairData);
    GLuint hairDataTextureID_simulated = generateTextureFromHairData(NULL);


    // build and compile our shader program
    // ------------------------------------
    Shader hairShader("../shaders/Hair.vert","../shaders/Hair.frag", "../shaders/Hair.geom",
                               "../shaders/Hair.tesc", "../shaders/Hair.tese");
    Shader shader("../shaders/shader.vert","../shaders/shader.frag");
    ComputeShader computeShader("../shaders/HairSimulation.comp");
    shader.use();
    hairShader.use();
    computeShader.use();

    // uniform variables
    // be sure to activate shader when setting uniforms/drawing objects
    shader.use();
    shader.setInt("mainTexture", 0);

    hairShader.use();
    hairShader.setInt("mainTexture", 0);
    hairShader.setInt("hairDataTexture", 1);
    hairShader.setInt("randomDataTexture", 2);


    computeShader.use();
    float rotationAngle = 0.f;
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
    
        // simulation of hair
        // -------------------------------------------------------------------
        windMagnitude *= (pow(sin(currentFrame * 0.05), 2) + 0.5);

        computeShader.use();
        glBindImageTexture(0, hairDataTextureID_rest, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(1, hairDataTextureID_last, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(2, hairDataTextureID_current, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(3, hairDataTextureID_simulated, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        computeShader.setMat4("modelMatrix", model);
        computeShader.setFloat("damping", damping);
        computeShader.setFloat("timeStep", timeStep);
        computeShader.setInt("verticesPerStrand", verticesPerStrand);
        computeShader.setFloat("hairStrandLength", hairStrandLength);
        computeShader.setFloat("windMagnitude", windMagnitude + windAmount);
        computeShader.setVec4("windDirection", windDirection.x, windDirection.y, windDirection.z, windDirection.w);
        glDispatchCompute(1, noOfMasterHairs, 1); // Call for each master hair strand

        // rendering
        // -------------------------------------------------------------------

        // render object ( sphere or any other object)
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mainTexture.texID);
        sphere.draw(GL_TRIANGLES);

        // Wait until simulation is finished
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //render hair
        hairShader.use();
        hairShader.setMat4("model", model);
        hairShader.setMat4("projection", projection);
        hairShader.setMat4("view", view);
        hairShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
        hairShader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);
        hairShader.setVec3("cameraPosition", camera.Position.x, camera.Position.y, camera.Position.z);
        hairShader.setFloat("verticesPerStrand", (float)verticesPerStrand);
        hairShader.setFloat("noOfVertices", (float)noOfMasterHairs);
        hairShader.setFloat("dataVariablesPerMasterHair", (float)dataVariablesPerMasterHair);

        // Main texture (for color)
        glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, mainTexture.texID);

        // Hair data saved in texture
        glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
        glBindTexture(GL_TEXTURE_2D, hairDataTextureID_simulated);

        sphere.draw(GL_PATCHES);

        glCopyImageSubData(hairDataTextureID_current, GL_TEXTURE_2D, 0, 0, 0, 0,
                           hairDataTextureID_last, GL_TEXTURE_2D, 0, 0, 0, 0,
                           verticesPerStrand, noOfMasterHairs, 1);

        glCopyImageSubData(hairDataTextureID_simulated, GL_TEXTURE_2D, 0, 0, 0, 0,
                           hairDataTextureID_current, GL_TEXTURE_2D, 0, 0, 0, 0,
                           verticesPerStrand, noOfMasterHairs, 1);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{ 
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
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        model = glm::translate(model, glm::vec3(0.05f, 0.f, 0.f));
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        model = glm::translate(model, glm::vec3(-0.05f, 0.f, 0.f));
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        model = glm::translate(model, glm::vec3(0.f, -0.05f, 0.f));
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        model = glm::translate(model, glm::vec3(0.f, 0.05f, 0.f));
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            hairStrandLength += 0.005;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            hairStrandLength += 0.005;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        if(windAmount < maxWindAmount)
            windAmount += 10.f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        if(windAmount > minWindAmount)
            windAmount -= 10.f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glfwGetWindowSize( window, &width, &height );
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    //camera.ProcessMouseMovement(xoffset, yoffset);
    model = glm::translate(model, glm::vec3(0.f, yoffset*0.01f, -xoffset*0.01f));

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}



GLfloat* createMasterHairs(const Sphere& object){
    GLfloat* vertexArray = object.getVertexArray();
    noOfMasterHairs = object.getNoOfVertices();

    int amountOfDataPerMasterHair = verticesPerStrand * 4 * dataVariablesPerMasterHair;
    GLfloat* hairData = new GLfloat[noOfMasterHairs * amountOfDataPerMasterHair];

    int masterHairIndex = 0;
    int stride = 8; // 8 because the vertexArray consists of (vertex (3), normal (3), tex (2))
    for(int i = 0; i < noOfMasterHairs*stride; i = i+stride){
        glm::vec4 rootPos = glm::vec4(vertexArray[i], vertexArray[i+1], vertexArray[i+2], 1.f);
        rootPos = model * rootPos;
        glm::vec4 rootNormal = glm::vec4(vertexArray[i+3], vertexArray[i+4], vertexArray[i+5], 0.f);

        hairData[masterHairIndex++] = rootPos.x;
        hairData[masterHairIndex++] = rootPos.y;
        hairData[masterHairIndex++] = rootPos.z;
        hairData[masterHairIndex++] = rootPos.w;

        // Add hair vertices
        for(int vert = 0; vert < verticesPerStrand - 1; vert++){
            glm::vec4 newPos = rootPos + vert*hairStrandLength*rootNormal;
            hairData[masterHairIndex++] = newPos.x;
            hairData[masterHairIndex++] = newPos.y;
            hairData[masterHairIndex++] = newPos.z;
            hairData[masterHairIndex++] = newPos.w;
        }
    }
    return hairData;
}

GLuint generateTextureFromHairData(GLfloat* hairData){
    GLuint hairDataTextureID;
    glGenTextures(1, &hairDataTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hairDataTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, verticesPerStrand * dataVariablesPerMasterHair, noOfMasterHairs, 0, GL_RGBA, GL_FLOAT, hairData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    return hairDataTextureID;
}
