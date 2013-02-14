#include "ai_manager.h"

#include "core/state.h"
#include "ai.h"
#include "drop_decision.h"
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
    int wnextAppearedDelay = 0;

    while (true) {
        Game game;

        if (!m_protocol.readCurrentStatus(&game)) {
            m_protocol.sendInputWithoutDecision(game.id);
            continue;
        }

        if (game.shouldInitialize())
            m_ai.initialize(game);

        // Update enemy info if necessary.
        if (game.state & (STATE_YOU_GROUNDED << 1))
            m_ai.enemyGrounded(game);
        if (game.state & (STATE_WNEXT_APPEARED << 1))
            m_ai.enemyWNextAppeared(game);

        // Update my info if necessary.
        if (game.state & STATE_CHAIN_DONE)
            m_ai.myRensaFinished(game);
        if (game.state & STATE_OJAMA_DROPPED)
            m_ai.myOjamaDropped(game);
        if (game.state & STATE_WNEXT_APPEARED)
            wnextAppearedDelay = 8;

        // Think if necessary
        if (game.shouldThink())
            needsThink = true;

        if (needsThink && (game.state & STATE_YOU_CAN_PLAY)) {
            DropDecision dropDecision;
            m_ai.think(dropDecision, game);
            needsThink = false;
            m_protocol.sendInputWithDecision(game.id, dropDecision);
            continue;
        }

        // TODO(mayah): Since STATE_WNEXT_APPEARED and STATE_YOU_CAN_PLAY 
        // can come simultaneously in duel/duel, we would like to postpone
        // calling wnextAppeared by a few frames.
        // If this issue is fixed, we have to fix this.
        // If we adopt multi thread, we don't need to do this.
        if (wnextAppearedDelay > 0) {
            if (--wnextAppearedDelay == 0)
                m_ai.wnextAppeared(game);
        }
    
        m_protocol.sendInputWithoutDecision(game.id);
    }

    return 0;
}
