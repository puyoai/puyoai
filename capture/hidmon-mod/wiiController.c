#include "wiiController.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hidasp.h"
#include "gr.h"

#define PORTB 0x38
#define DDRB 0x37
#define TCCR0A 0x50

void tap(int id, int duration) {
  hidPokeMem(PORTB, 1 << id, 1 << id);
  usleep(duration*1000);
  hidPokeMem(PORTB, 0, 1 << id);
  usleep(1000 * 1000 / 30);
}

void outputKeys(int data) {
  hidPokeMem(PORTB, data, 0xff);
}

#define T1 (1000.0 / 40)
#define T2 (1000 / 30 * 12)

// Key ID on the device. Differs from the keycode of the puyo duel framework.
enum {
  K_B = 1, K_A, K_START, K_UP, K_RIGHT, K_LEFT, K_DOWN
};

struct {
  char c;
  int id;
  int duration;
} data[] = {
  {'a', K_LEFT, T1},
  {'d', K_RIGHT, T1},
  {'x', K_DOWN, T2},
  {'w', K_UP, T2},
  {',', K_B, T1},
  {'.', K_A, T1},
  {' ', K_START, T1},
  {0, 0, 0}
};

void kaiten() {
  int i;
  for (i = 0; i < 4 * 800; i++) {
    tap(K_A, T1);
  }
}

void tattaka() {
  tap(K_B, T1);
  tap(K_B, T1);
  tap(K_A, T1);
  tap(K_RIGHT, T1);
  tap(K_RIGHT, T1);
  tap(K_A, T1);
}

void puyo_controller() {
  // make port B available for I/O, even when the device is programmer mode.
  hidPokeMem(TCCR0A, 0x00, 0xff);
  hidPokeMem(DDRB, 0xff, 0xff);
  hidPokeMem(PORTB, 0x00, 0xff);
  while(1) {
    char c = getchar();
    int i;
    if (c == '\x1b') break;
    if (c == 'u') {
      kaiten();
    }
    if (c == 'e') {
      tattaka();
    }
    for (i = 0; data[i].c; i++) {
      if (c == data[i].c) {
	tap(data[i].id, data[i].duration);
	printf("%d", i);
	break;
      }
    }
  }
}

// from ../../core/field.h
enum Key {
  KEY_NONE,
  KEY_UP,
  KEY_RIGHT,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT_TURN,
  KEY_LEFT_TURN,
  // Not in field.h
  KEY_START,
};

const int keyToPortMap[] = {
  0, K_UP, K_RIGHT, K_DOWN, K_LEFT, K_A, K_B, K_START
};

void slave_mode() {
  // make port B available for I/O, even when the device is programmer mode.
  hidPokeMem(TCCR0A, 0x00, 0xff);
  hidPokeMem(DDRB, 0xff, 0xff);
  hidPokeMem(PORTB, 0x00, 0xff);
  while(1) {
    int keydata = 0;
#define BUFSIZE 80
    char buf[BUFSIZE];
    fgets(buf, BUFSIZE, stdin);
    int id1, id2;
    printf("buf=%s ", buf);
    int n = sscanf(buf, "%d%d", &id1, &id2);
    if (n >= 1) {
      keydata |= 1 << keyToPortMap[id1];
    }
    if (n >= 2) {
      keydata |= 1 << keyToPortMap[id2];
    }
    printf("keydata=%x\n", keydata);
    outputKeys(keydata);
    usleep(T1*1000);
  }
}
