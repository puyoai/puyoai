#include "gui/human_connector_key_listener.h"

using namespace std;

HumanConnectorKeyListener::HumanConnectorKeyListener(HumanConnector* c) :
    connector_(c)
{
}

HumanConnectorKeyListener::~HumanConnectorKeyListener()
{
}

void HumanConnectorKeyListener::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_x)
            keySet_.rightTurnKey = true;
        if (event.key.keysym.sym == SDLK_z)
            keySet_.leftTurnKey = true;
    }
}

void HumanConnectorKeyListener::handleAfterPollEvent()
{
    const Uint8* state = SDL_GetKeyboardState(NULL);

    keySet_.downKey = state[SDL_SCANCODE_DOWN];
    keySet_.leftKey = state[SDL_SCANCODE_LEFT];
    keySet_.rightKey = state[SDL_SCANCODE_RIGHT];
    connector_->setKeySet(keySet_);

    keySet_ = KeySet();
}


