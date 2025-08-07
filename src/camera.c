#include "../include/renderer.h"
static camera_t camera = {
    .position = {0,0,-3},
    .direction = {0,0,1},
    .forward_velocity = {0,0,0},
    .right = {1,0,0},
    .up = {0,1,0},
    .yaw_angle = 0.0,
    .pitch_angle = 0.0,
    .roll_angle = 0.0
};

vec3f_t get_camera_pos(void){
    return camera.position;
}

vec3f_t get_camera_dir(void){
    return camera.direction;
}
vec3f_t get_camera_vel(void){
    return camera.forward_velocity;
}

vec3f_t get_camera_right(){
    return camera.right;
}

vec3f_t get_camera_up(){
    return camera.up;
}

void set_camera_pos(vec3f_t pos){
  
    camera.position = pos;
}
void set_camera_dir(vec3f_t dir){
    camera.direction = dir;
}
void set_camera_vel(vec3f_t vel){
    camera.forward_velocity = vel;
}

void set_camera_right(vec3f_t right){
    camera.right = right;
}

void set_camera_up(vec3f_t up){
    camera.up = up;
}

float  get_camera_yaw(void){
    return camera.yaw_angle;
}
float  get_camera_pitch(void){
    return camera.pitch_angle;
}
float  get_camera_roll(void){
    return camera.roll_angle;
}

void  rotate_camera_yaw(float angle){
    camera.yaw_angle += angle;
}
void  rotate_camera_pitch(float angle){
    camera.pitch_angle += angle;
}
void  rotate_camera_roll(float angle){
    camera.roll_angle += angle;
}

vec3f_t get_camera_lookat_target_old(){
    vec3f_t target = {0,0,-1};
   
    
    mat4_t camera_yaw_rotation = mat4_rotate_y(get_camera_yaw());
    mat4_t camera_pitch_rotation = mat4_rotate_x(get_camera_pitch());

    mat4_t camera_rotation = mat4_identity();

    camera_rotation = matrix_mult_mat4(camera_yaw_rotation,camera_rotation);
    camera_rotation = matrix_mult_mat4(camera_pitch_rotation,camera_rotation);

    vector_t camera_direction;
    camera_direction = vec4_from_vec3(target);
    camera_direction = matrix_mult_vec4(camera_rotation, camera_direction);
    camera_direction.z = -camera_direction.z;
    camera.direction = vec3_from_vec4(camera_direction);


    target = vec3_add(camera.position,camera.direction);

    return target;

}

vec3f_t get_camera_lookat_target(){
    vec3f_t target = {0,0,-1};

    mat4_identity();
    mat_rotate_y(get_camera_yaw());
    mat_rotate_x(get_camera_pitch());

    vector_t camera_direction = (vector_t){0,0,0,0};

    float x = camera_direction.x;
    float y = camera_direction.y;
    float z = camera_direction.z;
    float w = camera_direction.w;

    mat_trans_single3_nodiv(x, y, z);

    camera_direction.x = x;
    camera_direction.y = y;
    camera_direction.z = z;
    camera_direction.w = w;


    target.x = camera.position.x + camera.direction.x;
    target.y = camera.position.y + camera.direction.y;
    target.z = camera.position.z + camera.direction.z; 

    return target;

}

void update_camera_vectors(){
     // calculate the new Front vector
    float local_pitch = get_camera_pitch();
    float local_yaw = get_camera_yaw();

    // local_pitch *= (M_PI/180.0f);
    // local_yaw   *= (M_PI/180.0f);

    camera.direction.x = cosf(local_yaw) * cosf(local_pitch);
    camera.direction.y = sinf(local_pitch);
    camera.direction.z = sinf(local_yaw) * cosf(local_pitch);
    camera.direction = vec_normalize(camera.direction);
    // also re-calculate the Right and Up vector
    vec3f_t up = get_camera_up();
    camera.right = vec3_cross(camera.direction,up);
    camera.right = vec_normalize(camera.right);
    up = vec3_cross(camera.right,camera.direction);
    camera.up = vec_normalize(camera.up);

    set_camera_vel((vec3f_t){0,0,0});
}

void process_mouse_move(float xPos, float yPos,float delta_time){
    float sensitivity = 0.1f;
    
    camera.yaw_angle += ((float)-xPos * sensitivity * delta_time);
    printf("%f\n",camera.yaw_angle);
    camera.pitch_angle += (float)yPos * sensitivity * delta_time;
    
    if(camera.pitch_angle > 89.0f)
        camera.pitch_angle = 89.0f;
    if(camera.pitch_angle < -89.0f)
        camera.pitch_angle = -89.0f;



    update_camera_vectors();
}