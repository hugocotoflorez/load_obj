/* +-----------------------------------------------------+ *
 * | Author: Hugo Coto Florez                            | *
 * | Contact: hugo.coto@rai.usc.es                       | *
 * | Repo: github.com/hugocotoflorez/load_obj            | *
 * | Licenseless: Free open source without any warranty. | *
 * +-----------------------------------------------------+ */

#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

#include "load_mtl.h"

/* QUIET possible values
 * 1: Supress any output
 * 0: Show loaded object info */

#define QUIET 0

/* DEBUG possible values.
 * 1: show info about loading process
 * 0: do not show debugging info */
#define DEBUG 0

#define SOURCE_PATH "assets/"

#include <glm/glm.hpp>

typedef struct lObject {
        unsigned int vao;
        unsigned int index_n;
        unsigned int shader;
        unsigned int no_display;
        glm::mat4 view;
        glm::mat4 model;
        glm::mat4 projection;
        lMaterial *material;
        char name[32];
} lObject;

enum __options
// no yet implemented
{
        LOAD_NOLOAD = 0, /* Do not load into VAO */
        LOAD_3_3 = 1, /* Load using opengl version 3.3 */
        LOAD_1_2 = 2, /* Load using opengl version 1.2 */
        LOAD_MTL = 3, /* Load textures (materials) */
};

#include <vector>

/* Load wavefront (.obj) file (filename) into objects. */
std::vector<lObject> load_obj(const char *filename, int options);

void link_textures();

#endif
