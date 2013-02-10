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
	players[0].set_state(state & Player::kAll);
	players[1].set_state((state >> 1) & Player::kAll);
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
      player.mutable_field()->SetField(value);
      break;
    case 'P':  // Tsumo
      player.mutable_field()->SetColorSequence(value);
      break;
    case 'S':  // Score
      player.set_score(atoi(value.c_str()));
      break;
    case 'X':  // X-position of pivot puyo
      player.set_x(atoi(value.c_str()));
      break;
    case 'Y':  // Y-position of pivot puyo
      player.set_y(atoi(value.c_str()));
      break;
    case 'R':  // Rotation of controled puyo
      player.set_r(atoi(value.c_str()));
      break;
    case 'O':  // The number of OjamaPuyo in stack
      player.set_ojama(atoi(value.c_str()));
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
  if (!enemy_update_) {
    int max_score = 0;
    int x = 0, r = 0;
    vector<Player> children;
    player_->Search(&children);
    for (size_t i = 0; i < children.size(); ++i) {
      if (max_score < children[i].score()) {
        max_score = children[i].score();
        x = children[i].get_x();
        r = children[i].get_r();
      }
    }
    if (x > 0) {
      oss << " X=" << x << " R=" << r
          << " MSG=MyControl";
    }
  } else {
    oss << " X=" << enemy_->get_x() << " R=" << enemy_->get_r()
	<< " MSG=" <<  enemy_->get_x() << "," << enemy_->get_r();
  }

  return oss.str();
}
