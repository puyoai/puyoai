#include <iostream>

#include "glog/logging.h"
#include <stdlib.h>

class RandomPlayer {
private:
  std::string id;
  int state;
  int x;
  int rotation;
  
public:
  RandomPlayer() : state(0), x(-1), rotation(-1) {
  }

  void processPiece(const std::string& piece) {
    const size_t pos = piece.find('=');
    if (pos == std::string::npos) {
      LOG(ERROR) << "Don't know how to handle this: " << piece;
      return;
    }

    const std::string left = piece.substr(0, pos);
    const std::string right = piece.substr(pos + 1, piece.length() - pos - 1);
    LOG(INFO) << left << " " << right;
    if (left == "ID") {
      id = piece;
    } else if (left == "STATE") {
      state = atoi(right.c_str());
    }
  }

  void processLine(const std::string& line) {
    LOG(INFO) << line;
    size_t prev = 0;
    size_t end;
    while ((end = line.find(' ', prev)) != std::string::npos) {
      processPiece(line.substr(prev, end - prev));
      prev = end + 1;
    }
    if (prev < line.size()) {
      processPiece(line.substr(prev, line.size() - prev));
    }
    
    if ((state & (1 << 4)) != 0) {
      x = -1;
    }
    if (x < 0) {
      rotation = rand() % 4;
      x = (((rotation == 0 || rotation == 2) ? rand() % 6 :
	    (rotation == 1 ? rand() % 5 : rand() % 5 + 1))) + 1;
    }
    std::cout << id << " X=" << x << " R=" << rotation
	      << std::endl << std::flush;
    LOG(INFO) << x << " " << rotation;
  }
};

int main(int /*argc*/, char **argv) {
  google::InitGoogleLogging(argv[0]);

  RandomPlayer player;
  std::string line;

  // Process ping.
  std::getline(std::cin, line, '\n');
  std::cout << line << std::endl << std::flush;

  while (std::getline(std::cin, line, '\n')) {
    player.processLine(line);
  }

  return 0;
}
