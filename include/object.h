#ifndef object_h
#define object_h
#include <stdio.h>
#include "mesh.h"



typedef struct {
  char* name;
  mesh_t* mesh;
} object_t;

bool create_object(char* name, mesh_t* mesh);
bool get_object(int index, object_t* out);


#endif
