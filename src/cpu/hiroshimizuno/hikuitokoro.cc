#include <iostream>
#include <string>

#include "hikuitokoro.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[]) {
  Hikuitokoro ai;
  while (true) {
    string line;
    getline(cin, line);
    const string decision = ai.ProcessFrame(line);
    //if (decision.size() != decision.find(" X=") + 8) {
    //  std::cerr << "[MIUZNO] " << line << "    " << decision << endl;
    //}
    cout << decision << endl;
  }
  return 0;
}
