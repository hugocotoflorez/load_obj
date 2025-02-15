#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define REALLOCARRAY(ptr, nmemb, size)                            \
        ({                                                        \
                __auto_type ret = reallocarray(ptr, nmemb, size); \
                if (!ret)                                         \
                {                                                 \
                        printf(__FILE__ ": %d: ", __LINE__);      \
                        perror("reallocarray");                   \
                }                                                 \
                ret;                                              \
        })

void load_obj(const char *filename, int *vao);

// clang-format off
typedef struct { float x, y, z, w; } vec4;
typedef struct { float x, y, z; } vec3;
typedef struct { float u, v, w; } uvw3;
typedef struct { struct {int v, vt, vn;} *f; int size; } face;
typedef struct { int* v; int size; } line;
// clang-format on

struct __obj
{
        vec4 *vertex;
        uvw3 *texture;
        vec3 *normal;
        uvw3 *parameter;
        face *face;
        line *line;
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

static void
__load_to_vao(int *vao)
{
        // TODO
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
        ++obj.v_size;
        debug_printf("[+] VERT %d: %f %f %f %f\n", obj.v_size, tmp->x, tmp->y,
                     tmp->z, tmp->w);
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
        ++obj.t_size;
        debug_printf("[+] TTRE %d: %f %f %f\n", obj.t_size, tmp->u, tmp->v, tmp->w);
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
        ++obj.n_size;
        debug_printf("[+] NORM %d: %f %f %f\n", obj.n_size, tmp->x, tmp->y, tmp->z);
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
        ++obj.p_size;
        debug_printf("[+] PARM %d: %f %f %f\n", obj.p_size, tmp->u, tmp->v, tmp->w);
#undef tmp
}

static void
__add_face_entry(face *f, const char *s)
{
        f->f = REALLOCARRAY(f->f, f->size + 1, sizeof *(f->f));
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
                debug_printf("%d (%d %d)", o->v, o->vt, o->vn);
        ++(f->size);
#undef o
}


static void
__add_face(const char *s)
{
#define tmp (obj.face + obj.f_size)
        char *sptr;
        char *s_cpy = strdup(s);
        obj.face = (face *) REALLOCARRAY(obj.face, obj.f_size + 1, sizeof(face));
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
__add_line_entry(line *l, char *s)
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
        obj.line = (line *) REALLOCARRAY(obj.line, obj.l_size + 1, sizeof(line));
        sptr = strtok(s_cpy, " ");
        tmp->size = 0;
        tmp->v = NULL;
        while (sptr)
        {
                __add_line_entry(tmp, sptr);
                sptr = strtok(NULL, " ");
        }
        ++obj.l_size;
        free(s_cpy);
        debug_printf("[+] LINE %d: ", obj.l_size);
        for (int i = 0; i < obj.l_size; i++)
                debug_printf("%d ", tmp->v[i]);
        debug_puts("");
#undef tmp
}

void
load_obj(const char *filename, int *vao)
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
        __load_to_vao(vao);
        __delete_obj();
}

#define TEST
#ifdef TEST

int
main(int argc, char *argv[])
{
        if (argc < 2)
        {
                printf("Usage: %s <file.obj>\n", argv[0]);
                return 0;
        }
        load_obj(argv[1], 0);
        return 0;
}

#endif
