#include "core/frame_request.h"

#include <sstream>

#include "core/field_pretty_printer.h"

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

static string formatNext(const KumipuyoSeq& seq)
{
    stringstream ss;
    if (seq.size() >= 1) {
        ss << toCompatibleChar(seq.get(0).axis);
        ss << toCompatibleChar(seq.get(0).child);
    }
    if (seq.size() >= 2) {
        ss << toCompatibleChar(seq.get(1).axis);
        ss << toCompatibleChar(seq.get(1).child);
    }
    if (seq.size() >= 3) {
        ss << toCompatibleChar(seq.get(2).axis);
        ss << toCompatibleChar(seq.get(2).child);
    }
    return ss.str();
}

static GameResult parseEnd(const char* value)
{
    int x = std::atoi(value);
    if (x == 1) {
        return GameResult::P1_WIN;
    } else if (x == 0) {
        return GameResult::DRAW;
    } else if (x == -1) {
        return GameResult::P2_WIN;
    }

    return GameResult::PLAYING;
}

// static
FrameRequest FrameRequest::parse(const std::string& line)
{
    FrameRequest req;

    string term;
    for (std::istringstream iss(line); iss >> term;) {
        if (term.find('=') == std::string::npos)
            continue;

        const char* key = term.c_str();
        const char* value = term.c_str() + term.find('=') + 1;
        if (strncmp(key, "STATE", 5) == 0) {
            int state = strtoull(value, NULL, 10);
            req.playerFrameRequest[0].state.parseFromDeprecatedState(state);
            req.playerFrameRequest[1].state.parseFromDeprecatedState(state >> 1);
            continue;
        } else if (strncmp(key, "ID", 2) == 0) {
            req.frameId = std::atoi(value);
            continue;
        } else if (strncmp(key, "END", 3) == 0) {
            req.gameResult = parseEnd(value);
            continue;
        }

        PlayerFrameRequest& pReq = req.playerFrameRequest[(key[0] == 'Y') ? 0 : 1];
        switch (key[1]) {
        case 'F':
            pReq.field = PlainField(string(value));
            break;
        case 'P':
            pReq.kumipuyoSeq = KumipuyoSeq(string(value));
            break;
        case 'S':
            pReq.score = std::atoi(value);
            break;
        case 'X':
            pReq.kumipuyoPos.x = std::atoi(value);
            break;
        case 'Y':
            pReq.kumipuyoPos.y = std::atoi(value);
            break;
        case 'R':
            pReq.kumipuyoPos.r = std::atoi(value);
            break;
        case 'O':
            pReq.ojama = std::atoi(value);
            break;
        }
    }

    VLOG(1) << req.toDebugString();

    return req;
}

string FrameRequest::toDebugString() const
{
    stringstream ss;
    ss << "ID=" << frameId << endl;
    ss << FieldPrettyPrinter::toStringFromMultipleFields(
        playerFrameRequest[0].field, playerFrameRequest[0].kumipuyoSeq,
        playerFrameRequest[1].field, playerFrameRequest[1].kumipuyoSeq) << endl;

    return ss.str();
}

string FrameRequest::toString() const
{
    const PlayerFrameRequest& me = playerFrameRequest[0];
    const PlayerFrameRequest& op = playerFrameRequest[1];

    std::string f0 = formatPlainField(me.field);
    std::string f1 = formatPlainField(op.field);
    std::string y0 = formatNext(me.kumipuyoSeq);
    std::string y1 = formatNext(op.kumipuyoSeq);
    int score0 = me.score;
    int score1 = op.score;
    int ojama0 = me.ojama;
    int ojama1 = op.ojama;
    int state0 = me.state.toDeprecatedState();
    int state1 = op.state.toDeprecatedState();
    KumipuyoPos pos0 = me.kumipuyoPos;
    KumipuyoPos pos1 = op.kumipuyoPos;

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
       << win;
    return ss.str();
}
