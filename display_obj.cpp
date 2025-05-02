#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cassert>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#include "load_obj.h"

void
obj_set_mats(lObject *obj, glm::mat4 view = glm::mat4(1),
             glm::mat4 model = glm::mat4(1), glm::mat4 projection = glm::mat4(1))
{
        obj->view = view;
        obj->model = model;
        obj->projection = projection;
}

int current_texture_id = -1;
GLint currentShader = -1;

void
display_obj(lObject obj)
{
        if (obj.vao == 0)
                return;

        if (obj.no_display)
                return;

        if (currentShader != obj.shader) {
                // printf("Shader set to %d\n", obj.shader);
                glUseProgram(obj.shader);
                currentShader = obj.shader;
        }

        if (!glIsTexture(obj.material->texture)) {
                // printf("Texture %d is not bound\n",
                       // obj.material->texture);
        }

        if (obj.material->texture > 0) {
                // glActiveTexture(GL_TEXTURE0);
        }

        GLuint viewLoc = glGetUniformLocation(obj.shader, "view");
        GLuint projectionLoc = glGetUniformLocation(obj.shader, "projection");
        GLuint modelLoc = glGetUniformLocation(obj.shader, "model");
        GLuint textureLoc = glGetUniformLocation(obj.shader, "texture1");

        // if (viewLoc == -1)
        //         printf("[!] Uniform 'view' no encontrado en el shader!\n");
        // if (projectionLoc == -1)
        //         printf("[!] Uniform 'projection' no encontrado en el shader!\n");
        // if (modelLoc == -1)
        //         printf("[!] Uniform 'model' no encontrado en el shader!\n");
        // if (textureLoc == -1)
        // printf("[!] Uniform 'texture1' no encontrado en el shader!\n");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(obj.view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(obj.projection));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
        glUniform1i(textureLoc, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj.material->texture);

        GLint currentTexture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
        if (currentTexture != obj.material->texture) {
                // printf("[!] Textura incorrecta! Esperado: %d, Actual: %d\n", obj.material->texture, currentTexture);
        }

        glBindVertexArray(obj.vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, obj.index_n, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0); // Desvincula la textura
}
