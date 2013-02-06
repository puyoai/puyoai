#include "game.h"

#include <cstring>
#include <glog/logging.h>
#include <sstream>
#include <string>

#include "base.h"
#include "player.h"

Game::Game(const std::string& name) : name_(name) {
  player_.reset(new Player());
  enemy_.reset(new Player());
}

Game::~Game() {}

bool Game::Input(const string& input) {
  Player players[2];

  istringstream iss(input);
  string key_val;
  while(getline(iss, key_val, ' ')) {
    size_t pos = key_val.find('=');
    const string key(key_val.substr(0, pos));
    const string value(key_val.substr(pos + 1));

    if (key.size() > 5) {
      LOG(WARNING) << "Unknown parameter : " << key_val << endl;
      continue;
    }

    if (key[0] != 'Y' && key[0] != 'O') {
      if (key == "ID") {
	id_ = atoi(value.c_str());
      } else if (key == "STATE") {
        int state = atoi(value.c_str());
	players[0].state = (state & Player::kAll);
	players[1].state = ((state >> 1) & Player::kAll);
      } else if (key == "ACK") {
	// Do nothing
      } else if (key == "NACK") {
	// Do nothing
      } else if (key == "END") {
        return false;
      } else {
	LOG(WARNING) << "Unknown field : " << key_val << endl;
      }
      continue;
    }

    Player& player = players[(key[0] == 'Y') ? 0 : 1];
    switch (key[1]) {
    case 'F':  // Field
      player.field.SetField(value);
      break;
    case 'P':  // Tsumo
      player.field.SetColorSequence(value);
      break;
    case 'S':  // Score
      player.score = atoi(value.c_str());
      break;
    case 'X':  // X-position of pivot puyo
      player.x = atoi(value.c_str());
      break;
    case 'Y':  // Y-position of pivot puyo
      player.y = atoi(value.c_str());
      break;
    case 'R':  // Rotation of controled puyo
      player.r = atoi(value.c_str());
      break;
    case 'O':  // The number of OjamaPuyo in stack
      player.ojama = atoi(value.c_str());
      break;
    }
  }

  // Copy if the status differs
  player_update_ = (players[0] != *player_);
  if (player_update_)
    player_->CopyFrom(players[0]);
  enemy_update_ = (players[1] != *enemy_);
  if (enemy_update_) {
    enemy_->CopyFrom(players[1]);
  }

  return true;
}

string Game::Play() {
  ostringstream oss;
  oss << "ID=" << id_;
  if (enemy_update_) {
    oss << " X=" << enemy_->x << " R=" << enemy_->r
	<< " MSG=" <<  enemy_->x << "," << enemy_->r;
  }

  return oss.str();
}
