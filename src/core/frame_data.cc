#include "core/frame_data.h"

PlayerFrameData::PlayerFrameData() :
    score(0),
    ojama(0)
{
}

PlayerFrameData::PlayerFrameData(const PlainField& field, const KumipuyoSeq& seq, const KumipuyoPos& pos,
                                 const UserState& userState, int score, int ojama) :
    field(field),
    kumipuyoSeq(seq),
    kumipuyoPos(pos),
    userState(userState),
    score(score),
    ojama(ojama)
{
}

FrameData::FrameData() :
    connectionLost(false),
    valid(false),
    id(-1)
{
}
