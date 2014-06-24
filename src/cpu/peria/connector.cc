#include "cpu/peria/connector.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "cpu/peria/game.h"

namespace peria {

Connector::Connector() {
  // Echo process
  std::string line;
  std::getline(std::cin, line);
  std::cout << line << std::endl;
}

Connector::~Connector() {}

bool Connector::Receive(Game* game) {
  std::string line;
  if (!std::getline(std::cin, line))
    return false;

  std::istringstream iss(line);
  for (std::string term; iss >> term;) {
    if (term.find('=') == std::string::npos)
      continue;
    std::istringstream isterm(term);
    const char* key = term.c_str();
    const char* value = term.c_str() + term.find('=') + 1;
    if (std::strncmp(key, "STATE=", 6) == 0) {
      // TODO(peria): Get status for each player.
      continue;
    } else if (std::strncmp(key, "ID=", 3) == 0) {
      game->id = std::atoi(value);
      continue;
    }
  }

  return true;
}

void Connector::Send() {
}

}  // namespace peria
