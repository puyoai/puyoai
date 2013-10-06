#include "source.h"

#include <gflags/gflags.h>

#include <SDL_image.h>

DEFINE_bool(save_parsed, false, "");

Source::Source()
  : ok_(false),
    width_(-1),
    height_(-1),
    done_(false) {
}

static void saveImg(SDL_Surface* surf, const char* prefix, int ss_num) {
  char buf[256];
  sprintf(buf, "/tmp/%s-puyo%05d.bmp", prefix, ss_num);
  SDL_SaveBMP(surf, buf);
}

void Source::saveScreenShot(SDL_Surface* surf, SDL_Surface* scr) {
  static int ss_num = 0;
  saveImg(surf, "orig", ss_num);
  if (scr && FLAGS_save_parsed) {
    saveImg(scr, "parsed", ss_num);
  }
  ss_num++;
}
