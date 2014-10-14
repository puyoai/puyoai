#include "core/client/ai/endless/endless.h"

#include <algorithm>

#include "core/frame_request.h"
#include "core/field_pretty_printer.h"

using namespace std;

Endless::Endless(unique_ptr<AI> ai) : ai_(std::move(ai))
{
}

int Endless::run(const KumipuyoSeq& seq)
{
    // Initialize ai.
    FrameRequest req;
    req.frameId = 1;
    ai_->gameWillBegin(req);

    req.playerFrameRequest[0].kumipuyoSeq = seq;
    req.playerFrameRequest[1].kumipuyoSeq = seq;

    setEnemyField(&req);

    int maxRensaScore = 0;
    for (int i = 0; i < 50; ++i) {
        req.frameId = i + 2;

        // For gazing.
        ai_->gaze(req.frameId, CoreField(req.enemyPlayerFrameRequest().field), req.enemyPlayerFrameRequest().kumipuyoSeq);

        // think
        ai_->next2Appeared(req);
        ai_->decisionRequested(req);

        DropDecision dropDecision = ai_->think(req.frameId, req.myPlayerFrameRequest().field,
                                               req.myPlayerFrameRequest().kumipuyoSeq, ai_->additionalThoughtInfo());

        CoreField f(req.myPlayerFrameRequest().field);
        if (!f.dropKumipuyo(dropDecision.decision(), req.myPlayerFrameRequest().kumipuyoSeq.front())) {
            // couldn't drop. break.
            break;
        }

        if (verbose_) {
            FieldPrettyPrinter::print(f, req.playerFrameRequest[0].kumipuyoSeq.subsequence(1));
        }

        RensaResult rensaResult = f.simulate();
        maxRensaScore = std::max(maxRensaScore, rensaResult.score);
        if (f.color(3, 12) != PuyoColor::EMPTY) {
            maxRensaScore = -1;
            break;
        }
        if (rensaResult.score > 10000) {
            // The main rensa must be fired.
            break;
        }
        req.playerFrameRequest[0].field = f;
        req.playerFrameRequest[0].kumipuyoSeq.dropFront();
        req.playerFrameRequest[1].kumipuyoSeq.dropFront();

        ai_->grounded(req);
    }

    return maxRensaScore;
}

void Endless::setEnemyField(FrameRequest* req)
{
    req->playerFrameRequest[1].field = PlainField(
            "Y   GY"
            "R   GG"
            "YRYGRY"
            "RYGRYY"
            "YRYGRG"
            "YRYGRG"
            "YGRYGR"
            "RYGRYG"
            "RYGRYG"
            "RYGRYG");
}
