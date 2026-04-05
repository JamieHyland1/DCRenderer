#ifndef object_h
#define object_h
#include <stdio.h>
#include "mesh.h"



typedef struct {
  char* name;
  int id;
} object_t;

bool create_object(char* name, int id);
bool get_object(int index, object_t* out);


#endif
