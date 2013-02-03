#include "game.h"

#include <cstring>
#include <sstream>
#include <string>

#include "base.h"

namespace {
const int kNone = 0;
const int kPlay = 1 << 0;
const int kNext2 = 1 << 2;
const int kSet = 1 << 4;
const int kWin = 1 << 6;
const int kChainEnd = 1 << 8;
const int kAll = 0x55555555u;
}

void Player::CopyFrom(const Player& player) {
  for (int i = 1; i <= Field::kWidth; ++i) {
    for (int j = 1; j <= Field::kHeight; ++j) {
      this->field.Set(i, j, field.Get(i, j));
    }
  }
  state = player.state;
  score = player.score;
  ojama = player.ojama;
  x = player.x;
  y = player.y;
  r = player.r;
}

Game::Game(const std::string& name)
  : name_(name) {
  player_.reset(new Player());
  enemy_.reset(new Player());
  log_.open((name + ".log").c_str());
}

Game::~Game() {
  log_.close();
}

bool Game::Input(const string& input) {
  Player players[2];

  istringstream iss(input);
  string key_val;
  while(getline(iss, key_val, ' ')) {
    size_t pos = key_val.find('=');
    const string key(key_val.substr(0, pos));
    const string value(key_val.substr(pos + 1));

    if (key.size() > 5) {
      log_ << "Unknown parameter : " << key_val << endl;
      continue;
    }

    if (key[0] != 'Y' && key[0] != 'O') {
      if (key == "ID") {
	id_ = atoi(value.c_str());
      } else if (key == "STATE") {
        int state = atoi(value.c_str());
	players[0].state = (state & kAll);
	players[1].state = ((state >> 1) & kAll);
      } else if (key == "ACK") {
	// Do nothing
      } else if (key == "NACK") {
	// Do nothing
      } else if (key == "END") {
        return false;
      } else {
	log_ << "Unknown field : " << key_val << endl;
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

bool operator==(const Player& a, const Player& b) {
  if (a.state != b.state) return false;
  if (a.score != b.score) return false;
  if (a.ojama != b.ojama) return false;
  if (a.x != b.x) return false;
  if (a.y != b.y) return false;
  if (a.r != b.r) return false;

  // Compare visible field
  if (!a.field.EqualTo(b.field, true))
    return false;

  return true;
}

bool operator!=(const Player& a, const Player& b) {
  return !(a == b);
}
