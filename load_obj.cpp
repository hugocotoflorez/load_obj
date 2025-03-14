#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <strings.h>
#include <vector>

#include "load_mtl.h"
#include "load_obj.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// clang-format off
typedef struct { float x, y, z, w; } vec4;
typedef struct { float x, y, z; } vec3;
typedef struct { float u, v, w; } uvw3;
typedef struct { unsigned int v, vt, vn; } fvec;
typedef std::vector<fvec> face_T;
typedef std::vector<int> line_T;
// clang-format on

__attribute__((constructor)) void
__typetest__()
{
        assert(sizeof(vec3) == sizeof(float) * 3);
        assert(sizeof(vec4) == sizeof(float) * 4);
        assert(sizeof(uvw3) == sizeof(float) * 3);
        assert(sizeof(fvec) == sizeof(unsigned int) * 3);
}

struct __obj {
        std::vector<vec4> vertex;
        std::vector<uvw3> texture;
        std::vector<vec3> normal;
        std::vector<uvw3> parameter;
        std::vector<face_T> face;
        std::vector<line_T> line;
        lMaterial *material;
        char name[32];
} obj;

static void
__clear_obj()
{
        obj.face.clear();
        obj.line.clear();
}

static std::vector<float>
__compute_data(std::vector<unsigned int> &ind)
{
        printf("Computing data ...");
        std::vector<float> data;
        std::vector<unsigned int> temp_ind;

        for (face_T face : obj.face) {
                for (int i = 0; i < face.size(); i++) {
                        fvec f = face.at(i);

                        if (obj.vertex.size() < f.v) {
                                printf("Vertex oob\n");
                                continue;
                        }

                        if (obj.texture.size() < f.vt) {
                                printf("Texture oob\n");
                                continue;
                        }

                        assert(data.size() % 7 == 0);
                        temp_ind.push_back(data.size() / 7);

                        data.push_back(obj.vertex.at(f.v - 1).x);
                        data.push_back(obj.vertex.at(f.v - 1).y);
                        data.push_back(obj.vertex.at(f.v - 1).z);
                        data.push_back(obj.vertex.at(f.v - 1).w);
                        data.push_back(obj.texture.at(f.vt - 1).u);
                        data.push_back(obj.texture.at(f.vt - 1).v);
                        data.push_back(obj.texture.at(f.vt - 1).w);

                        unsigned int fails = 0;

                        if (obj.texture.at(f.vt - 1).u > 1.0f || obj.texture.at(f.vt - 1).u < 0.0f) {
                                printf("[!] obj.texture.at(f.vt - 1).u > 1.0f FAILED\n");
                                ++fails;
                        }
                        if (obj.texture.at(f.vt - 1).v > 1.0f || obj.texture.at(f.vt - 1).v < 0.0f) {
                                printf("[!] obj.texture.at(f.vt - 1).v > 1.0f FAILED\n");
                                ++fails;
                        }
                        if (obj.texture.at(f.vt - 1).w > 1.0f || obj.texture.at(f.vt - 1).w < 0.0f) {
                                printf("[!] obj.texture.at(f.vt - 1).w > 1.0f FAILED\n");
                                ++fails;
                        }

                        if (fails)
                                printf("Errors in textures: %u\n", fails);
                }

                for (int i = 0; i < temp_ind.size() - 2; i++) {
                        ind.push_back(temp_ind.at(0));
                        ind.push_back(temp_ind.at(i + 1));
                        ind.push_back(temp_ind.at(i + 2));
                }

                temp_ind.clear();
        }
        printf(" data computed: \n"
               " %zu vertex\n"
               " %zu indexes\n",
               data.size(), ind.size());

        return data;
}

static int
__is_valid_obj()
{
        return obj.vertex.size() > 0 && obj.face.size() > 0;
}

static void
__load_to_vao(lObject *o)
{
        GLuint VBO, EBO;
        GLuint *vao = &o->vao;

        strcpy(o->name, obj.name);
        o->no_display = 0;


        if (!__is_valid_obj()) {
                printf("OBJ is not printable\n");
                return;
        }

        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);
        assert(*vao);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        assert(VBO);

        std::vector<unsigned int> indexes = std::vector<unsigned int>();
        std::vector<float> data = __compute_data(indexes);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        assert(EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(*indexes.data()),
                     indexes.data(), GL_STATIC_DRAW);

        o->index_n = indexes.size();

        // load texture and link it to a texture if not linked */
        load_texture(*obj.material);

        o->material = obj.material;

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(*data.data()), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(*data.data()), (void *) (4 * sizeof(*data.data())));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
}

static void
__add_vertex(const char *s)
{
        vec4 tmp;
        sscanf(s, "%f %f %f %f", &tmp.x, &tmp.y, &tmp.z, &tmp.w);
        obj.vertex.push_back(tmp);
}

static void
__add_texture(const char *s)
{
        uvw3 tmp;
        sscanf(s, "%f %f %f", &tmp.u, &tmp.v, &tmp.w);
        obj.texture.push_back(tmp);
}

static void
__add_normal(const char *s)
{
        vec3 tmp;
        sscanf(s, "%f %f %f", &tmp.x, &tmp.y, &tmp.z);
        obj.normal.push_back(tmp);
}

static void
__add_parameter(const char *s)
{
        uvw3 tmp;
        sscanf(s, "%f %f %f", &tmp.u, &tmp.v, &tmp.w);
        obj.parameter.push_back(tmp);
}

static fvec
__parse_face_entry(const char *s)
{
        fvec face = { 0 };
        if (sscanf(s, "%d//%d", &face.v, &face.vn) == 2)
                ;
        else if (sscanf(s, "%d/%d/%d", &face.v, &face.vt, &face.vn) == 3)
                ;
        else if (sscanf(s, "%d/%d", &face.v, &face.vt) == 2)
                ;
        else if (sscanf(s, "%d", &face.v) == 1)
                ;
        return face;
}


static void
__add_face(const char *s)
{
        face_T f;
        char *sptr;
        char *s_cpy = strdup(s);
        sptr = strtok(s_cpy, " ");

        while (sptr) {
                f.push_back(__parse_face_entry(sptr));
                sptr = strtok(NULL, " ");
        }

        obj.face.push_back(f);
        free(s_cpy);
}

static void
__add_line_entry(line_T *l, char *s)
{
        char *sptr = strtok(s, " ");
        while (sptr) {
                l->push_back(atoi(sptr));
                sptr = strtok(NULL, " ");
        }
}

static void
__add_line(const char *s)
{
        char *sptr;
        char *s_cpy = strdup(s);
        sptr = strtok(s_cpy, " ");
        line_T l;
        while (sptr) {
                __add_line_entry(&l, sptr);
                sptr = strtok(NULL, " ");
        }
        obj.line.push_back(l);
        free(s_cpy);
}

void
__add_material(char *str, std::vector<lMaterial> *materials)
{
        char *c;
        if ((c = strchr(str, '\n')))
                *c = 0;

        for (int i = 0; i < materials->size(); i++) {
                lMaterial *mat = materials->data() + i;
                if (strcmp(mat->name, str) == 0) {
                        obj.material = mat;
                        return;
                }
        }

        printf("Material %s not found!\n", str);
}

void
__add_mtl_file(const char *buf, std::vector<lMaterial> &materials)
{
        char buf2[256];
        char *c;
        strcpy(buf2, SOURCE_PATH);
        strcat(buf2, buf + 7);
        if ((c = strrchr(buf2, '\r')))
                *c = 0; // strip \n
        if ((c = strrchr(buf2, '\n')))
                *c = 0; // strip \n

        // Allow multiple mtl files
        for (lMaterial mat : load_mtl(buf2))
                materials.push_back(mat);
}

/*
 * o name
 * v
 * v
 * v
 * v
 * usemtl mat
 * f
 * f
 * f
 * usemtl mat2  <- create new obj (name + mat)
 * f
 * f
 * f
 *              <- create new obj (name + mat2)
 */

void
__create_new_obj(std::vector<lObject> &objects, int options)
{
        if (__is_valid_obj()) {
                if (obj.material->image != NULL) {
                        if (options & LOAD_3_3) {
                                lObject o;
                                __load_to_vao(&o);
                                objects.push_back(o);
                        }
                        if (options & LOAD_1_2)
                                printf("LOAD 1.2 not implemented!\n");
                        __clear_obj();
                }
        }
}


void
__add_name(const char *str)
{
        char *c;
        strcpy(obj.name, str);
        if ((c = strrchr(obj.name, '\n')))
                *c = 0; // strip \n
}


std::vector<lMaterial> materials;

std::vector<lObject>
load_obj(const char *filename, int options)
{
        FILE *file;
        char buf[1024];
        std::vector<lObject> objects;

        file = fopen(filename, "r");
        if (file == NULL) {
                fprintf(stderr, "load_obj can not load %s", filename);
                perror("");
                return objects;
        }

        while (fgets(buf, sizeof buf - 1, file)) {
                if (!memcmp(buf, "o ", 2)) {
                        __add_name(buf + 2);
                        __create_new_obj(objects, options);
                }
                if (!memcmp(buf, "v ", 2))
                        __add_vertex(buf + 2);
                if (!memcmp(buf, "vt ", 3))
                        __add_texture(buf + 3);
                if (!memcmp(buf, "vn ", 3))
                        __add_normal(buf + 3);
                if (!memcmp(buf, "vp ", 3))
                        __add_parameter(buf + 3);
                if (!memcmp(buf, "f ", 2))
                        __add_face(buf + 2);
                if (!memcmp(buf, "l ", 2))
                        __add_line(buf + 2);
                if (!memcmp(buf, "mtllib ", 7))
                        __add_mtl_file(buf, materials);

                if (!memcmp(buf, "usemtl ", 7)) {
                        __add_material(buf + 7, &materials);
                        __create_new_obj(objects, options);
                }
        }

        __create_new_obj(objects, options);
        fclose(file);

        for (lObject obj : objects) {
                printf("New Object created:\n");
                printf("  name: %s\n", obj.name);
                printf("  VAO: %u\n", obj.vao);
                printf("  MAT: %s\n", obj.material->name);
                printf("  MAT id: %u\n", obj.material->texture);
                printf("\n");
        }

        return objects;
}

