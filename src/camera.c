#include "../include/renderer.h"
static camera_t camera = {
    .position = {{ .x = 0.0f, .y = 0.0f, .z = -3.0f }},
    .direction = {{ .x = 0.0f, .y = 0.0f, .z = 1.0f }},
    .forward_velocity = {{ .x = 0.0f, .y = 0.0f, .z = 0.0f }},
    .right = {{ .x = 1.0f, .y = 0.0f, .z = 0.0f }},
    .up = {{ .x = 0.0f, .y = 1.0f, .z = 0.0f }},
    .yaw_angle = 0.0,
    .pitch_angle = 0.0,
    .roll_angle = 0.0
};

shz_vec3_t get_camera_pos(void){
    return camera.position;
}

shz_vec3_t get_camera_dir(void){
    return camera.direction;
}
shz_vec3_t get_camera_vel(void){
    return camera.forward_velocity;
}

shz_vec3_t get_camera_right(){
    return camera.right;
}

shz_vec3_t get_camera_up(){
    return camera.up;
}

void set_camera_pos(shz_vec3_t pos){
  
    camera.position = pos;
}
void set_camera_dir(shz_vec3_t dir){
    camera.direction = dir;
}
void set_camera_vel(shz_vec3_t vel){
    camera.forward_velocity = vel;
}

void set_camera_right(shz_vec3_t right){
    camera.right = right;
}

void set_camera_up(shz_vec3_t up){
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

shz_vec3_t get_camera_lookat_target(){
    shz_vec3_t target = {{ .x = 0, .y = 0, .z = -1}};

    mat4_identity();
    mat_rotate_y(get_camera_yaw());
    mat_rotate_x(get_camera_pitch());

    shz_vec4_t camera_direction = (shz_vec4_t){0,0,0,0};

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
    float local_pitch   = get_camera_pitch();
    float local_yaw     = get_camera_yaw();

    camera.direction.x = cosf(local_yaw) * cosf(local_pitch);
    camera.direction.y = sinf(local_pitch);
    camera.direction.z = sinf(local_yaw) * cosf(local_pitch);
    camera.direction = shz_vec_normalize(camera.direction);
    // also re-calculate the Right and Up vector
    shz_vec3_t up = get_camera_up();
    camera.right = shz_vec_cross(camera.direction,up);
    camera.right = shz_vec_normalize(camera.right);
    up = shz_vec_cross(camera.right,camera.direction);
    camera.up = shz_vec_normalize(camera.up);

    set_camera_vel((shz_vec3_t){0,0,0});
}