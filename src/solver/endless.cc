#include "solver/endless.h"

#include <algorithm>
#include <iostream>

#include "base/time.h"
#include "core/frame_request.h"
#include "core/field_pretty_printer.h"

using namespace std;

Endless::Endless(unique_ptr<AI> ai) : ai_(std::move(ai))
{
}

EndlessResult Endless::run(const KumipuyoSeq& originalSeq)
{
    // Initialize ai.
    FrameRequest req;
    req.frameId = 1;
    ai_->gameWillBegin(req);

    {
        KumipuyoSeq seq = originalSeq.subsequence(0, 2);
        req.playerFrameRequest[0].kumipuyoSeq = seq;
        req.playerFrameRequest[1].kumipuyoSeq = seq;
    }

    setEnemyField(&req);

    ai_->enemy_.field = CoreField(req.playerFrameRequest[1].field);
    ai_->enemy_.seq = req.playerFrameRequest[1].kumipuyoSeq;

    vector<Decision> decisions;

    int maxRensaScore = 0;
    int maxRensa = 0;
    for (int i = 0; i < 50; ++i) {
        req.frameId = i + 2;
        req.playerFrameRequest[0].kumipuyoSeq = originalSeq.subsequence(i, 2);
        req.playerFrameRequest[1].kumipuyoSeq = originalSeq.subsequence(i, 2);

        if (verbose_) {
            FieldPrettyPrinter::print(req.myPlayerFrameRequest().field, req.playerFrameRequest[0].kumipuyoSeq);
        }

        // For gazing.
        ai_->gaze(req.frameId, CoreField(req.enemyPlayerFrameRequest().field), req.enemyPlayerFrameRequest().kumipuyoSeq);

        // think
        ai_->next2AppearedForMe(req);
        ai_->decisionRequestedForMe(req);

        double beginTime = currentTime();

        DropDecision dropDecision = ai_->think(req.frameId,
                                               CoreField(req.myPlayerFrameRequest().field),
                                               req.myPlayerFrameRequest().kumipuyoSeq,
                                               ai_->myPlayerState(),
                                               ai_->enemyPlayerState(),
                                               false);

        double endTime = currentTime();

        CoreField f(req.myPlayerFrameRequest().field);
        if (!f.dropKumipuyo(dropDecision.decision(), req.myPlayerFrameRequest().kumipuyoSeq.front())) {
            // couldn't drop. break.
            break;
        }

        if (verbose_) {
            cout << "time=" << (endTime - beginTime) << endl;
        }

        decisions.push_back(dropDecision.decision());

        RensaResult rensaResult = f.simulate();
        maxRensaScore = std::max(maxRensaScore, rensaResult.score);
        maxRensa = std::max(maxRensa, rensaResult.chains);
        if (f.color(3, 12) != PuyoColor::EMPTY) {
            return EndlessResult {
                i,                         // .hand
                -1,                        // .score
                -1,                        // .maxRensa
                false,                     // .zenkeshi
                decisions,                 // .decisions
                EndlessResult::Type::DEAD, // .type
            };
        }
        if (rensaResult.score > 10000) {
            // The main rensa must be fired.
            return EndlessResult {
                i,                               // .hand
                rensaResult.score,               // .score
                rensaResult.chains,              // .maxRensa
                f.isZenkeshi(),                  // .zenkeshi
                decisions,                       // .decisions
                EndlessResult::Type::MAIN_CHAIN, // .type
            };
        }
        if (f.isZenkeshi()) {
            return EndlessResult {
                i,                             // .hand
                rensaResult.score,             // .score
                rensaResult.chains,            // .maxRensa
                true,                          // .zenkeshi
                decisions,                     // .decisions
                EndlessResult::Type::ZENKESHI, // .type

            };
        }

        req.playerFrameRequest[0].field = f.toPlainField();

        // Update the current field.
        ai_->me_.field = f;

        ai_->groundedForMe(req);
    }

    return EndlessResult {
        50,                                  // .hand
        maxRensaScore,                       // .score
        maxRensa,                            // .maxRensa
        false,                               // .zenkeshi
        decisions,                           // .decisions
        EndlessResult::Type::PUYOSEQ_RUNOUT, // .type
    };
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
