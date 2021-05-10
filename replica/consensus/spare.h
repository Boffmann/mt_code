#ifndef __SPARE_H__
#define __SPARE_H__

#include <stdbool.h>

/**
 * Reads the ActivateSpare topic and activates (follower) or deactivates (spare) this_replica accordingly
 * 
 * @return true when replica got activated, false otherwise
 */
bool activate_when_promted();

#endif