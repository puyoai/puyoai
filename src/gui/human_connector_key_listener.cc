#include "gui/human_connector_key_listener.h"

#include "core/server/connector/human_connector.h"

using namespace std;

HumanConnectorKeyListener::HumanConnectorKeyListener(HumanConnector* c) :
    connector_(c)
{
}

HumanConnectorKeyListener::~HumanConnectorKeyListener()
{
}

void HumanConnectorKeyListener::handleAfterPollEvent()
{
    const Uint8* state = SDL_GetKeyboardState(NULL);

    KeySet keySet;
    keySet.downKey = state[SDL_SCANCODE_DOWN];
    keySet.leftKey = state[SDL_SCANCODE_LEFT];
    keySet.rightKey = state[SDL_SCANCODE_RIGHT];
    keySet.rightTurnKey = state[SDL_SCANCODE_X];
    keySet.leftTurnKey = state[SDL_SCANCODE_Z];

    connector_->setKeySet(keySet);
}


