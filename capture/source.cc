#include "source.h"

Source::Source()
  : ok_(false),
    width_(-1),
    height_(-1),
    done_(false) {
}

void Source::saveScreenShot(SDL_Surface* surf, SDL_Surface* scr) {
  static int ss_num = 0;
  char buf[256];
  sprintf(buf, "/tmp/orig-puyo%05d.bmp", ss_num);
  SDL_SaveBMP(surf, buf);
  if (scr) {
    sprintf(buf, "/tmp/parsed-puyo%05d.bmp", ss_num);
    SDL_SaveBMP(scr, buf);
  }
  ss_num++;
}
