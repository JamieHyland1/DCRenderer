#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct {
    vec3f_t position;
    vec3f_t direction;
    vec3f_t forward_velocity;
    vec3f_t right;
    vec3f_t up;
    float yaw_angle;
    float pitch_angle;
    float roll_angle;

} camera_t;

vec3f_t get_camera_lookat_target(void);

vec3f_t get_camera_pos(void);
vec3f_t get_camera_dir(void);
vec3f_t get_camera_vel(void);
vec3f_t get_camera_right(void);
vec3f_t get_camera_up(void);

void set_camera_pos(vec3f_t pos);
void set_camera_dir(vec3f_t dir);
void set_camera_vel(vec3f_t vel);
void set_camera_right(vec3f_t right);
void set_camera_up(vec3f_t up);

float get_camera_yaw(void);
float get_camera_pitch(void);
float get_camera_roll(void);

void rotate_camera_yaw(float angle);
void rotate_camera_pitch(float angle);
void rotate_camera_roll(float angle);

void update_camera_vectors();
void process_mouse_move(float xPos, float yPos, float delta_time);

#endif