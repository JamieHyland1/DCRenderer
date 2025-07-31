#include <kos.h>
#include "../include/input.h"
#include "../include/camera.h"
#include "../include/pipeline.h"
#include "../include/scene.h"
uint32_t prev_buttons = 0;
uint8_t prev_ltrig = 0;
uint8_t prev_rtrig = 0;

//////////////////////////////////////
// Read input from Dreamcast Controller
// Quite simple, just reads the controller state
/////////////////////////////////////


void process_input(void)
{

    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (cont)
    {
        cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
        if (!state) return;

        uint32_t buttons = state->buttons;

        #define PRESSED(btn) ((buttons & (btn)) && !(prev_buttons & (btn)))

        if (PRESSED(CONT_A)) {
            // TODO
        }
        if (PRESSED(CONT_B)) {
            render_mode++;
        }
        if (PRESSED(CONT_X)) {
            render_mode--;
        }
        if (PRESSED(CONT_Y)) {
            isRunning = false;
        }
        if (state->buttons & CONT_DPAD_UP)
        {
            set_camera_vel(vec3_mult(get_camera_dir(),0.2f));
            set_camera_pos(vec3_add(get_camera_pos(),get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_DOWN)
        {
            set_camera_vel(vec3_mult(get_camera_dir(),0.2f));
            set_camera_pos(vec3_sub(get_camera_pos(),get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_LEFT)
        {
            vec3f_t vel = (vec3_cross(get_camera_dir(), get_camera_up()));
            vel = vec_normalize(vel);
            vel = vec3_mult(vel,0.2f);
            set_camera_vel(vel);
            set_camera_pos(vec3_add(get_camera_pos(),get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_RIGHT)
        {
            vec3f_t vel = (vec3_cross(get_camera_dir(), get_camera_up()));
            vel = vec_normalize(vel);
            vel = vec3_mult(vel,0.2f);
            set_camera_vel(vel);
            set_camera_pos(vec3_sub(get_camera_pos(),get_camera_vel()));
        }

        // Trigger edge detection
        if (state->rtrig > 200 && prev_rtrig <= 200) {
            render_mode++;
        }
        if (state->ltrig > 200 && prev_ltrig <= 200) {
            render_mode--;
        }

        if (render_mode < 0) {
            render_mode = 5;
        } else if (render_mode > 5) {
            render_mode = 0;
        }

        prev_buttons = buttons;
        prev_ltrig = state->ltrig;
        prev_rtrig = state->rtrig;
    }
}