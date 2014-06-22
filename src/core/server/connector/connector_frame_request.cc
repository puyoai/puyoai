#include "core/server/connector/connector_frame_request.h"

#include <sstream>

using namespace std;

static char toCompatibleChar(PuyoColor pc)
{
    switch (pc) {
    case PuyoColor::EMPTY:  return '0';
    case PuyoColor::OJAMA:  return '1';
    case PuyoColor::WALL:   return '2';
    case PuyoColor::RED:    return '4';
    case PuyoColor::BLUE:   return '5';
    case PuyoColor::YELLOW: return '6';
    case PuyoColor::GREEN:  return '7';
    default:
        LOG(WARNING) << "Unknown PuyoColor: " << static_cast<int>(pc);
        return '?';
    }
}

static string formatPlainField(const PlainField& field)
{
    stringstream ss;
    for (int y = 12; y >= 1; y--) {
        for (int x = 1; x <= 6; x++) {
            ss << toCompatibleChar(field.get(x, y));
        }
    }
    return ss.str();
}

static string formatNext(const Kumipuyo kumipuyo[3])
{
    stringstream ss;
    ss << toCompatibleChar(kumipuyo[0].axis);
    ss << toCompatibleChar(kumipuyo[0].child);
    ss << toCompatibleChar(kumipuyo[1].axis);
    ss << toCompatibleChar(kumipuyo[1].child);
    ss << toCompatibleChar(kumipuyo[2].axis);
    ss << toCompatibleChar(kumipuyo[2].child);
    return ss.str();
}

static std::string formatAckNack(int ackFrameId, const vector<int>& nackFrameIds)
{
    stringstream nack;
    bool hasNack = false;
    for (size_t i = 0; i < nackFrameIds.size(); i++) {
        if (i != 0)
            nack << ",";
        nack << nackFrameIds[i];
        hasNack = true;
    }

    stringstream ret;
    if (ackFrameId > 0)
        ret << "ACK=" << ackFrameId << " ";
    if (hasNack)
        ret << "NACK=" << nack.str();
    return ret.str();
}

string ConnectorFrameRequest::toRequestString(int playerId) const
{
    int opponentId = 1 - playerId;

    std::string f0 = formatPlainField(field[playerId]);
    std::string f1 = formatPlainField(field[opponentId]);
    std::string y0 = formatNext(kumipuyo[playerId]);
    std::string y1 = formatNext(kumipuyo[opponentId]);
    int score0 = score[playerId];
    int score1 = score[opponentId];
    int ojama0 = ojama[playerId];
    int ojama1 = ojama[opponentId];
    int state0 = userState[playerId].toDeprecatedState();
    int state1 = userState[opponentId].toDeprecatedState();
    KumipuyoPos pos0 = kumipuyoPos[playerId];
    KumipuyoPos pos1 = kumipuyoPos[opponentId];

    string win;
    switch (gameResult) {
    case GameResult::PLAYING:
        break;
    case GameResult::P1_WIN:
        win = "END=1 ";
        break;
    case GameResult::P2_WIN:
        win = "END=-1 ";
        break;
    default:
        win = "END=0 ";
        break;
    }

    std::string ack = formatAckNack(ackFrameId[playerId], nackFrameIds[playerId]);

    stringstream ss;
    ss << "ID=" << frameId << " "
       << "STATE=" << (state0 + (state1 << 1)) << " "
       << "YF=" << f0 << " "
       << "OF=" << f1 << " "
       << "YP=" << y0 << " "
       << "OP=" << y1 << " "
       << "YX=" << pos0.axisX() << " "
       << "YY=" << pos0.axisY() << " "
       << "YR=" << pos0.r << " "
       << "OX=" << pos1.axisX() << " "
       << "OY=" << pos1.axisY() << " "
       << "OR=" << pos1.r << " "
       << "YO=" << ojama0 << " "
       << "OO=" << ojama1 << " "
       << "YS=" << score0 << " "
       << "OS=" << score1 << " "
       << win
       << ack;
    return ss.str();
}
