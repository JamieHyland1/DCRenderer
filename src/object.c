#include "../include/core.h"
#define MAX_OBJECTS_PER_SCENE 150

static object_t objects[MAX_OBJECTS_PER_SCENE];
static int object_count = 0;

bool create_object(char* name, int id){
  objects[object_count].name = name;
  objects[object_count].id = id;
  object_count++;
  return true;
}


int get_num_objects(){
  return object_count;
}



bool get_object(int index, object_t* out){
  if(index < 0 || index >= MAX_OBJECTS_PER_SCENE )return false;
  *out = objects[index];
  return true;
}
