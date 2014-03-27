#include "game.h"

#include <cstring>
#include <glog/logging.h>
#include <sstream>
#include <string>

#include "base.h"
#include "player.h"

Game::Game(const std::string& name) : name_(name) {
  players_.resize(2, NULL);
  players_[0] = new Player();
  if (name != "HITOPUYO") {
    players_[1] = new Player();
    players_[0]->set_opposite(players_[1]);
  }
}

Game::~Game() {
  delete(players_[0]);
  delete(players_[1]);
}

bool Game::Input(const string& input) {
  istringstream iss(input);
  string key_val;
  while (getline(iss, key_val, ' ')) {
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
	players_[0]->set_state(state & Player::kAll);
        if (players_[1])
          players_[1]->set_state((state >> 1) & Player::kAll);
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

    Player* player = players_[(key[0] == 'Y') ? 0 : 1];
    switch (key[1]) {
    case 'F':  // Field
      player->mutable_field()->SetField(value);
      break;
    case 'P':  // Tsumo
      player->SetColorSequence(value);
      break;
    case 'S':  // Score
      player->set_score(atoi(value.c_str()));
      break;
    case 'X':  // X-position of pivot puyo
      player->set_x(atoi(value.c_str()));
      break;
    case 'Y':  // Y-position of pivot puyo
      player->set_y(atoi(value.c_str()));
      break;
    case 'R':  // Rotation of controled puyo
      player->set_r(atoi(value.c_str()));
      break;
    case 'O':  // The number of OjamaPuyo in stack
      player->set_ojama(atoi(value.c_str()));
      break;
    }
  }

  return true;
}

string Game::Play() {
  ostringstream oss;
  oss << "ID=" << id_;

  Player::Control control;
  string message;
  players_[0]->GetControl(&control, &message);
  oss << " X=" << control.first
      << " R=" << control.second
      << " MSG=" << message;

  return oss.str();
}
