#include "../include/renderer.h"
#include <string.h>
mesh_t mesh = {
    .vertices =NULL,
    .faces = NULL,
    .face_normals = NULL,
    .rotation = {0,0,0},
    .scale = {1.0,1.0,1.0},
    .translation = {0,0,0}
};

#define MAX_NUMBER_MESHES 32    
static mesh_t meshes[MAX_NUMBER_MESHES];
static int mesh_count = 0;


void load_mesh_obj_data(mesh_t* mesh, char* filename) {
    FILE* file;
    file = fopen(filename, "r");
    char line[1024];
    tex2_t* texcoords = NULL;
    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            shz_vec3_t vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh->vertices, vertex);
           
        }
        if(strncmp(line,"vn ",3) == 0){
            shz_vec3_t normal;
            sscanf(line,"vn %f %f %f", &normal.x, &normal.y, &normal.z);
            array_push(mesh->normals, normal);
        }
        if(strncmp(line,"vt ",3) == 0){
            tex2_t texcoord;
            sscanf(line,"vt %f %f", &texcoord.u, &texcoord.v);
            array_push(texcoords,texcoord);
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0], 
                &vertex_indices[1], &texture_indices[1], &normal_indices[1], 
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]
            ); 
            face_t face = {
                .a = vertex_indices[0] - 1,
                .b = vertex_indices[1] - 1,
                .c = vertex_indices[2] - 1,
                .a_n = normal_indices[0] - 1,
                .b_n = normal_indices[1] - 1,
                .c_n = normal_indices[2] - 1,
                .a_uv = texcoords[texture_indices[0] - 1],
                .b_uv = texcoords[texture_indices[1] - 1],
                .c_uv = texcoords[texture_indices[2] - 1]
                
            };

            // printf("Loaded face: %d/%d/%d %d/%d/%d %d/%d/%d\n",
            //     vertex_indices[0], texture_indices[0], normal_indices[0],
            //     vertex_indices[1], texture_indices[1], normal_indices[1],
            //     vertex_indices[2], texture_indices[2], normal_indices[2]
            // );
            array_push(mesh->faces, face);
        }
    }
    array_free(texcoords);
}

void load_mesh_png_data(mesh_t* mesh, char* filename){
    png_to_img(filename, 0, &mesh->img);
    if(mesh->img.w > 0){
    }else{
         fprintf(stderr, "Error loading PNG file: %s\n", filename);
        return;
    }
}

void load_mesh(char* obj_path, char* png_path, shz_vec3_t scale, shz_vec3_t translation, shz_vec3_t rotation){
    load_mesh_obj_data(&meshes[mesh_count], obj_path);
    load_mesh_png_data(&meshes[mesh_count], png_path);
   
    meshes[mesh_count].scale = scale;
    meshes[mesh_count].translation = translation;
    meshes[mesh_count].rotation = rotation;


    mesh_count++;
}

int get_num_meshes(){
    return mesh_count;
}

mesh_t* get_mesh(int index){
    return &meshes[index];
}

void free_meshes(){

}
