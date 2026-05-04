#include "../include/core.h"
#define MAX_OBJECTS_PER_SCENE 150

static object_t objects[MAX_OBJECTS_PER_SCENE] __attribute__((aligned(32)));
static int object_count = 0;

bool create_object(const int id){
  objects[object_count].id = id;
  update_obj_translation(object_count, &(shz_vec3_t){0, 0, 0});
  update_obj_rotation(object_count, &(shz_vec3_t){0, 0, 0});
  update_obj_scale(object_count, &(shz_vec3_t){1, 1, 1});
  object_count++;
  return true;
}

int get_num_objects(){
  return object_count;
}

bool get_object(int index, object_t* out) {
    if (out == NULL) return false;
    if (index < 0 || index >= object_count) return false;

    *out = objects[index];
    return true;
}


mesh_t* get_object_mesh(object_t* obj){
   return get_mesh(obj->id);
 }

void update_obj_translation(int id, shz_vec3_t* t){
  objects[id].translation.x = t->x;
  objects[id].translation.y = t->y;
  objects[id].translation.z = t->z;
}

void update_obj_rotation(int id, shz_vec3_t* r){
  objects[id].rotation.x = r->x;
  objects[id].rotation.y = r->y;
  objects[id].rotation.z = r->z;
}

void update_obj_scale(int id, shz_vec3_t* s){
  objects[id].scale.x = s->x;
  objects[id].scale.y = s->y;
  objects[id].scale.z = s->z;
}

shz_vec3_t get_obj_translation(int id){
  return objects[id].translation;
}

shz_vec3_t get_obj_rotation(int id){
  return objects[id].rotation;
}

shz_vec3_t get_obj_scale(int id){
  return objects[id].scale;
}
