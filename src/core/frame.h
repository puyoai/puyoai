#ifndef CORE_FRAME_H_
#define CORE_FRAME_H_

#include <glog/logging.h>

// Returns the number of animation frames when ojama is grounding
inline int framesGroundingOjama(int numOjama)
{
    if (numOjama <= 0)
        return 0;

    // TODO(mayah): This is not accurate.

    if (numOjama <= 3)
        return 4;

    if (numOjama <= 18)
        return 12;

    return 16;
}

#endif
