#ifndef __PROTOCOL_H
#define __PROTOCOL_H

class Game;
class DropDecision;

class Protocol {
public:
    // Reads current status from input
    bool readCurrentStatus(Game* game);

    // Sends KEY input.
    void sendInputWithoutDecision(int id);
    void sendInputWithDecision(int id, const DropDecision&);
};

#endif
