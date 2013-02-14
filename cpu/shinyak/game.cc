#include "game.h"

#include <glog/logging.h>
#include "../../core/state.h"

using namespace std;

Game::Game() :
    id(0),
    state(0)
{
}

Game::~Game()
{
}

bool Game::shouldInitialize() const
{
    return id == 1;
}

bool Game::shouldThink() const
{
    if (shouldInitialize())
        return true;
    return state & STATE_YOU_GROUNDED;
}


