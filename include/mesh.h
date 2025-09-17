#ifndef mesh_h
#define mesh_h

#include "triangle.h"
#include "vector.h"
#include "upng.h"
#include <png/png.h>
#include <stddef.h>
#include <stdio.h>


#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6*2)

/////////////////////////////////////////////////////////////////////////////////////////
// Define a struct for dynamic size meshes
////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    shz_vec3_t* vertices;     // 4 bytes
    shz_vec3_t* normals;      // 4 bytes
    face_t* faces;         // 4 bytes
    upng_t* texture;       // 4 bytes
    shz_vec3_t* face_normals; // 4 bytes
    shz_vec3_t rotation;      // 12 bytes
    shz_vec3_t scale;         // 12 bytes
    shz_vec3_t translation;   // 12 bytes
    kos_img_t img;         // 20 bytes
    uint8_t padding[20];      // Add 20 bytes to reach 96 bytes
} mesh_t __attribute__((aligned(32)));

_Static_assert(sizeof(mesh_t) % 32 == 0, "mesh_t must be 32-byte aligned for performance");
extern mesh_t mesh;

void load_mesh(char* obj_path, char* png_path, shz_vec3_t scale, shz_vec3_t translation, shz_vec3_t rotation);
void load_mesh_png_data(mesh_t* mesh, char* filename);
void load_obj_file_data(mesh_t mesh,char* filename);

int get_num_meshes();
mesh_t* get_mesh(int index);

void free_meshes(void);

#endif