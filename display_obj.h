#ifndef DISPLAY_OBJ_H
#define DISPLAY_OBJ_H

#include "load_obj.h"
#include <glm/glm.hpp>

void
obj_set_mats(lObject *obj, glm::mat4 view = glm::mat4(1),
             glm::mat4 model = glm::mat4(1), glm::mat4 projection = glm::mat4(1));

void display_obj(lObject obj);

#endif // !DISPLAY_OBJ_H
