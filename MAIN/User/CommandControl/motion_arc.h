#ifndef MOTION_ARC_H
#define MOTION_ARC_H

#include "Jaws_main.h"

#define MM_PER_ARC_SEGMENT 1
#define N_ARC_CORRECTION 25

namespace command{

void mc_arc(float *position, float *target, float *offset, unsigned char axis_0, unsigned char axis_1, unsigned char axis_linear, float feed_rate, float radius, unsigned char isclockwise, uint8_t extruder);

}

#endif /* MOTION_ARC_H */
