#include "SDL_kanji.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUF 256

static void InitFont(Kanji_Font* font) {
  int i;
  for (i = 0; i < 96*96 + 256; i++) {
    font->moji[i] = 0;
  }
}

Kanji_Font* Kanji_OpenFont(const char* file, int size) {
  Kanji_Font* font;
  font = (Kanji_Font*)malloc(sizeof(Kanji_Font));

  font->k_size = size;
  font->a_size = size / 2;
  font->sys = KANJI_JIS;

  InitFont(font);
  if (0 == Kanji_AddFont(font, file)) {
    return font;
  }
  else {
    free(font);
    return 0;
  }
}

static void ParseChar(Kanji_Font* font, int index, FILE* fp, int shift) {
  char buf[BUF];
  int y;

  if (font->moji[index] != 0) return;

  font->moji[index] = (Uint32*)malloc(sizeof(Uint32)*font->k_size);

  for (y = 0; y < font->k_size; y++) {
    if (fgets(buf, BUF, fp) == NULL)
      abort();
    font->moji[index][y] = (strtol(buf, 0, 16) >> shift);
  }
}

static int ParseFont(Kanji_Font* font, FILE* fp) {
  char buf[BUF], *p;
  int index;
  int k_rshift, a_rshift;
  int s;

  for (s = 8; s < font->k_size; s += 8) {}
  k_rshift = s - font->k_size;
  for (s = 8; s < font->a_size; s += 8) {}
  a_rshift = s - font->a_size;

  while (1) {
    if (fgets(buf, BUF, fp) == NULL) {
      break;
    }

    if (strstr(buf, "ENCODING") != NULL) {
      p = strchr(buf, ' ');
      index = strtol(p, 0, 10);

      while (strstr(buf, "BITMAP") == NULL) {
        if (fgets(buf, BUF, fp) == NULL)
          abort();
      }

      if (index > 255) {
        index = (((index & 0xff00) >> 8) - 0x20) * 96
          + (index & 0xff) - 0x20 + 0xff;
        ParseChar(font, index, fp, k_rshift);
      }
      else {
        ParseChar(font, index, fp, a_rshift);
      }
    }
  }

  return 0;
}

int Kanji_AddFont(Kanji_Font* font, const char* file) {
  FILE* fp;

  fp = fopen(file, "r");
  if(fp==NULL){
    fprintf(stderr, "cant open [%s]\n", file);
    return -1;
  }

  if (0 != ParseFont(font, fp)) return -1;

  fclose(fp);
  return 0;
}

int Kanji_FontHeight(Kanji_Font* font) {
  return font->k_size;
}

int Kanji_FontWidth(Kanji_Font* font, const char* text) {
  if (text == 0) return font->a_size;
  else return strlen(text) * font->a_size;
}

static void KanjiPutpixel(SDL_Surface *s,int x,int y,Uint32 pixel){
  Uint8 *p,bpp;
  if(SDL_MUSTLOCK(s)){
    if(SDL_LockSurface(s)<0) return;
  }
  bpp=s->format->BytesPerPixel;
  p=(Uint8*)(s->pixels)+y*s->pitch+x*bpp;

  switch(bpp){
  case 1:
    *((Uint8 *)p)=(Uint8)pixel;
    break;
  case 2:
    *((Uint16 *)p)=(Uint16)pixel;
    break;
  case 4:
    *((Uint32 *)p)=pixel;
    break;
  }

  if(SDL_MUSTLOCK(s)){
    SDL_UnlockSurface(s);
  }
}

static void euc2jis(unsigned char *c1, unsigned char *c2)
{
  *c1 &= 0x7f;
  *c2 &= 0x7f;
}

static void sjis2jis(unsigned char *c1, unsigned char *c2)
{
  if( *c2 < 0x9f )
    {
      if( *c1 < 0xa0 )
        {
          *c1 -= 0x81;
          *c1 *= 2;
          *c1 += 0x21;
        }
      else
        {
          *c1 -= 0xe0;
          *c1 *= 2;
          *c1 += 0x5f;
        }
      if( *c2 > 0x7f )
        -- *c2;
      *c2 -= 0x1f;
    }
  else
    {
      if( *c1 < 0xa0 )
        {
          *c1 -= 0x81;
          *c1 *= 2;
          *c1 += 0x22;
        }
      else
        {
          *c1 -= 0xe0;
          *c1 *= 2;
          *c1 += 0x60;
        }
      *c2 -= 0x7e;
    }
}

static void ConvertCodingSystem(Kanji_Font* font,
                                unsigned char *c1, unsigned char *c2) {
  if (font->sys == KANJI_SJIS) {
    sjis2jis(c1, c2);
  }
  else if (font->sys == KANJI_EUC) {
    euc2jis(c1, c2);
  }
}

int Kanji_PutText(Kanji_Font* font, int dx, int dy,
                  SDL_Surface* dst, const char* txt, SDL_Color fg)
{
  Uint32 fgcol;
  int index;
  int x, y, cx = dx, cy = dy;
  unsigned char high, low;
  int minx, miny, maxx, maxy;
  int nowKanji = 0;
  const unsigned char* text = (const unsigned char*)txt;

  fgcol = SDL_MapRGB(dst->format, fg.r, fg.g, fg.b);
  while (*text != 0) {
    if (font->sys == KANJI_JIS && *text == 0x1b) {
      if (*(text+1) == 0x24 && *(text+2) == 0x42) {
        nowKanji = 1;
      }
      else if (*(text+1) == 0x28 && *(text+2) == 0x42) {
        nowKanji = 0;
      }
      text += 3;
      continue;
    }
    if (font->sys != KANJI_JIS) nowKanji = !isprint(*text);

    if (!nowKanji) {
      index = *text;
      text++;
      if (font->moji[index] == 0) {
        cx += font->a_size;
        continue;
      }

      minx = (cx >= 0) ? 0 : -cx;
      miny = (cy >= 0) ? 0 : -cy;
      maxx = (cx+font->a_size <= dst->w) ? font->a_size : dst->w-cx;
      maxy = (cy+font->k_size <= dst->h) ? font->k_size : dst->h-cy;

      for (y = miny; y < maxy; y++) {
        for (x = minx; x < maxx; x++) {
          if (font->moji[index][y] & (1 << (font->a_size-x-1))) {
            KanjiPutpixel(dst, cx+x, cy+y, fgcol);
          }
        }
      }
      cx += font->a_size;
    }
    else {
      high = *text;
      low = *(text+1);
      ConvertCodingSystem(font, &high, &low);
      index = (high - 0x20) * 96 + low - 0x20 + 0xff;
      text += 2;
      if (font->moji[index] == 0) {
        cx += font->k_size;
        continue;
      }

      minx = (cx >= 0) ? 0 : -cx;
      miny = (cy >= 0) ? 0 : -cy;
      maxx = (cx+font->k_size <= dst->w) ? font->k_size : dst->w-cx;
      maxy = (cy+font->k_size <= dst->h) ? font->k_size : dst->h-cy;

      for (y = miny; y < maxy; y++) {
        for (x = minx; x < maxx; x++) {
          if (font->moji[index][y] & (1 << (font->k_size-x-1))) {
            KanjiPutpixel(dst, cx+x, cy+y, fgcol);
          }
        }
      }
      cx += font->k_size;
    }
  }
  return 0;
}

int Kanji_PutTextTate(Kanji_Font* font, int dx, int dy,
                      SDL_Surface* dst, const char* txt, SDL_Color fg)
{
  Uint32 fgcol;
  int index;
  int x, y, cx = dx, cy = dy;
  unsigned char high, low;
  int minx, miny, maxx, maxy;
  int nowKanji = 0;
  const unsigned char* text = (const unsigned char*)txt;

  fgcol = SDL_MapRGB(dst->format, fg.r, fg.g, fg.b);
  while (*text != 0) {
    if (font->sys == KANJI_JIS && *text == 0x1b) {
      if (*(text+1) == 0x24 && *(text+2) == 0x42) {
        nowKanji = 1;
      }
      else if (*(text+1) == 0x28 && *(text+2) == 0x42) {
        nowKanji = 0;
      }
      text += 3;
      continue;
    }
    if (font->sys != KANJI_JIS) nowKanji = !isprint(*text);

    if (!nowKanji) {
      text++;
    }
    else {
      high = *text;
      low = *(text+1);
      ConvertCodingSystem(font, &high, &low);
      index = (high - 0x20) * 96 + low - 0x20 + 0xff;
      text += 2;
      if (font->moji[index] == 0) {
        cy += font->k_size;
        continue;
      }

      if (high == 0x21 && (low >= 0x22 || low <= 0x25)) {
        cx += font->k_size * 0.6;
        cy -= font->k_size * 0.6;
      }

      minx = (cx >= 0) ? 0 : -cx;
      miny = (cy >= 0) ? 0 : -cy;
      maxx = (cx+font->k_size <= dst->w) ? font->k_size : dst->w-cx;
      maxy = (cy+font->k_size <= dst->h) ? font->k_size : dst->h-cy;

      for (y = miny; y < maxy; y++) {
        for (x = minx; x < maxx; x++) {
          if (font->moji[index][y] & (1 << (font->k_size-x-1))) {
            KanjiPutpixel(dst, cx+x, cy+y, fgcol);
          }
        }
      }

      if (high == 0x21 && (low >= 0x22 || low <= 0x25)) {
        cx -= font->k_size * 0.6;
        cy += font->k_size * 1.6;
      }
      else {
        cy += font->k_size;
      }
    }
  }
  return 0;
}

SDL_Surface* Kanji_CreateSurface(Kanji_Font* font, const char* text,
                                 SDL_Color fg, int bpp)
{
  SDL_Surface* textbuf;
  int len;
  Uint32 bgcol;

  if (text == NULL) return NULL;
  if (*text == 0 ) return NULL;
  len = strlen(text);

  textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE, font->a_size*len, font->k_size,
                                 bpp, 0, 0, 0, 0);
  if (textbuf == NULL) {
    fprintf(stderr,"ERROR: at Kanji_RenderText\n");
    return NULL;
  }
  bgcol = SDL_MapRGB(textbuf->format, 255-fg.r, 255-fg.g, 255-fg.b);
  SDL_FillRect(textbuf, NULL, bgcol);
  SDL_SetColorKey(textbuf, SDL_TRUE, bgcol);

  Kanji_PutText(font, 0, 0, textbuf, text, fg);

  return textbuf;
}

SDL_Surface* Kanji_CreateSurfaceTate(Kanji_Font* font, const char* text,
                                     SDL_Color fg, int bpp)
{
  SDL_Surface* textbuf;
  int len;
  Uint32 bgcol;

  if (text == NULL) return NULL;
  if (*text == 0 ) return NULL;
  len = strlen(text);

  textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE, font->k_size, font->a_size*len,
                                 bpp, 0, 0, 0, 0);
  if (textbuf == NULL) {
    fprintf(stderr,"ERROR: at Kanji_RenderText\n");
    return NULL;
  }

  bgcol = SDL_MapRGB(textbuf->format, 255-fg.r, 255-fg.g, 255-fg.b);
  SDL_FillRect(textbuf, NULL, bgcol);
  SDL_SetColorKey(textbuf, SDL_TRUE, bgcol);

  Kanji_PutTextTate(font, 0, 0, textbuf, text, fg);

  return textbuf;
}

void Kanji_CloseFont(Kanji_Font* font) {
  int i;
  for (i = 0; i < 96*96 + 256; i++) {
    if (font->moji[i] != 0) {
      free(font->moji[i]);
    }
  }
  free(font);
}

void Kanji_SetCodingSystem(Kanji_Font* font, Kanji_CodingSystem sys) {
  font->sys = sys;
}
