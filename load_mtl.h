#ifndef LOAD_MTL_H
#define LOAD_MTL_H
#define TEXTURE_PATH "resources/textures/"


typedef struct lMaterial {
        char *name;
        float Ns;
        float Ka[3];
        float Ks[3];
        float Ke[3];
        float Ni;
        float d;
        int illum;
        unsigned int texture;
        unsigned char *image;
        int height;
        int width;
        int comp;
        unsigned int id;
} lMaterial;

#include <vector>

/* Load a mtl file into a lMaterial array */
std::vector<lMaterial> load_mtl(const char *filename, int options = 0);

/* Load a lMaterial to a openGL texture */
unsigned int load_texture(lMaterial &mat);

/* Load a lMaterial to a openGL texture and free lMaterial image data */
unsigned int load_texture_free_mat(lMaterial *mat);

/* Free a material or vector of materials */
void free_mat(lMaterial *mat);
void free_mat(std::vector<lMaterial> &mats);

/* Print material info to stdout */
void print_material(lMaterial mat);


#endif // !LOAD_MTL_H
