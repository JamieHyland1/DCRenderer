#ifndef CAMERA_H
#define CAMERA_H
#include "vector.h"
#include "shz_vector.h"
typedef struct{
    shz_vec3_t position;
    shz_vec3_t direction;
    shz_vec3_t forward_velocity;
    shz_vec3_t right;
    shz_vec3_t up;
    float yaw_angle;
    float pitch_angle;
    float roll_angle;

} camera_t;


shz_vec3_t get_camera_lookat_target(void);

shz_vec3_t get_camera_pos(void);
shz_vec3_t get_camera_dir(void);
shz_vec3_t get_camera_vel(void);
shz_vec3_t get_camera_right(void);
shz_vec3_t get_camera_up(void);

void set_camera_pos(shz_vec3_t pos);
void set_camera_dir(shz_vec3_t dir);
void set_camera_vel(shz_vec3_t vel);
void set_camera_right(shz_vec3_t right);
void set_camera_up(shz_vec3_t up);

float  get_camera_yaw(void);
float  get_camera_pitch(void);
float  get_camera_roll(void);

void  rotate_camera_yaw(float angle);
void  rotate_camera_pitch(float angle);
void  rotate_camera_roll(float angle);

void update_camera_vectors();
void process_mouse_move(float xPos, float yPos, float delta_time);

#endif