#include "core/frame_data.h"

#include "core/field/field_pretty_printer.h"

#include <sstream>

using namespace std;

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

string FrameData::toString() const
{
    stringstream ss;
    ss << "ID=" << id << endl;
    ss << FieldPrettyPrinter::toStringFromMultipleFields(
        playerFrameData[0].field, playerFrameData[0].kumipuyoSeq,
        playerFrameData[1].field, playerFrameData[1].kumipuyoSeq) << endl;

    return ss.str();
}
