#include <glad/glad.h> // have to be included before glfw3.h

#include "load_obj.h"

#include "setShaders.h"
#include <GL/gl.h> // also included in glad
#include <GLFW/glfw3.h> // also included in glad
#include <assert.h>
#include <stdio.h> // fuck u iostream

#define BG_COLOR .0f, .0f, .0f, 1.0f
#define WIDTH 600
#define HEIGHT 480

/* Global to made easy share it among funtions */
GLuint shader_program;
unsigned int VAO;
unsigned int indexes_size;


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
main()
{
        /* ---- Init GLFW ---- */
        if (!glfwInit())
        {
                printf("Can not init glfw\n");
                return 1;
        }
        // just info that I dont know if it is needed
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        // TODO: Creo que me descarque el otro
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* ---- Create the main window ---- */
        // Monitor = NULL
        // Share = NULL
        GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Titulo", NULL, NULL);

        if (window == NULL)
        {
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
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        {
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
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        load_obj("square.obj", &VAO, &indexes_size, LOAD_3_3);
        printf("cube.obj loaded\n");

        return mainloop(window);
}

/* Main loop. Executed until program is closed manually. */
int
mainloop(GLFWwindow *window)
{
        /* Execute until window is closed */
        while (!glfwWindowShouldClose(window))
        {
                // Call our process input function
                __process_input(window);

                // Clear buffers
                glClearColor(BG_COLOR); // BG
                glClear(GL_COLOR_BUFFER_BIT);
                glClear(GL_DEPTH_BUFFER_BIT);

                // Use shader program
                glUseProgram(shader_program);

                /* Binds the specified Vertex Array Object
                 * (VAO).
                 * This ensures that subsequent vertex
                 * operations use the correct VAO
                 * configuration.
                 * Without this, OpenGL wouldn't know which
                 * vertex data to use. */
                glBindVertexArray(VAO);

                /* Renders primitives (lines, triangles, etc.)
                 * using vertex data in order.
                 * - `GL_LINES`: Draws lines, each pair of
                 *   vertices forms a line.
                 * - `0`: Starts from the first vertex in
                 *   the buffer.
                 * - `6`: Number of vertices to process
                 *   (draws 3 lines).
                 * Use this when you don't need indexed
                 * drawing. */
                // glDrawArrays(GL_LINES, 0, 6);
                // glDrawArrays(GL_TRIANGLES, 0, 6); // for square

                /* Renders primitives using indexed drawing
                 * with an Element Buffer Object (EBO).
                 * - `GL_LINES`: Draws lines using the
                 *   specified indices.
                 * - `6`: Number of indices to read from the
                 *   EBO.
                 * - `GL_UNSIGNED_INT`: Type of the indices
                 *   in the EBO.
                 * - `0`: Start at the beginning of the EBO.
                 * Use this when vertices are reused to
                 * optimize memory usage. */
                // glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);

                glDrawElements(GL_TRIANGLES, indexes_size, GL_UNSIGNED_INT, 0);

                /* Unbinds the currently active VAO.
                 * This prevents unintended modifications to
                 * the VAO.
                 * It's a good practice when working with
                 * multiple VAOs. */
                glBindVertexArray(0);

                /* @brief Swaps the front and back buffers of
                 * the specified window.
                 * This function swaps the front and back
                 * buffers of the specified window when
                 * rendering with OpenGL or OpenGL ES.
                 * If the swap interval is greater than zero,
                 * the GPU driver waits the specified number of
                 * screen updates before swapping the buffers. */
                /* ~~ load new frame */
                glfwSwapBuffers(window);

                /* @brief Processes all pending events.
                 * This function processes only those events
                 * that are already in the event queue and
                 * then returns immediately. Processing
                 * events will cause the window and input
                 * callbacks associated with those events
                 * to be called. */
                /* It does not block */
                glfwPollEvents();
        }

        /* Deletes the specified Vertex Array Object (VAO).
         * This frees the GPU memory associated with the VAO.
         * It's important to call this when the VAO is no
         * longer needed to avoid memory leaks.
         */
        glDeleteVertexArrays(1, &VAO);

        /* Deletes the specified Vertex Buffer Object (VBO).
         * This releases the allocated GPU memory for vertex
         * data. Deleting unused buffers
         * helps optimize resource usage.
         */
        // glDeleteBuffers(1, &VBO);

        glfwTerminate();
        return 0;
}
