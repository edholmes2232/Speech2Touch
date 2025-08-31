#ifndef TOUCH_MAPPER_H
#define TOUCH_MAPPER_H

#include "touch_targets.h"

// To be called before handling any targets
// Reset the target system to default state
void TOUCHMAPPER_ResetState(void);

void TOUCHMAPPER_HandleTarget(TARGET_T target);

#endif