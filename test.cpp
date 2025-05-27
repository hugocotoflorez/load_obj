#include <glad/glad.h> // have to be included before glfw3.h

#include "display_obj.h"
#include "load_mtl.h"
#include "load_obj.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "setShaders.h"
#include <GL/gl.h> // also included in glad
#include <GLFW/glfw3.h> // also included in glad
#include <assert.h>
#include <stdio.h> // fuck u iostream
#include <strings.h>
#include <vector>

#define BG_COLOR .9f, .9f, .9f, 1.0f
#define WIDTH 640
#define HEIGHT 480

/* Global to made easy share it among funtions */
GLuint shader_program;

std::vector<lObject> objects;

/* --- CHATGPT moment --- */

// Matriz de proyección en perspectiva
glm::mat4 projection =
glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);

// Matriz de vista (coloca la cámara en (0,0,3) mirando al origen)
glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), // Posición de la cámara
                             glm::vec3(0.0f, 0.0f, 0.0f), // A dónde mira
                             glm::vec3(0.0f, 1.0f, 0.0f) // Vector "arriba"
);

float cameraZ = 3.0f; // Distancia inicial de la cámara
float rotationAngle_Y = 0.0f;
float rotationAngle_X = 0.0f;

/* --- END --- */

int mainloop(GLFWwindow *window);

/* This funtion is going to be executed in main loop
 * to catch input and do whatever is needed */
void
__process_input(GLFWwindow *window)
{
        /* TODO: I think this shit do not work in linux. Idk
         * why GLFW_KEY_ESCAPE is 256 as this is del not esc */
        // check if escape is pressed
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);


        // Acercar la cámara con W
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
                cameraZ -= 0.1f;

        // Alejar la cámara con S
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
                cameraZ += 0.1f;

        // Acercar la cámara con W
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                rotationAngle_X -= 2.0f;

        // Alejar la cámara con S
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                rotationAngle_X += 2.0f;

        // Rotar a la izquierda con A
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                rotationAngle_Y -= 2.0f; // Ajusta la velocidad de rotación

        // Rotar a la derecha con D
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                rotationAngle_Y += 2.0f;

#define DISPLAY_ONLY_OBJECT_N(n)                                                        \
        if (glfwGetKey(window, GLFW_KEY_##n + 1) == GLFW_PRESS && objects.size() > n) { \
                for (int i = 0; i < objects.size(); i++) {                              \
                        if (i == n)                                                     \
                                objects.at(i).no_display = 0;                           \
                        else                                                            \
                                objects.at(i).no_display = 1;                           \
                }                                                                       \
        }

        DISPLAY_ONLY_OBJECT_N(0);
        DISPLAY_ONLY_OBJECT_N(1);
        DISPLAY_ONLY_OBJECT_N(2);
        DISPLAY_ONLY_OBJECT_N(3);
        DISPLAY_ONLY_OBJECT_N(4);
        DISPLAY_ONLY_OBJECT_N(5);
        DISPLAY_ONLY_OBJECT_N(6);
        DISPLAY_ONLY_OBJECT_N(7);
        DISPLAY_ONLY_OBJECT_N(8);
        DISPLAY_ONLY_OBJECT_N(9);

        // RESET
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
                for (int i = 0; i < objects.size(); i++) {
                        objects.at(i).no_display = 0;
                }
        }
}

/* This function is called on window resize */
void
__framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
}

/* Entry point: Initialize stuff and then enter mainloop. */
int
main(int argc, char **argv)
{
        /* ---- Init GLFW ---- */
        if (!glfwInit()) {
                printf("Can not init glfw\n");
                return 1;
        }
        // just info that I dont know if it is needed
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        // TODO: Creo que me descarque el otro
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* ---- Create the main window ---- */
        GLFWmonitor *monitor = glfwGetPrimaryMonitor(); // fullscreen
        // GLFWmonitor *monitor = NULL; // floating (or not)
        //  Share = NULL
        GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Titulo", monitor, NULL);

        if (window == NULL) {
                perror("glfwCreateWindow");
                glfwTerminate(); // terminate initialized glfw
                return 1;
        }

        /* @brief Makes the context of the specified window
         * current for the calling thread.
         * This function makes the OpenGL or OpenGL ES
         * context of the specified window current on the
         * calling thread. It can also detach the current
         * context from the calling thread without making a
         * new one current by passing in `NULL`. */
        glfwMakeContextCurrent(window);

        /* Call __framebuffer_size_callback() on window resize */
        glfwSetFramebufferSizeCallback(window, __framebuffer_size_callback);


        /* Load the GLAD. IDK what is this */
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
                perror("gladLoadGLLoader");
                /* In main1_33a.cpp it just return but I think
                 * it is needed to call glfwTerminate(). */
                return 1;
        }

        // z-buffer value
        glClearDepth(1.0f);
        // clear buffer color
        glClearColor(BG_COLOR);
        // enable z-buffer
        glEnable(GL_DEPTH_TEST);
        // hide back faces
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // do not hide
        // glDisable(GL_CULL_FACE);

        /* Program shader */
        shader_program =
        setShaders("vertex_shader.glsl", "fragment_shader.glsl");


        /* Sets the polygon rasterization mode for rendering.
         * This determines how OpenGL draws polygons.
         *
         * Options for `face`:
         * - `GL_FRONT`: Applies the mode to front-facing polygons only.
         * - `GL_BACK`: Applies the mode to back-facing polygons only.
         * - `GL_FRONT_AND_BACK`: Applies the mode to both front and back faces.
         *
         * Options for `mode`:
         * - `GL_POINT`: Renders only the vertices as points.
         * - `GL_LINE`: Renders only the edges as wireframes.
         * - `GL_FILL`: Renders solid, filled polygons (default).  */
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        if (argc == 2)
                objects = load_obj(argv[1], LOAD_3_3);
        else {
                printf("Usage %s <file.obj>\n", argv[0]);
                objects = load_obj("./source/Boat.obj", LOAD_3_3);
                // exit(0);
        }


        printf(".obj loaded\n");

        printf("%zu vaos loaded\n", objects.size());

        for (lObject &obj : objects)
                obj.shader = shader_program;


        assert(objects.size() > 0);

        mainloop(window);


        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
}

/* Main loop. Executed until program is closed manually. */
int
mainloop(GLFWwindow *window)
{
        /* Execute until window is closed */
        while (!glfwWindowShouldClose(window)) {
                // Call our process input function
                __process_input(window);

                // Clear buffers
                glClearColor(BG_COLOR); // BG
                glClear(GL_COLOR_BUFFER_BIT);
                glClear(GL_DEPTH_BUFFER_BIT);

                // Use shader program
                glUseProgram(shader_program);
                view = glm::lookAt(glm::vec3(0.0f, 0.0f, cameraZ), // Posición de la cámara
                                   glm::vec3(0.0f, 0.0f, 0.0f), // A dónde mira
                                   glm::vec3(0.0f, 1.0f, 0.0f)); // Vector "arriba"

                glm::mat4 model = glm::mat4(1.0f); // Matriz identidad
                model = glm::rotate(model, glm::radians(rotationAngle_Y),
                                    glm::vec3(0.0f, 1.0f, 0.0f)); // Rotar en eje Y
                model = glm::rotate(model, glm::radians(rotationAngle_X),
                                    glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar en eje X

                for (lObject obj : objects) {
                        obj_set_mats(&obj, view, model, projection);
                        display_obj(obj);
                }

                glfwSwapBuffers(window);
                glfwPollEvents();
        }

        for (lObject &obj : objects) {
                glDeleteVertexArrays(1, &obj.vao);
        }

        return 0;
}
