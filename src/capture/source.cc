#include "source.h"

#include <gflags/gflags.h>

#include <SDL_image.h>

Source::Source() :
    ok_(false),
    done_(false),
    width_(-1),
    height_(-1)
{
}
