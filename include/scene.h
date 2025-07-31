#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

extern int frame_count;
extern bool isRunning;

bool init_sceene(void);
void update(void);

#endif