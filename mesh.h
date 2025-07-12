#ifndef mesh_h
#define mesh_h

#include "triangle.h"
#include <dc/vector.h>


#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6*2)

/////////////////////////////////////////////////////////////////////////////////////////
// Define a struct for dynamic size meshes
////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    vector_t* vertices; // Dynamic array
    face_t* faces; // Dynamic array
    kos_img_t* texture; // Mesh png texture pointer
    vector_t* face_normals; // dynamic array
    vector_t rotation;
    vector_t scale; // Scale x,y,x
    vector_t translation; // Translate x,y,z
    
} mesh_t;

extern mesh_t mesh;

void load_mesh(char* obj_path, char* png_path, vector_t scale, vector_t translation, vector_t rotation);
void load_mesh_png_data(mesh_t* mesh, char* filename);
void load_obj_file_data(mesh_t mesh,char* filename);

int get_num_meshes();
mesh_t* get_mesh(int index);

void free_meshes(void);

#endif