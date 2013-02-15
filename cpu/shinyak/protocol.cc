#include "protocol.h"

#include "drop_decision.h"
#include "game.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace std;

void Protocol::sendInputWithoutDecision(int id)
{
    std::cout << "ID=" << id << std::endl;
}

void Protocol::sendInputWithDecision(int id, const DropDecision& dropDecision)
{
    std::cout << "ID=" << id
              << " X=" << dropDecision.decision().x
              << " R=" << dropDecision.decision().r
              << " MSG=" << dropDecision.message() << endl;
}

bool Protocol::readCurrentStatus(Game* game) {
    std::string line, term;
    std::getline(std::cin, line);

    LOG(INFO) << line;

    bool valid = false;
    for (std::istringstream iss(line); iss >> term;) {
        if (term.find('=') == std::string::npos) {
            continue;
        }
        
        const char* key = term.c_str();
        const char* value = term.c_str() + term.find('=') + 1;
        if (std::strncmp(key, "STATE", 5) == 0) {
            game->state = strtoull(value, NULL, 10);
            continue;
        } else if (std::strncmp(key, "ID", 2) == 0) {
            game->id = std::atoi(value);
            valid = true;
            continue;
        }

        PlayerState& playerState = game->playerStates[(key[0] == 'Y') ? 0 : 1];
        switch (key[1]) {
        case 'F':
            playerState.field = Field(std::string(value));
            break;
        case 'P':
            setKumiPuyo(std::string(value), playerState.kumiPuyos);
            break;
        case 'S':
            playerState.score = std::atoi(value);
            break;
        case 'X':
        case 'Y':
        case 'R':
            break;
        case 'O':
            playerState.ojama = std::atoi(value);
            break;
        }
    }
    
    return valid;
}
