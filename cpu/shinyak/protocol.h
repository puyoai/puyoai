#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <iostream>
#include <fstream>

class Game;
class Decision;

class Protocol {
public:
    // Reads current status from input
    bool readCurrentStatus(Game* game, std::ofstream& log);

    // Sends KEY input.
    void sendInput(const int id, const Decision* decision);    
};

#endif
