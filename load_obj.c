#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_obj(const char *filename, int *vao);

// clang-format off
typedef struct { float x, y, z, w; } vec4;
typedef struct { float x, y, z; } vec3;
typedef struct { float u, v, w; } uvw3;
typedef struct { int v, vt, vn; } face;
typedef struct { int* v; } line;
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
        obj.vertex = (vec4 *) reallocarray(obj.vertex, obj.v_size + 1, sizeof(vec4));
        tmp->x = 0;
        tmp->y = 0;
        tmp->z = 0;
        tmp->w = 0;
        sscanf(s, "%f %f %f %f", &tmp->x, &tmp->y, &tmp->z, &tmp->w);
        printf("[+] VERT %d: %f %f %f %f\n", obj.v_size, tmp->x, tmp->y, tmp->z,
               tmp->w);
        ++obj.v_size;
#undef tmp
}

static void
__add_texture(const char *s)
{
#define tmp (obj.texture + obj.t_size)
        obj.texture = (uvw3 *) reallocarray(obj.texture, obj.t_size + 1, sizeof(uvw3));
        tmp->u = 0;
        tmp->v = 0;
        tmp->w = 0;
        sscanf(s, "%f %f %f", &tmp->u, &tmp->v, &tmp->w);
        printf("[+] TTRE %d: %f %f %f\n", obj.t_size, tmp->u, tmp->v, tmp->w);
        ++obj.t_size;
#undef tmp
}

static void
__add_normal(const char *s)
{
#define tmp (obj.normal + obj.n_size)
        obj.normal = (vec3 *) reallocarray(obj.normal, obj.n_size + 1, sizeof(vec3));
        tmp->x = 0;
        tmp->y = 0;
        tmp->z = 0;
        sscanf(s, "%f %f %f", &tmp->x, &tmp->y, &tmp->z);
        printf("[+] NORM %d: %f %f %f\n", obj.n_size, tmp->x, tmp->y, tmp->z);
        ++obj.n_size;
#undef tmp
}

static void
__add_parameter(const char *s)
{
#define tmp (obj.parameter + obj.p_size)
        obj.parameter =
        (uvw3 *) reallocarray(obj.parameter, obj.p_size + 1, sizeof(uvw3));
        tmp->u = 0;
        tmp->v = 0;
        tmp->w = 0;
        sscanf(s, "%f %f %f", &tmp->u, &tmp->v, &tmp->w);
        printf("[+] PARM %d: %f %f %f\n", obj.p_size, tmp->u, tmp->v, tmp->w);
        ++obj.p_size;
#undef tmp
}

static void
__add_face_entry(face *f, const char *s)
{
        int size = 0;
        f->vn = 0;
        f->vt = 0;
        f->v = 0;
        if (sscanf(s, "%d//%d", &f->v, &f->vn) == 2)
                printf("%d//%d", f->v, f->vn);
        else if (sscanf(s, "%d/%d/%d", &f->v, &f->vt, &f->vn) == 3)
                printf("%d/%d/%d", f->v, f->vt, f->vn);
        else if (sscanf(s, "%d/%d", &f->v, &f->vt) == 2)
                printf("%d/%d", f->v, f->vt);
        else if (sscanf(s, "%d", &f->v) == 1)
                printf("%d", f->v);
        return;
}

static void
__add_face(const char *s)
{
#define tmp (obj.face + obj.f_size)
        char *sptr;
        char *s_cpy = strdup(s);
        obj.face = (face *) reallocarray(obj.face, obj.f_size + 1, sizeof(face));
        sptr = strtok(s_cpy, " ");
        printf("[+] FACE %d: ", obj.f_size + 1);
        while (sptr)
        {
                __add_face_entry(obj.face, sptr);
                putchar(' ');
                sptr = strtok(NULL, " ");
        }
        puts("");
        ++obj.f_size;
        free(s_cpy);
#undef tmp
}

static void
__add_line_entry(line *l, char *s)
{
        int size = 0;
        char *sptr = strtok(s, " ");
        l->v = NULL;
        while (sptr)
        {
                l->v = (int *) reallocarray(l->v, size, sizeof(int));
                (l->v)[size] = atoi(sptr);
                sptr = strtok(NULL, " ");
                ++size;
        }
}

static void
__add_line(const char *s)
{
#define tmp (obj.line + obj.l_size)
        char *sptr;
        char *s_cpy = strdup(s);
        obj.line = (line *) reallocarray(obj.line, obj.l_size + 1, sizeof(line));
        sptr = strtok(s_cpy, " ");
        while (sptr)
        {
                __add_line_entry(obj.line, sptr);
                sptr = strtok(NULL, " ");
        }
        ++obj.l_size;
        free(s_cpy);
        printf("[+] LINE %d: ", obj.l_size);
        for (int i = 0; i < obj.l_size; i++)
                printf("%d ", tmp->v[i]);
        puts("");
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
