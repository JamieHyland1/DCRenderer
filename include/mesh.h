#ifndef mesh_h
#define mesh_h

#include "triangle.h"
#include "vector.h"
#include "upng.h"
#include <png/png.h>

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6*2)

/////////////////////////////////////////////////////////////////////////////////////////
// Define a struct for dynamic size meshes
////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    vec3f_t* vertices; // Dynamic array
    face_t* faces; // Dynamic array
    upng_t* texture; // Mesh png texture pointer
    vec3f_t* face_normals; // dynamic array
    vec3f_t rotation;
    vec3f_t scale; // Scale x,y,x
    vec3f_t translation; // Translate x,y,z
    kos_img_t img;
    
} mesh_t;

extern mesh_t mesh;

void load_mesh(char* obj_path, char* png_path, vec3f_t scale, vec3f_t translation, vec3f_t rotation);
void load_mesh_png_data(mesh_t* mesh, char* filename);
void load_obj_file_data(mesh_t mesh,char* filename);

int get_num_meshes();
mesh_t* get_mesh(int index);

void free_meshes(void);

#endif