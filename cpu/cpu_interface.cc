#include "cpu/cpu_interface.h"
#include <iostream>
#include <sstream>
#include <string>

#include "core/decision.h"

using namespace std;

void Cpu::Run() {
  // Make sure the CPU is connected to the duel server.
  // Echo back what we receive.
  // MAKE SURE to flush.
  Data data;
  ReceiveCurrentStatus(&data);
  cout << data.original << endl << flush;

  while (true) {
    ReceiveCurrentStatus(&data);

    Decision decision;
    string message;
    GetDecision(data, &decision, &message);
    SendDecision(data, decision, message);
  }
}

void Cpu::ReceiveCurrentStatus(Data* data) const {
  string str;
  getline(cin, str, '\n');

  istringstream iss(str);
  string tmp;
  data->original = string(str);
  while(getline(iss, tmp, ' ')) {
    if (tmp.substr(0, 3) == "ID=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->id;
    } else if (tmp.substr(0, 3) == "YX=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[0].x;
    } else if (tmp.substr(0, 3) == "YY=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[0].y;
    } else if (tmp.substr(0, 3) == "YR=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[0].r;
    } else if (tmp.substr(0, 3) == "OX=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[1].x;
    } else if (tmp.substr(0, 3) == "OY=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[1].y;
    } else if (tmp.substr(0, 3) == "OR=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[1].r;
    } else if (tmp.substr(0, 3) == "YP=") {
      istringstream istr(tmp.c_str() + 3);
      string yokoku;
      istr >> yokoku;
      data->player[0].field.SetColorSequence(yokoku);
    } else if (tmp.substr(0, 3) == "OP=") {
      istringstream istr(tmp.c_str() + 3);
      string yokoku;
      istr >> yokoku;
      data->player[1].field.SetColorSequence(yokoku);
    } else if (tmp.substr(0, 3) == "YF=") {
      istringstream istr(tmp.c_str() + 3);
      string yf;
      istr >> yf;
      string yokoku = data->player[0].field.GetColorSequence();
      data->player[0].field = FieldWithColorSequence(yf);
      data->player[0].field.SetColorSequence(yokoku);
    } else if (tmp.substr(0, 3) == "OF=") {
      istringstream istr(tmp.c_str() + 3);
      string of;
      istr >> of;
      string yokoku = data->player[1].field.GetColorSequence();
      data->player[1].field = FieldWithColorSequence(of);
      data->player[1].field.SetColorSequence(yokoku);
    } else if (tmp.substr(0, 6) == "STATE=") {
      istringstream istr(tmp.c_str() + 6);
      int state;
      istr >> state;
      data->player[0].state = (state >> 0) & 0x55555555;
      data->player[1].state = (state >> 1) & 0x55555555;
    } else if (tmp.substr(0, 3) == "YO=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[0].ojama;
    } else if (tmp.substr(0, 3) == "OO=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> data->player[1].ojama;
    }
  }
}

void Cpu::SendDecision(
    const Data& data, const Decision& decision,
    const string& message) const {
  cout << "ID=" << data.id << " X=" << decision.x << " R=" << decision.r <<
      " MSG=" << message << endl << flush;
}
