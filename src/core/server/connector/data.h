#ifndef CORE_SERVER_CONNECTOR_DATA_H_
#define CORE_SERVER_CONNECTOR_DATA_H_

#include <string>

enum ConnectionStatus {
    UNREAD,
    OK,
    DIE,
};

struct Data {
    int id;
    int x; // TODO(mayah): Use Decision instead of x and r.
    int r;
    int usec;
    ConnectionStatus status;
    std::string original;
    std::string msg;
    std::string mawashi_area;

    Data() :
        id(0), x(0), r(0), usec(0), status(UNREAD) {
    }

    bool HasDecision() const {
        return (x != 0 || r != 0);
    }

    bool IsValid() const {
        if (status != OK)
            return false;

        // This pattern is OK.
        if (x == 0 && r == 0)
            return true;

        // Out of range
        if (x == 1 && r == 3)
            return false;
        if (x == 6 && r == 1)
            return false;

        return (1 <= x && x <= 6 && 0 <= r && r <= 3);
    }
};

#endif
