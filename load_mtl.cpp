#include "load_mtl.h"
#include <cassert>
#include <cstdlib>

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <vector>

#define endwith(src, sufix) \
        (strcmp((src) + strlen((src)) - strlen((sufix)), (sufix)) == 0)

#define ZERO(obj_ptr) memset((obj_ptr), 0, sizeof(*obj_ptr));

lMaterial material;

GLuint
load_texture(lMaterial &mat)
{
        if (mat.texture > 0) {
                printf("Texture loaded yet\n");
                return mat.texture;
        }

        if (mat.image == NULL) {
                printf("Material has no image data!\n");
                return 0;
        }
        glGenTextures(1, &mat.texture);
        glBindTexture(GL_TEXTURE_2D, mat.texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        assert(mat.width > 0 && mat.height > 0);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mat.width, mat.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, mat.image);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        return mat.texture;
}

void
free_mat(lMaterial *mat)
{
        free(mat->name);
        stbi_image_free(mat->image);
        ZERO(mat)
}

void
free_mat(std::vector<lMaterial> &mats)
{
        for (lMaterial mat : mats)
                free_mat(&mat);
}

GLuint
load_texture_free_mat(lMaterial &mat)
{
        load_texture(mat);
        stbi_image_free(mat.image);
        return mat.texture;
}


void
__add_map_Kd(const char *str)
{
        char s[1024];
        strcpy(s, TEXTURE_PATH);
        strcat(s, str);
        char *c;
        if ((c = strchr(s, '\r')))
                *c = 0;
        if ((c = strchr(s, '\n')))
                *c = 0;

        stbi_set_flip_vertically_on_load(1);

        material.image = stbi_load(s, &material.width, &material.height,
                                   &material.comp, STBI_rgb_alpha);

        if (material.image == NULL) {
                if (endwith(s, "jpg")) {
                        char *str2 = strdup(str);
                        str2 = (char *) realloc(str2, strlen(str2) + 2);
                        strcpy(strstr(str2, "jpg"), "jpeg");
                        __add_map_Kd(str2);
                        free(str2);

                } else
                        fprintf(stderr, "Failed to load texture '%s'\n", s);
        }
}

void
__add_illum(const char *str)
{
        sscanf(str, "%d", &material.illum);
}

void
__add_d(const char *str)
{
        sscanf(str, "%f", &material.d);
}

void
__add_Ni(const char *str)
{
        sscanf(str, "%f", &material.Ni);
}

void
__add_Ke(const char *str)
{
        sscanf(str, "%f %f %f", &material.Ke[0], &material.Ke[1], &material.Ke[2]);
}

void
__add_Ks(const char *str)
{
        sscanf(str, "%f %f %f", &material.Ks[0], &material.Ks[1], &material.Ks[2]);
}

void
__add_Ka(const char *str)
{
        sscanf(str, "%f %f %f", &material.Ka[0], &material.Ka[1], &material.Ka[2]);
}

void
__add_Ns(const char *str)
{
        sscanf(str, "%f", &material.Ns);
}

static void
__named_object(const char *str)
{
        material.name = strdup(str);
        char *c;
        if ((c = strchr(material.name, '\r')))
                *c = 0;
        if ((c = strchr(material.name, '\n')))
                *c = 0;
}

static std::vector<lMaterial> materials;

std::vector<lMaterial>
load_mtl(const char *filename, int options)
{
        FILE *file;
        char buf[1024];

        file = fopen(filename, "r");
        if (file == NULL) {
                fprintf(stderr, "load_mtl can not load %s: ", filename);
                perror("");
                return materials;
        }

        ZERO(&material);

        /* Default mat */
        material.name = strdup("DefaultZeroMat");
        material.width = 1;
        material.height = 1;
        material.image = (unsigned char *) calloc(1, sizeof(char));
        materials.push_back(material);

        ZERO(&material);

        while (fgets(buf, sizeof buf - 1, file)) {
                if (!memcmp(buf, "newmtl ", 7)) {
                        if (material.image != NULL) {
                                materials.push_back(material);
                        }
                        ZERO(&material);
                        __named_object(buf + 7);
                }
                if (!memcmp(buf, "Ns ", 3))
                        __add_Ns(buf + 2);
                if (!memcmp(buf, "Ka ", 3))
                        __add_Ka(buf + 3);
                if (!memcmp(buf, "Ks ", 3))
                        __add_Ks(buf + 3);
                if (!memcmp(buf, "Ke ", 3))
                        __add_Ke(buf + 3);
                if (!memcmp(buf, "Ni ", 3))
                        __add_Ni(buf + 3);
                if (!memcmp(buf, "d ", 2))
                        __add_d(buf + 2);
                if (!memcmp(buf, "illum ", 6))
                        __add_illum(buf + 6);
                if (!memcmp(buf, "map_Kd ", 7))
                        __add_map_Kd(buf + 7);
        }

        materials.push_back(material);
        ZERO(&material);
        fclose(file);

        for (int i = 0; i < materials.size(); i++) {
                materials.at(i).id = i;
        }

        for (lMaterial m : materials)
                print_material(m);

        return materials;
}


void
print_material(lMaterial mat)
{
        printf("newmtl %s\n"
               "  id: %d\n"
               "  Ns %f\n"
               "  Ka %f %f %f\n"
               "  Ks %f %f %f\n"
               "  Ke %f %f %f\n"
               "  Ni %f\n"
               "  d %f\n"
               "  illum %d\n"
               "  height %d\n"
               "  width %d\n\n",
               mat.name, mat.id, mat.Ns, mat.Ka[0], mat.Ka[1], mat.Ka[2], mat.Ks[0],
               mat.Ks[1], mat.Ks[2], mat.Ke[0], mat.Ke[1], mat.Ke[2], mat.Ni,
               mat.d, mat.illum, mat.height, mat.width);
}
