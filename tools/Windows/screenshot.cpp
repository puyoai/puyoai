// NOTE(hiroshimizuno):
// 1. We expect Puyopuyo Fever to run in 640x480, fullscreen mode.
// 2. You can keep hardware acceleration on.
// 3. This code is tested on 32bit Windows 2003.
// 4. Input keys for 2P (AI) should be configured to:
//    [start key] z
//    [anti-clock rotate] x
//    [clock rotate] c
//    [left] h
//    [right] l
//    [up] k
//    [down] j
// 5. You need to configure PuyoPuyo Fever with following values:
//    Full-Screen, Sharp, Dierct3D.
// 6. You can build this program by following steps:
//    - Create "Blank Project" in Visual C++.
//    - Add this file to the "source file" in the solution explorer.
//    - Build the solution.
// 7. To try the current behavior, follow these steps:
//    - Run this program by pressing "F5" when your focus is in Visual Studio.
//    - Start PuyoPuyo Fever.
//    - Configure pads and keys appropriately.
//    - Start a two player game and choose a classic game by using your gamepad.
//    - Choose characters for yourself and for the AI. You have already set "X"
//      key as the ok button for 2P, if you have read my notes above.
//    - Mock AI keeps dropping puyos to the lowest col, as quick as possible.

// TODO(hiroshimizuno):
// 1. Detect next puyos of 1P (Momoken-san).
// 2. Implement game-end detector, character selector and chuu-kara selector.
// 3. Make sure the detector is not confused by:
//    a) vanish-effect
//    b) drop-effect
//    c) single-puyo-effect (Single puyo ha tokidoki ukiuki shite imasu.)
// 4. Stop sending unnecessary fields to CPU.
// 5. Improve keyboard event handling so that we can move puyos more quickly.

// TODO(hamaji):
// 1. Test it in the real environment.
// 2. Design and implement interaction with our cpu.cc
// 3. If possible, port Yamaguchi-san's keystroke generator to this file for
//    even more accurate & quick movements.

#include "windows.h"
#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

#ifdef _DEBUG
#include "stdio.h"
void debug(char* str) { OutputDebugString(str); }
void debug(unsigned int num) {
  char buf[32];
  sprintf(buf, "%d\n", num);
  OutputDebugString(buf);
}
void debug(unsigned int i1, unsigned int i2) {
  char buf[32];
  sprintf(buf, "%d %d\n", i1, i2);
  OutputDebugString(buf);
}
#define PRINT(arg) debug(arg);
#define PRINT2(arg1, arg2) debug(arg1, arg2);
#else
#define PRINT(args)
#define PRINT2(arg1, arg2)
#endif



const int LEFT1 = 52;
const int LEFT2 = 397;
const int WIDTH = 32;
const int BOTTOM = 41;
const int HEIGHT = 28;
const int COLOR_THRESHOLD = 123;
const int LEFT1_NEXT = 263;
const int LEFT2_NEXT = 348;
const int BOTTOM_NEXT = 333;
const int SIZE_NEXTNEXT = 24;
const int LEFT1_NEXTNEXT = 285;
const int LEFT2_NEXTNEXT = 329;
const int BOTTOM_NEXTNEXT = 280;
const int COLOR_THRESHOLD_NEXTNEXT = 45;

const int NEXTNEXTLINE_LEFT1 = 298;
const int NEXTNEXTLINE_LEFT2 = 342;
const int NEXTNEXTLINE_BOTTOM = 287;
const int NEXTNEXTLINE_HEIGHT = 56;
const int NEXTNEXTLINE_BGCOLOR = 0x4A4584;
const int NEXTNEXTLINE_BACKGROUND_MIN = 10;
const int NEXTNEXTLINE_MIN_VOTES = 3;
const int NEXTNEXTLINE_HEIGHT5 = 20;

const int PIXEL_THRESHOLD_EMPTY_NEXT = 432;
const int MIN_PIXELS_NEXTS_STABLE = 1;
const int MAX_PIXELS_NEXTS_STABLE = 600;

const int CHUUKARA_WHITE_COLOR = 0xFFFFFF;
const int CHUUKARA_WHITE_Y = 358;
const int CHUUKARA_WHITE_LEFT = 450;
const int CHUUKARA_WHITE_RIGHT = 500;

const int SELECT_CHUUKARA_INTERVAL = 1000;
const int WAITTIME_NEXTNEXT_MOVING = 70;
const int KEY_INPUT_INTERVAL = 40;  // TODO(hiroshimizuno): optimize this value.

const int WIDTH_WIN = 640;
const int HEIGHT_WIN = 480;

const int EMPTY = 8;
const int RED = 1;
const int BLUE = 2;
const int GREEN = 3;
const int YELLOW = 4;
const int OJAMA = 5;

const int STATE_NOTINTERESTING = 0;
const int STATE_CANMOVE = 1;
const int STATE_GOTNEXTNEXT = 2;
const int STATE_NOTINGAME = 3;
const int STATE_LOADINGNEXTS = 4;
const int STATE_LOADEDNEXTS = 5;
const int STATE_MAYBESECCHI1 = 6 << 4;

const int DIKEYBOARD_H = 0x0423;
const int DIKEYBOARD_J = 0x0424;
const int DIKEYBOARD_K = 0x0425;
const int DIKEYBOARD_L = 0x0426;
const int DIKEYBOARD_Z = 0x042C;
const int DIKEYBOARD_X = 0x042D;
const int DIKEYBOARD_C = 0x042E;

const int KEYCODE_TBL[7] = {
  0, DIKEYBOARD_K, DIKEYBOARD_L, DIKEYBOARD_J, DIKEYBOARD_H, DIKEYBOARD_C, DIKEYBOARD_X
};

int KEY_COLORS[30] = {
  0x840000, 0x940000, 0xA50000, 0xBD0000, 0xCE0000, 0xDE0000,  // RED = 1
  0x0028C6, 0x0040D6, 0x0049E7, 0x0020C6, 0x0018B5, 0x000894,  // BLUE = 2
  0x00C700, 0x00D700, 0x00E700, 0x008600, 0x009E00, 0x218E21,  // GREEN = 3
  0xDED700, 0xE7E700, 0xD6CF00, 0xCEBE00, 0xBDA608, 0xAD9608,  // YELLOW = 4
  // 0x392418 and 0xFFFFFF are blacklisted because of false positives.
  0xAA392418, 0x636963, 0xB5B6B5, 0xEFEFDE, 0x7B7973, 0xAAFFFFFF   // OJAMA = 5
};

HBITMAP bitmap = NULL;
LPDWORD pixel;
HDC dcCopied = NULL;
int field1[72];
int field2[72];
int field1Secchi[72];
int nexts1[6] = { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY };
int nexts2[6] = { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY };
DWORD timeLastKey = 0;
bool pressed = false;
bool isEmptyNext = false;
std::vector<INPUT> inputs;
int timeGameSuspended = 0;
DWORD timeLoadingNextsStarted = 0;

int DetectNextsState2() {
  int blankPixels = 0;
  for (int v = 0; v < HEIGHT * 2; ++v) {
    for (int h = 0; h < WIDTH; ++h) {
      if (pixel[(BOTTOM_NEXT + v) * WIDTH_WIN + (LEFT2_NEXT + h)] == NEXTNEXTLINE_BGCOLOR) {
        ++blankPixels;
      }
    }
  }
  //if (blankPixels < MIN_PIXELS_NEXTS_STABLE) { return STATE_NOTINGAME; }
  if (blankPixels > MAX_PIXELS_NEXTS_STABLE) { return STATE_LOADINGNEXTS; }
  return STATE_LOADEDNEXTS;
}

int DetectNextsState1() {
  int blankPixels = 0;
  for (int v = 0; v < HEIGHT * 2; ++v) {
    for (int h = 0; h < WIDTH; ++h) {
      if (pixel[(BOTTOM_NEXT + v) * WIDTH_WIN + (LEFT1_NEXT + h)] == NEXTNEXTLINE_BGCOLOR) {
        ++blankPixels;
      }
    }
  }
  //if (blankPixels < MIN_PIXELS_NEXTS_STABLE) { return STATE_NOTINGAME; }
  if (blankPixels > MAX_PIXELS_NEXTS_STABLE) { return STATE_LOADINGNEXTS; }
  return STATE_LOADEDNEXTS;
}

bool DetectChuukara() {
  for (int h = CHUUKARA_WHITE_LEFT; h < CHUUKARA_WHITE_RIGHT; ++h) {
    if (pixel[CHUUKARA_WHITE_Y * WIDTH_WIN + h] != CHUUKARA_WHITE_COLOR) {
      return false;
    }
  }
  return true;
}

// Detects AI's next and nextnext.
bool DetectNexts2() {
  int foreground = 0;
  int line[NEXTNEXTLINE_HEIGHT];
  for (int v = 0; v < NEXTNEXTLINE_HEIGHT; ++v) {
    int argb = pixel[(NEXTNEXTLINE_BOTTOM + v) * WIDTH_WIN + NEXTNEXTLINE_LEFT2];
    if (argb != NEXTNEXTLINE_BGCOLOR) { line[foreground++] = argb; }
  }

  int pos = 0;
  for (int red = 0, blue = 0, green = 0, yellow = 0; pos < foreground; ++pos) {
    int r = (line[pos] >> 16) & 0xFF;
    int g = (line[pos] >> 8) & 0xFF;
    int b = line[pos] & 0xFF;
    if (r > 0x80 && g + b < 0x50) {
      red += 1;
      if (red >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[4] = 0 + 1;
        break;
      }
    } else if (b > 0x80 && r + g < 0x50) {
      blue += 1;
      if (blue >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[4] = 1 + 1;
        break;
      }
    } else if (g > 0x80 && r + b < 0x50) {
      green += 1;
      if (green >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[4] = 2 + 1;
        break;
      }
    } else if (r > 0x80 && g > 0x80 && b < 0x50) {
      yellow += 1;
      if (yellow >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[4] = 3 + 1;
        break;
      }
    }
  }
  //if (pos + NEXTNEXTLINE_HEIGHT5 > foreground) { PRINT("DETECT_NEXTS2_FAILED"); return false; }
  for (int i = foreground - 1, red = 0, blue = 0, green = 0, yellow = 0; i > pos; --i) {
    int r = (line[i] >> 16) & 0xFF;
    int g = (line[i] >> 8) & 0xFF;
    int b = line[i] & 0xFF;
    if (r > 0x80 && g + b < 0x50) {
      red += 1;
      if (red >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[5] = 0 + 1;
        break;
      }
    } else if (b > 0x80 && r + g < 0x50) {
      blue += 1;
      if (blue >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[5] = 1 + 1;
        break;
      }
    } else if (g > 0x80 && r + b < 0x50) {
      green += 1;
      if (green >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[5] = 2 + 1;
        break;
      }
    } else if (r > 0x80 && g > 0x80 && b < 0x50) {
      yellow += 1;
      if (yellow >= NEXTNEXTLINE_MIN_VOTES) {
        nexts2[5] = 3 + 1;
        break;
      }
    }
  }
  if (nexts2[2] == EMPTY || nexts2[3] == EMPTY) {
    for (int y = 0; y < 2; ++y) {
      int freqs[4] = { 0, 0, 0, 0 };
      for (int v = 0; v < HEIGHT; ++v) {
        for (int h = 0; h < WIDTH; ++h) {
          int argb = pixel[(BOTTOM_NEXT + y * HEIGHT + v) * WIDTH_WIN + (LEFT2_NEXT + h)];
          for (int color = 0; color < 4; ++color) {
            for (int i = 0; i < 6; ++i) {
              if (KEY_COLORS[color * 6 + i] == argb) { freqs[color] += 1; }
            }
          }
        }
      }
      for (int i = 3; i >= 0; --i) {
        if (freqs[i] > COLOR_THRESHOLD) { nexts2[2 + y] = i + 1; }
      }
    }
  }
  //PRINT2(nexts2[2] * 10 + nexts2[3], nexts2[4] * 10 + nexts2[5]);
  return true;
}

// Detects the player's next and nextnext.
bool DetectNexts1() {
  int foreground = 0;
  int line[NEXTNEXTLINE_HEIGHT];
  for (int v = 0; v < NEXTNEXTLINE_HEIGHT; ++v) {
    int argb = pixel[(NEXTNEXTLINE_BOTTOM + v) * WIDTH_WIN + NEXTNEXTLINE_LEFT1];
    if (argb != NEXTNEXTLINE_BGCOLOR) { line[foreground++] = argb; }
  }

  int pos = 0;
  for (int red = 0, blue = 0, green = 0, yellow = 0; pos < foreground; ++pos) {
    int r = (line[pos] >> 16) & 0xFF;
    int g = (line[pos] >> 8) & 0xFF;
    int b = line[pos] & 0xFF;
    if (r > 0x80 && g + b < 0x50) {
      red += 1;
      if (red >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[4] = 0 + 1;
        break;
      }
    } else if (b > 0x80 && r + g < 0x50) {
      blue += 1;
      if (blue >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[4] = 1 + 1;
        break;
      }
    } else if (g > 0x80 && r + b < 0x50) {
      green += 1;
      if (green >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[4] = 2 + 1;
        break;
      }
    } else if (r > 0x80 && g > 0x80 && b < 0x50) {
      yellow += 1;
      if (yellow >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[4] = 3 + 1;
        break;
      }
    }
  }
  for (int i = foreground - 1, red = 0, blue = 0, green = 0, yellow = 0; i > pos; --i) {
    int r = (line[i] >> 16) & 0xFF;
    int g = (line[i] >> 8) & 0xFF;
    int b = line[i] & 0xFF;
    if (r > 0x80 && g + b < 0x50) {
      red += 1;
      if (red >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[5] = 0 + 1;
        break;
      }
    } else if (b > 0x80 && r + g < 0x50) {
      blue += 1;
      if (blue >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[5] = 1 + 1;
        break;
      }
    } else if (g > 0x80 && r + b < 0x50) {
      green += 1;
      if (green >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[5] = 2 + 1;
        break;
      }
    } else if (r > 0x80 && g > 0x80 && b < 0x50) {
      yellow += 1;
      if (yellow >= NEXTNEXTLINE_MIN_VOTES) {
        nexts1[5] = 3 + 1;
        break;
      }
    }
  }
  if (nexts1[2] == EMPTY || nexts1[3] == EMPTY) {
    for (int y = 0; y < 2; ++y) {
      int freqs[4] = { 0, 0, 0, 0 };
      for (int v = 0; v < HEIGHT; ++v) {
        for (int h = 0; h < WIDTH; ++h) {
          int argb = pixel[(BOTTOM_NEXT + y * HEIGHT + v) * WIDTH_WIN + (LEFT1_NEXT + h)];
          for (int color = 0; color < 4; ++color) {
            for (int i = 0; i < 6; ++i) {
              if (KEY_COLORS[color * 6 + i] == argb) { freqs[color] += 1; }
            }
          }
        }
      }
      for (int i = 3; i >= 0; --i) {
        if (freqs[i] > COLOR_THRESHOLD) { nexts1[2 + y] = i + 1; }
      }
    }
  }
  //PRINT2(nexts1[2] * 10 + nexts1[3], nexts1[4] * 10 + nexts1[5]);
  return true;
}

HWND FindWindow() {
  HWND window = NULL;
  RECT rect = { 0, 0, 0, 0 };
  while (rect.right - rect.left != WIDTH_WIN || rect.bottom - rect.top != HEIGHT_WIN) {
    Sleep(100);
    window = GetForegroundWindow();
    GetWindowRect(window, &rect);
  }
  HDC dc = GetDC(window);
  BITMAPINFO bmpinfo;
  bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFO);
  bmpinfo.bmiHeader.biWidth = WIDTH_WIN;
  bmpinfo.bmiHeader.biHeight = HEIGHT_WIN;
  bmpinfo.bmiHeader.biPlanes = 1;
  bmpinfo.bmiHeader.biBitCount = 32;
  bmpinfo.bmiHeader.biCompression = BI_RGB;
  bitmap = CreateDIBSection(dc, &bmpinfo, DIB_RGB_COLORS, (void**)&pixel, NULL, 0);
  dcCopied = CreateCompatibleDC(dc);
  SelectObject(dcCopied, bitmap);
  ReleaseDC(window, dc);
  return window;
}

void KeyDown(int directInputKeyCode, INPUT* input) {
  memset(input, 0, sizeof(INPUT));
  input->type = INPUT_KEYBOARD;
  input->ki.wScan = directInputKeyCode;
  input->ki.dwFlags = 0;
}
void KeyUp(int directInputKeyCode, INPUT* input) {
  memset(input, 0, sizeof(INPUT));
  input->type = INPUT_KEYBOARD;
  input->ki.wScan = directInputKeyCode;
  input->ki.dwFlags = KEYEVENTF_KEYUP;
}

void GenerateSampleKeyStrokes(int* f2) {
  int h[6] = {0, 0, 0, 0, 0, 0};
  for (int x = 0; x < 6; ++x) {
    for (int y = 0; y < 12; ++y) {
      if (f2[y * 6 + x] == EMPTY) {
        h[x] = y;
        break;
      }
    }
  }
  INPUT buf;
  if (h[2] < h[0] && h[2] < h[1] && h[2] < h[3] && h[2] < h[4] && h[2] < h[5]) {
    KeyDown(DIKEYBOARD_J, &buf);
    inputs.push_back(buf);
  } else if (h[3] < h[0] && h[3] < h[1] && h[3] < h[4] && h[3] < h[5]) {
    KeyDown(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_J, &buf);
    inputs.push_back(buf);
  } else if (h[1] < h[0] && h[1] < h[4] && h[1] < h[5]) {
    KeyDown(DIKEYBOARD_H, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_H, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_J, &buf);
    inputs.push_back(buf);
  } else if (h[4] < h[0] && h[4] < h[5]) {
    KeyDown(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_J, &buf);
    inputs.push_back(buf);
  } else if (h[0] < h[5]) {
    KeyDown(DIKEYBOARD_H, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_H, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_H, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_H, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_J, &buf);
    inputs.push_back(buf);
  } else {
    KeyDown(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyUp(DIKEYBOARD_L, &buf);
    inputs.push_back(buf);
    KeyDown(DIKEYBOARD_J, &buf);
    inputs.push_back(buf);
  }
}

void GetKeysFromCpu(int* f1, int* f2, int* n1, int* n2, int state) {
  PRINT("\n\n");
  int v1 = 0, v2 = 0;
  for (int i = 0; i < 6; ++i) { v1 = v1 * 10 + n1[i]; v2 = v2 * 10 + n2[i]; }
  PRINT2(v1, v2);
  PRINT("\n");
  for (int y = 11; y >= 0; --y) {
    int value1 = 0;
    int value2 = 0;
    for (int x = 0; x < 6; ++x) {
      value1 *= 10;
      value1 += field1[y * 6 + x];
      value2 *= 10;
      value2 += field2[y * 6 + x];
    }
    PRINT2(value1, value2);
  }

  if (state == STATE_GOTNEXTNEXT) {
    GenerateSampleKeyStrokes(f2);
  }
}

void SendKeysToFever() {
  DWORD timeNow = GetTickCount();
  if (timeLastKey + KEY_INPUT_INTERVAL < timeNow && inputs.size() > 0) {
    INPUT i = inputs[0];
    //fprintf(stderr, "SendKeysToFever: %d %d\n", i.ki.dwFlags, i.ki.wScan);
    //fflush(stderr);
    inputs.erase(inputs.begin());
    SendInput(1, &i, sizeof(INPUT));
    timeLastKey = timeNow;
  }
}

void CancelAllKeys() {
  if (inputs.size()) {
    fprintf(stderr, "cancel %lu keys\n", inputs.size());
  }
  inputs.clear();
  INPUT input[7];
  KeyUp(DIKEYBOARD_H, input + 1);
  KeyUp(DIKEYBOARD_J, input + 1);
  KeyUp(DIKEYBOARD_K, input + 2);
  KeyUp(DIKEYBOARD_L, input + 3);
  KeyUp(DIKEYBOARD_Z, input + 4);
  KeyUp(DIKEYBOARD_X, input + 5);
  KeyUp(DIKEYBOARD_C, input + 6);
  SendInput(7, input, sizeof(INPUT));
}

void SelectChuukara() {
  INPUT input[2];
  KeyUp(DIKEYBOARD_X, input + 0);
  KeyDown(DIKEYBOARD_X, input + 1);
  SendInput(2, input, sizeof(INPUT));
}

// Detects AI's field.
void DetectField2() {
  int field2New[72];
  for (int i = 0; i < 72; ++i) { field2New[i] = EMPTY; }
  for (int y = 0; y < 12; ++y) {
    for (int x = 0; x < 6; ++x) {
      int freqs[5] = { 0, 0, 0, 0, 0 };
      for (int v = 0; v < HEIGHT; ++v) {
        for (int h = 0; h < WIDTH; ++h) {
          int argb = pixel[(BOTTOM + y * HEIGHT + v) * WIDTH_WIN + (LEFT2 + x * WIDTH + h)];
          for (int color = 0; color < 5; ++color) {
            for (int i = 0; i < 6; ++i) {
              if (KEY_COLORS[color * 6 + i] == argb) { freqs[color] += 1; }
            }
          }
        }
      }
      for (int i = 4; i >= 0; --i) {
        if (freqs[i] > COLOR_THRESHOLD) { field2New[y * 6 + x] = i + 1; }
      }
    }
  }
  field2New[68] = EMPTY;  // Clears the "X" mark.
  for (int i = 0; i < 72; ++i) { field2[i] = field2New[i]; }
}

// Detects the player's field.
void DetectField1() {
  int field1New[72];
  for (int i = 0; i < 72; ++i) { field1New[i] = EMPTY; }
  for (int y = 0; y < 12; ++y) {
    for (int x = 0; x < 6; ++x) {
      int freqs[5] = { 0, 0, 0, 0, 0 };
      for (int v = 0; v < HEIGHT; ++v) {
        for (int h = 0; h < WIDTH; ++h) {
          int argb = pixel[(BOTTOM + y * HEIGHT + v) * WIDTH_WIN + (LEFT1 + x * WIDTH + h)];
          for (int color = 0; color < 5; ++color) {
            for (int i = 0; i < 6; ++i) {
              if (KEY_COLORS[color * 6 + i] == argb) { freqs[color] += 1; }
            }
          }
        }
      }
      for (int i = 4; i >= 0; --i) {
        if (freqs[i] > COLOR_THRESHOLD) { field1New[y * 6 + x] = i + 1; }
      }
    }
  }
  field1New[68] = EMPTY;  // Clears the "X" mark.
  for (int i = 0; i < 72; ++i) { field1[i] = field1New[i]; }
}

bool DetectField1Secchi() {
  for (int i = 0; i < 72; ++i) { field1Secchi[i] = EMPTY; }
  for (int y = 0; y < 12; ++y) {
    for (int x = 0; x < 6; ++x) {
      int freqs[5] = { 0, 0, 0, 0, 0 };
      for (int v = 0; v < HEIGHT; ++v) {
        for (int h = 0; h < WIDTH; ++h) {
          int argb = pixel[(BOTTOM + y * HEIGHT + v) * WIDTH_WIN + (LEFT1 + x * WIDTH + h)];
          for (int color = 0; color < 5; ++color) {
            for (int i = 0; i < 6; ++i) {
              if (KEY_COLORS[color * 6 + i] == argb) { freqs[color] += 1; }
            }
          }
        }
      }
      for (int i = 4; i >= 0; --i) {
        if (freqs[i] > COLOR_THRESHOLD) { field1Secchi[y * 6 + x] = i + 1; }
      }
    }
  }
  field1Secchi[68] = EMPTY;  // Clears the "X" mark.
  int diff = 0;
  int xs[2];
  int ys[2];
  for (int y = 0; y < 12; ++y) {
    for (int x = 0; x < 6; ++x) {
      if (field1[y * 6 + x] != field1Secchi[y * 6 + x]) {
        if (diff > 1) { return false; }
        xs[diff] = x;
        ys[diff] = y;
        ++diff;
      }
    }
  }
  if (diff == 2 &&
      ((xs[0] == xs[1]) && ((ys[0] - ys[1]) * (ys[0] - ys[1]) == 1) ||
       (ys[0] == ys[1]) && ((xs[0] - xs[1]) * (xs[0] - xs[1]) == 1)) &&
      (ys[0] == 0 ||
       ys[1] == 0 ||
       (ys[0] <= ys[1] && field1Secchi[ys[0] * 6 + xs[0] - 6] != EMPTY) ||
       (ys[1] <= ys[0] && field1Secchi[ys[1] * 6 + xs[1] - 6] != EMPTY)) &&
      ((field1Secchi[ys[0] * 6 + xs[0]] == nexts1[0] &&
        field1Secchi[ys[1] * 6 + xs[1]] == nexts1[1]) ||
       (field1Secchi[ys[0] * 6 + xs[0]] == nexts1[1] &&
        field1Secchi[ys[1] * 6 + xs[1]] == nexts1[0]))) {
    return true;
  }
  return false;
}

// NOTE(hiroshimizuno):
// We do not use Direct3D because it would make the capture slower!
// See the URL below for details:
// http://webcache.googleusercontent.com/search?q=cache:5iPo_mKdwkEJ:www.ureader.com/msg/14641021.aspx+&cd=1&hl=ja&ct=clnk&gl=jp
void UpdateScreenshot(HWND window) {
  HDC dc = GetDC(window);
  BitBlt(dcCopied, 0, 0, WIDTH_WIN, HEIGHT_WIN, dc, 0, 0, SRCCOPY);
  ReleaseDC(window, dc);
}

void OutputState(int state, bool isMaybeSecchi1) {
  printf("%d ", (isMaybeSecchi1 ? state | STATE_MAYBESECCHI1 : state));
  for (int i = 0; i < 72; i++) {
    printf("%d", (isMaybeSecchi1 ? field1Secchi[i] : field1[i]));
  }
  printf(" ");
  for (int i = 0; i < 6; i++) {
    printf("%d", nexts1[i]);
  }
  printf(" ");
  for (int i = 0; i < 72; i++) {
    printf("%d", field2[i]);
  }
  printf(" ");
  for (int i = 0; i < 6; i++) {
    printf("%d", nexts2[i]);
  }
  puts("");
  fflush(stdout);
}


void GetKeysFromStdin() {
  char buf[256];
  gets(buf);
  string key_seq = buf;
  if (buf[0] != '.') {
    //fprintf(stderr, "screenshot.exe: from stdin %s\n", buf);
    //fflush(stderr);
  }

  INPUT ibuf;
  for (size_t i = 0; i < key_seq.size() && key_seq[i] != '.'; i++) {
    int c = KEYCODE_TBL[key_seq[i] - '0'];
    i++;
    int b = key_seq[i];
    if (b != ',') {
      i++;
      b = KEYCODE_TBL[b - '0'];
      KeyDown(b, &ibuf);
      inputs.push_back(ibuf);
      KeyUp(b, &ibuf);
      inputs.push_back(ibuf);
    }
    KeyDown(c, &ibuf);
    inputs.push_back(ibuf);
    if (c != DIKEYBOARD_J) {
      KeyUp(c, &ibuf);
      inputs.push_back(ibuf);
    }
    //fprintf(stderr, "screenshot.exe: %d %d\n", c, b);
    //fflush(stderr);
  }
  key_seq.clear();
}

int debug_prev_state = 0;
//int WINAPI WinMain(
//    HINSTANCE instance, HINSTANCE instancePrev, LPSTR cmd, int show) {
int main(int argc, char* argv[]) {
  string key_seq;
  bool prod = false;
  if (argc == 2 && !strcmp(argv[1], "-p"))
    prod = true;

  // In order not to bother Momoken-san.
  SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
  for (int i = 0; i < 72; ++i) {
	field1[i] = EMPTY;
    field2[i] = EMPTY;
	field1Secchi[i] = EMPTY;
  }
  HWND window = FindWindow();
  PRINT("WINDOW FOUND\n");
  Sleep(2000);
  int nextsState2 = 0;  // Not in game yet.
  int nextsState1 = 0;
  CancelAllKeys();
  fprintf(stderr, "screenshot.exe: detected\n");
  fflush(stderr);
  // NOTE(hiroshimizuno): For development only. Will be "while (true)".
  while (true) {
  //for (int i = 0; i < 1234567; ++i) {
    bool isMaybeSecchi1 = false;
    UpdateScreenshot(window);
    int nextsState2New = DetectNextsState2();
    if (nextsState2New != STATE_LOADINGNEXTS) { timeLoadingNextsStarted = 0; }
    if (nextsState2New == STATE_NOTINGAME) {
      if (nextsState2 != STATE_NOTINGAME) { CancelAllKeys(); }
      if (timeGameSuspended == 0) {
        timeGameSuspended = GetTickCount();
      } else if (timeGameSuspended + SELECT_CHUUKARA_INTERVAL < GetTickCount()) {
        timeGameSuspended = 0;
        SelectChuukara();
      }
    } else {
      timeGameSuspended = 0;
      if (DetectChuukara()) {
        for (int i = 0; i < 6; ++i) { nexts2[i] = EMPTY; nexts1[i] = EMPTY; }
        DetectNexts2();
        DetectNexts1();
        SelectChuukara();
      }
      int nextsState1New = DetectNextsState1();
      if (nextsState1New == STATE_LOADEDNEXTS) {
        if (DetectField1Secchi()) {
          isMaybeSecchi1 = true;
          // Prints field1Secchi to stderr.
          /*
          fprintf(stderr, "[MAYBE_SECCHI_DETECTED]\n");
          for (int y = 11; y >= 0; --y) {
            int value = 0;
            for (int x = 0; x < 6; ++x) {
              value = value * 10 + field1Secchi[y * 6 + x];
            }
            fprintf(stderr, "%d\n", value);
          }
          fprintf(stderr, "\n");
          fflush(stderr);
          */
        }
      }
      if (nextsState1 != nextsState1New) {
        if (nextsState1 == STATE_LOADINGNEXTS &&
            nextsState1New == STATE_LOADEDNEXTS) {
          DetectNexts1();
        } else if (nextsState1 == STATE_LOADEDNEXTS &&
                   nextsState1New == STATE_LOADINGNEXTS) {
          nexts1[0] = nexts1[2];
          nexts1[1] = nexts1[3];
          nexts1[2] = nexts1[4];
          nexts1[3] = nexts1[5];
          nexts1[4] = EMPTY;
          nexts1[5] = EMPTY;
          DetectField1();
        }
      }
      nextsState1 = nextsState1New;
      if (nextsState2 != nextsState2New) {
        if (nextsState2 == STATE_LOADINGNEXTS &&
            nextsState2New == STATE_LOADEDNEXTS) {
          DetectNexts2();
        } else if (nextsState2 == STATE_LOADEDNEXTS &&
                   nextsState2New == STATE_LOADINGNEXTS) {
          timeLoadingNextsStarted = timeGetTime();
          CancelAllKeys();
          nexts2[0] = nexts2[2];
          nexts2[1] = nexts2[3];
          nexts2[2] = nexts2[4];
          nexts2[3] = nexts2[5];
          nexts2[4] = EMPTY;
          nexts2[5] = EMPTY;
          DetectField2();
        }
      }
    }
    nextsState2 = nextsState2New;

    bool isCanMove = (nextsState2 == STATE_LOADINGNEXTS &&
                      timeLoadingNextsStarted > 0 &&
                      timeLoadingNextsStarted + WAITTIME_NEXTNEXT_MOVING < timeGetTime());
    if (prod) {
      OutputState((isCanMove ? STATE_CANMOVE : nextsState2), isMaybeSecchi1);
      GetKeysFromStdin();
    }
    // Debug only.  Can be removed later.
    if (!prod) {
      if (debug_prev_state != (isCanMove ? STATE_CANMOVE : nextsState2)) {
        if (isCanMove) {
          inputs.clear();
          GetKeysFromCpu(field1, field2, nexts1, nexts2, STATE_GOTNEXTNEXT);
        }
        fprintf(stderr, "[STATE_CHANGE] %d -> %d\n", debug_prev_state, (isCanMove ? STATE_CANMOVE : nextsState2));
        fflush(stderr);
      }
      debug_prev_state = (isCanMove ? STATE_CANMOVE : nextsState2);
    }

    SendKeysToFever();
  }
  DeleteDC(dcCopied);
  return 0;
}
