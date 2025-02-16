#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#include "load_obj.h"

#if defined(DEBUG) && DEBUG
#define debug_printf(format, ...) printf(format, ##__VA_ARGS__)
#define debug_putchar(chr) putchar(chr)
#define debug_puts(str) puts(str)

#else
#define debug_printf(format, ...)
#define debug_putchar(chr)
#define debug_puts(str)

#endif

#define REALLOCARRAY(ptr, nmemb, size)                       \
        ({                                                   \
                void *ret = reallocarray(ptr, nmemb, size);  \
                if (!ret)                                    \
                {                                            \
                        printf(__FILE__ ": %d: ", __LINE__); \
                        perror("reallocarray");              \
                }                                            \
                ret;                                         \
        })


// clang-format off
typedef struct __attribute__((packed)) { float x, y, z, w; } vec4;
typedef struct __attribute__((packed)) { float x, y, z; } vec3;
typedef struct __attribute__((packed)) { float u, v, w; } uvw3;
typedef struct __attribute__((packed)) {int v, vt, vn;} fvec;
typedef struct { fvec *f; int size; } face_T;
typedef struct { int* v; int size; } line_T;
// clang-format on

struct __obj
{
        vec4 *vertex;
        uvw3 *texture;
        vec3 *normal;
        uvw3 *parameter;
        face_T *face;
        line_T *line;
        int v_size;
        int t_size;
        int n_size;
        int p_size;
        int f_size;
        int l_size;
} obj;


#if QUIET
static void
__obj_print_info()
{
}

#else
static void
__obj_print_info()
{
#define o (obj.vertex[i])
        if (obj.v_size > 0)
        {
                puts("[VERTEX]");
                for (int i = 0; i < obj.v_size; ++i)
                        printf("[%d] %f %f %f %f\n", i + 1, o.x, o.y, o.z, o.w);
        }
#undef o

#define o (obj.texture[i])
        if (obj.t_size > 0)
        {
                puts("[TEXTURE]");
                for (int i = 0; i < obj.t_size; ++i)
                        printf("[%d] %f %f %f\n", i + 1, o.u, o.v, o.w);
        }
#undef o

#define o (obj.normal[i])
        if (obj.n_size > 0)
        {
                puts("[NORMAL]");
                for (int i = 0; i < obj.n_size; ++i)
                        printf("[%d] %f %f %f\n", i + 1, o.x, o.y, o.z);
        }
#undef o

#define o (obj.parameter[i])
        if (obj.p_size > 0)
        {
                puts("[PARAM]");
                for (int i = 0; i < obj.p_size; ++i)
                        printf("[%d] %f %f %f\n", i + 1, o.u, o.v, o.w);
        }
#undef o

#define o (obj.face[i])
        if (obj.f_size > 0)
        {
                puts("[FACE]");
                for (int i = 0; i < obj.f_size; ++i)
                {
                        printf("[%d] ", i + 1);
                        for (int j = 0; j < o.size; ++j)
                                if (o.f[j].vn && o.f[j].vt)
                                        printf("%d/%d/%d ", o.f[j].v, o.f[j].vt,
                                               o.f[j].vn);
                                else if (o.f[j].vn)
                                        printf("%d//%d ", o.f[j].v, o.f[j].vn);
                                else if (o.f[j].vt)
                                        printf("%d/%d ", o.f[j].v, o.f[j].vt);
                                else
                                        printf("%d ", o.f[j].v);
                        puts("");
                }
        }
#undef o

#define o (obj.line[i])
        if (obj.l_size > 0)
        {
                puts("[LINE]");
                for (int i = 0; i < obj.l_size; ++i)
                {
                        printf("[%d] ", i + 1);
                        for (int j = 0; j < o.size; ++j)
                                printf("%d ", o.v[j]);
                        puts("");
                }
#undef o
        }
}

#endif


static void
__clear_obj()
{
        obj.vertex = NULL;
        obj.texture = NULL;
        obj.normal = NULL;
        obj.parameter = NULL;
        obj.face = NULL;
        obj.line = NULL;
        obj.v_size = 0;
        obj.t_size = 0;
        obj.n_size = 0;
        obj.p_size = 0;
        obj.f_size = 0;
        obj.l_size = 0;
}

static void
__delete_obj()
{
        free(obj.vertex);
        free(obj.texture);
        free(obj.normal);
        free(obj.parameter);
        free(obj.face);
        if (obj.line)
                for (int i = 0; i < obj.l_size; ++i)
                        free(obj.line[i].v);
        free(obj.line);
}

typedef struct
{
        unsigned int size;
        int *data;
} Indexes;

static Indexes
__get_indexes_from_faces()
{
        Indexes ind = { .size = 0, .data = NULL };
        for (int i = 0; i < obj.f_size; ++i)
        {
                ind.data =
                (int *) REALLOCARRAY(ind.data, ind.size + (obj.face[i].size - 2) * 3,
                                     sizeof(int));
                for (int j = 0; j < obj.face[i].size - 2; ++j)
                {
                        (ind.data)[j * 3] = obj.face[i].f[j].v - 1;
                        (ind.data)[j * 3 + 1] = obj.face[i].f[j + 1].v - 1;
                        (ind.data)[j * 3 + 2] = obj.face[i].f[j + 2].v - 1;
                        debug_printf("[TRIANGLE]: %d %d %d (%d,%d,%d)\n",
                                     obj.face[i].f[j].v, obj.face[i].f[j + 1].v,
                                     obj.face[i].f[j + 2].v, j * 3, j * 3 + 1, j * 3 + 2);
                }
                ind.size += (obj.face[i].size - 2) * 3;
        }
        return ind;
}

static int
__is_valid_obj()
{
        return obj.vertex != NULL && obj.face != NULL;
}

static void
__load_to_vao(GLuint *vao, unsigned int *indexes_size)
{
        GLuint VBO, EBO;

        if (!__is_valid_obj())
        {
                printf("OBJ is not printable\n");
                return;
        }

        /* ----[ *vao ]---- */
        /* Generates a Vertex Array Object (*vao) and binds it.
         * - *vao stores vertex attribute configurations and
         *   buffer bindings. It must be bound before
         *   configuring any vertex attributes. */
        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);
        assert(*vao);

        /* ----[ VBO ]---- */
        /* Generates a Vertex Buffer Object (VBO) and binds it.
         * - VBO stores the vertex data in GPU memory.
         * - `GL_ARRAY_BUFFER`: Specifies that this buffer
         *   holds vertex attributes. */
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        assert(VBO);

        /* Allocates and copies vertex data into the VBO.
         * - ``: Size of the data in bytes.
         * - ``: Pointer to the vertex data.
         * - `GL_STATIC_DRAW`: Data is set once and used
         *   many times (optimized for performance). */
        if (obj.vertex)
                glBufferData(GL_ARRAY_BUFFER, obj.v_size * sizeof(vec4),
                             obj.vertex, GL_STATIC_DRAW);

        /* ----[ EBO ]---- */
        /* Generates an Element Buffer Object (EBO) and binds it.
         * - EBO stores indices that define the order of vertices.
         * - `GL_ELEMENT_ARRAY_BUFFER`: Specifies that this buffer
         *   holds indices for indexed drawing. */
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        assert(EBO);

        /* Allocates and copies index data into the EBO.
         * - `sizeof(indices)`: Size of the index data in bytes.
         * - `indices`: Pointer to the index data.
         * - `GL_STATIC_DRAW`: Data is set once and used
         *   many times (optimized for performance). */
        if (obj.face)
        {
                Indexes indexes = __get_indexes_from_faces();

                printf("[INDEXES] ");
                for (int i =0; i < indexes.size; i++)
                        printf("%u ", indexes.data[i]);
                puts("");

                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size * sizeof(unsigned int),
                             indexes.data, GL_STATIC_DRAW);
                *indexes_size = indexes.size;
        }
        else
        {
                printf("Can not load to vao as figure has no faces\n");
                return;
        }

        /* ----[ ~~~ ]---- */

        /*
         * Defines how OpenGL should interpret the vertex
         * data.
         * - `0`: Attribute location in the shader
         *   (layout location 0).
         * - `3`: Number of components per vertex (x, y, z).
         * - `GL_FLOAT`: Data type of each component.
         * - `GL_FALSE`: No normalization.
         * - `3 * sizeof(float)`: Stride (distance between
         *   consecutive vertices).
         * - `(void *)0`: Offset in the buffer
         *   (starts at the beginning).
         */
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

        /* Enables the vertex attribute at location 0.
         * This allows OpenGL to use the vertex data
         * when rendering. */
        glEnableVertexAttribArray(0);

        /* Unbinds the VBO to prevent accidental
         * modification. Not neccesary */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Unbinds the *vao to avoid modifying it
         * unintentionally. This is good practice when
         * working with multiple *vaos. */
        glBindVertexArray(0);
}

static void
__add_vertex(const char *s)
{
#define tmp (obj.vertex + obj.v_size)
        obj.vertex = (vec4 *) REALLOCARRAY(obj.vertex, obj.v_size + 1, sizeof(vec4));
        tmp->x = 0;
        tmp->y = 0;
        tmp->z = 0;
        tmp->w = 0;
        sscanf(s, "%f %f %f %f", &tmp->x, &tmp->y, &tmp->z, &tmp->w);
        debug_printf("[+] VERT %d: %f %f %f %f\n", obj.v_size, tmp->x, tmp->y,
                     tmp->z, tmp->w);
        ++obj.v_size;
#undef tmp
}

static void
__add_texture(const char *s)
{
#define tmp (obj.texture + obj.t_size)
        obj.texture = (uvw3 *) REALLOCARRAY(obj.texture, obj.t_size + 1, sizeof(uvw3));
        tmp->u = 0;
        tmp->v = 0;
        tmp->w = 0;
        sscanf(s, "%f %f %f", &tmp->u, &tmp->v, &tmp->w);
        debug_printf("[+] TTRE %d: %f %f %f\n", obj.t_size, tmp->u, tmp->v, tmp->w);
        ++obj.t_size;
#undef tmp
}

static void
__add_normal(const char *s)
{
#define tmp (obj.normal + obj.n_size)
        obj.normal = (vec3 *) REALLOCARRAY(obj.normal, obj.n_size + 1, sizeof(vec3));
        tmp->x = 0;
        tmp->y = 0;
        tmp->z = 0;
        sscanf(s, "%f %f %f", &tmp->x, &tmp->y, &tmp->z);
        debug_printf("[+] NORM %d: %f %f %f\n", obj.n_size, tmp->x, tmp->y, tmp->z);
        ++obj.n_size;
#undef tmp
}

static void
__add_parameter(const char *s)
{
#define tmp (obj.parameter + obj.p_size)
        obj.parameter =
        (uvw3 *) REALLOCARRAY(obj.parameter, obj.p_size + 1, sizeof(uvw3));
        tmp->u = 0;
        tmp->v = 0;
        tmp->w = 0;
        sscanf(s, "%f %f %f", &tmp->u, &tmp->v, &tmp->w);
        debug_printf("[+] PARM %d: %f %f %f\n", obj.p_size, tmp->u, tmp->v, tmp->w);
        ++obj.p_size;
#undef tmp
}

static void
__add_face_entry(face_T *f, const char *s)
{
        f->f = (fvec *) REALLOCARRAY(f->f, f->size + 1, sizeof *(f->f));
#define o (f->f + f->size)
        o->vn = 0;
        o->vt = 0;
        o->v = 0;
        if (sscanf(s, "%d//%d", &o->v, &o->vn) == 2)
                debug_printf("%d//%d (%d)", o->v, o->vn, o->vt);
        else if (sscanf(s, "%d/%d/%d", &o->v, &o->vt, &o->vn) == 3)
                debug_printf("%d/%d/%d", o->v, o->vt, o->vn);
        else if (sscanf(s, "%d/%d", &o->v, &o->vt) == 2)
                debug_printf("%d/%d (%d)", o->v, o->vt, o->vn);
        else if (sscanf(s, "%d", &o->v) == 1)
                debug_printf("%d", o->v);
        ++(f->size);
#undef o
}


static void
__add_face(const char *s)
{
#define tmp (obj.face + obj.f_size)
        char *sptr;
        char *s_cpy = strdup(s);
        obj.face = (face_T *) REALLOCARRAY(obj.face, obj.f_size + 1, sizeof(face_T));
        sptr = strtok(s_cpy, " ");
        debug_printf("[+] FACE %d: ", obj.f_size + 1);
        tmp->f = NULL;
        tmp->size = 0;
        while (sptr)
        {
                __add_face_entry(tmp, sptr);
                debug_putchar(' ');
                sptr = strtok(NULL, " ");
        }
        debug_puts("");
        ++obj.f_size;
        free(s_cpy);
#undef tmp
}

static void
__add_line_entry(line_T *l, char *s)
{
        char *sptr = strtok(s, " ");
        while (sptr)
        {
                l->v = (int *) REALLOCARRAY(l->v, l->size, sizeof(int));
                (l->v)[l->size] = atoi(sptr);
                sptr = strtok(NULL, " ");
                ++(l->size);
        }
}

static void
__add_line(const char *s)
{
#define tmp (obj.line + obj.l_size)
        char *sptr;
        char *s_cpy = strdup(s);
        obj.line = (line_T *) REALLOCARRAY(obj.line, obj.l_size + 1, sizeof(line_T));
        sptr = strtok(s_cpy, " ");
        tmp->size = 0;
        tmp->v = NULL;
        while (sptr)
        {
                __add_line_entry(tmp, sptr);
                sptr = strtok(NULL, " ");
        }
        free(s_cpy);
        debug_printf("[+] LINE %d: ", obj.l_size);
        for (int i = 0; i < obj.l_size; i++)
                debug_printf("%d ", tmp->v[i]);
        debug_puts("");
        ++obj.l_size;
#undef tmp
}

void
load_obj(const char *filename, unsigned int *vao, unsigned int *indexes_size, int options)
{
        FILE *file;
        char buf[1024];

        file = fopen(filename, "r");
        if (file == NULL)
        {
                fprintf(stderr, "load_obj can not load %s\n", filename);
                return;
        }

        __clear_obj();

        while (fgets(buf, sizeof buf - 1, file))
        {
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
        }

        __obj_print_info();
        __load_to_vao(vao, indexes_size);
        __delete_obj();
}

#ifdef TEST

int
main(int argc, char *argv[])
{
        if (argc < 2)
        {
                printf("Usage: %s <file.obj>\n", argv[0]);
                return 0;
        }
        load_obj(argv[1], 0, 0);
        return 0;
}

#endif
