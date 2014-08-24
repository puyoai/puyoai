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
            keySet_.setKey(Key::KEY_RIGHT_TURN);
        if (event.key.keysym.sym == SDLK_z)
            keySet_.setKey(Key::KEY_LEFT_TURN);
    }
}

void HumanConnectorKeyListener::handleAfterPollEvent()
{
    const Uint8* state = SDL_GetKeyboardState(NULL);

    keySet_.setKey(Key::KEY_DOWN, state[SDL_SCANCODE_DOWN]);
    keySet_.setKey(Key::KEY_LEFT, state[SDL_SCANCODE_LEFT]);
    keySet_.setKey(Key::KEY_RIGHT, state[SDL_SCANCODE_RIGHT]);
    connector_->setKeySet(keySet_);

    keySet_ = KeySet();
}


