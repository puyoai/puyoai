#ifndef __PROTOCOL_H
#define __PROTOCOL_H

class Game;
class Decision;

class Protocol {
public:
    // Reads current status from input
    bool readCurrentStatus(Game* game);

    // Sends KEY input.
    void sendInput(const int id, const Decision* decision);    
};

#endif
