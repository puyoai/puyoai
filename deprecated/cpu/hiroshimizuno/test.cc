#include <iostream>
using std::cout;
using std::endl;

#include "sse.h"

void PrintField(unsigned char field[16]) {
  for (int x = 0; x < 6; ++x) {
    for (int y = 0; y < 16; ++y) {
      int p = field[x * 16 + y];
      if (p < 0x10) { std::cout << "0"; }
      std::cout << std::hex << p << std::dec << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int main(int argc, char* argv[]) {
  unsigned char __attribute__((aligned (16))) field[96] = {
    6, 0, 0, 0, 2, 2, 2, 8, 8, 4, 4, 8, 8, 6, 6, 6,
    0, 0, 0, 0, 2, 6, 6, 6, 6, 1, 4, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 1, 1, 1, 1, 1, 4, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 8, 8, 1, 8, 8, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  /*
  unsigned char __attribute__((aligned (16))) field[96] = {
    0, 0, 0, 0, 8, 8, 8, 8, 6, 6, 6, 6, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 8, 8, 1, 8, 8, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  */
  PrintField(field);
  int vanished = Simulate(field, 1);
  std::cout << "[VANISHED] " << vanished << std::endl;
  PrintField(field);
  return 0;
}
