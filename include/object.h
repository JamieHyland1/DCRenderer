#ifndef object_h
#define object_h
#include <stdio.h>
#include "mesh.h"
#include "core.h"
typedef struct __attribute__((aligned(32))){
  shz_vec3_t rotation;      // 12 bytes
  shz_vec3_t scale;         // 12 bytes
  shz_vec3_t translation;   // 12 bytes
  int id;                   // 4 bytes
  int pad[6];               // 24 bytes (padding to align to 32-byte boundary)

} object_t; // Total: 64 bytes, aligned to 32 bytes boundary with padding

_Static_assert(sizeof(object_t) == 64, "object_t should be 64 bytes");
_Static_assert(_Alignof(object_t) == 32, "object_t should be 32-byte aligned");

bool create_object(const int id);
bool get_object(int index, object_t* out);
int get_num_objects();
mesh_t* get_object_mesh(object_t* obj);
void update_obj_translation(int id, shz_vec3_t* t);
void update_obj_rotation(int id, shz_vec3_t* r);
void update_obj_scale(int id, shz_vec3_t* s);
shz_vec3_t get_obj_translation(int id);
shz_vec3_t get_obj_rotation(int id);
shz_vec3_t get_obj_scale(int id);
#endif
