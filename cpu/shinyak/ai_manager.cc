#include "ai_manager.h"

#include "../../core/state.h"
#include "ai.h"
#include "decision.h"
#include "game.h"
#include "protocol.h"

AIManager::AIManager(const std::string& name)
    : m_name(name)
    , m_ai(name)
{
}

int AIManager::runLoop()
{
    bool needsThink = true;

    while (true) {
        Game game;

        if (!m_protocol.readCurrentStatus(&game)) {
            m_protocol.sendInput(game.id, NULL);
            continue;
        }

        if (game.shouldInitialize())
            m_ai.initialize(game);

        // Update enemy info if necessary.
        if (game.enemyHasPutPuyo())
            m_ai.enemyGrounded(game);
        if (game.enemyWNextAppeared())
            m_ai.enemyWNextAppeared(game);

        // Update my info if necessary.
        if (game.state & STATE_CHAIN_DONE)
            m_ai.myRensaFinished(game);
        if (game.state & STATE_OJAMA_DROPPED)
            m_ai.myOjamaDropped(game);

        // Think if necessary
        if (game.shouldThink())
            needsThink = true;

        if (needsThink && game.canPlay()) {
            Decision decision;
            m_ai.think(decision, game);
            needsThink = false;
            m_protocol.sendInput(game.id, &decision);
            continue;
        }
    
        m_protocol.sendInput(game.id, NULL);
    }

    return 0;
}
