#define __STDC_CONSTANT_MACROS

#include <assert.h>
#include <stdint.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <SDL.h>

#include "field.h"

DEFINE_bool(show_video, true, "");
DEFINE_int32(num_matches, 0, "");
DEFINE_string(out, "out.log", "");
DEFINE_bool(is_2p, false, "");

#define PIC_WIDTH  640
#define PIC_HEIGHT 480
#define PIC_FORMAT PIX_FMT_YUV420P

//typedef int8_t byte;

static uint8_t *buffer = NULL;

static SDL_Surface* scr = NULL;

enum State {
  CHAOS, START, NEXT_READY, NEXT_EMPTY,
};

State state = CHAOS;

static const int W = 20;
static const int H = 14;
float prev_rgb[H][W][3];
byte prev_next[6];
vector<LP> expected_plans;

static int g_next_x, g_next_next_x, g_field_x;

struct Box {
  int sx, sy, dx, dy;
};
Box BB[W][H];
bool bb_initialized;

struct Hist {
  LF f;
  byte p[2];
};
vector<Hist*> history;

void error(const char *msg){
    fprintf(stderr, msg);
    fprintf(stderr, "\n");
    exit(-1);
}

void saveFrame(AVFrame *pFrame, int width, int height, int saveFrameCount) {
  FILE *fp;
  char szFilename[32];

  sprintf(szFilename, "t/frame%d.ppm", saveFrameCount);
  fp = fopen(szFilename, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't open file for save.\n");
    return;
  }

  /* Write PPM Header.
   * P6 = Portable Pixcel Map (PPM)
   * @see http://netpbm.sourceforge.net/doc/ppm.html
   */
  fprintf(fp, "P6\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    fwrite(pFrame->data[0]+y * pFrame->linesize[0], 1, width*3, fp);
  }
  fclose(fp);
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
  case 1:
    *p = pixel;
    break;

  case 2:
    *(Uint16 *)p = pixel;
    break;

  case 3:
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    } else {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }
    break;

  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}

static float COLOR_BASES[][3] = {
  //{56.587992,44.975155,7.6718425},  // bg
  {111.9306415,80.7184255,39.6759835},  // yellow
  {96.3354035,45.0175975,85.491718},  // purple
  {55.6363003333333,74.040718,95.2091093333333},  // blue
  {104.126297,52.8944095,58.0559005},  // red
  {57.26087,84.612835,36.7795025},  // green
  {78.750519,80.8354035,64.9151135},  // ojama
};

enum {
  PURPLE = 8,
};

static int COLOR_NAMES[] = {
  YELLOW,
  PURPLE,
  BLUE,
  RED,
  GREEN,
  OJAMA,
};

struct HSV {
  float h, s, v;
};

HSV toHSV(float* c) {
  float mx = -1;
  float mn = 257;
  for (int i = 0; i < 3; i++) {
    mx = max(mx, c[i]);
    mn = min(mn, c[i]);
  }
  HSV r;
  if (mx == mn) {
    r.h = 180;
  } else if (mx == c[0]) {
    r.h = 60 * (c[1] - c[2]) / (mx - mn);
  } else if (mx == c[1]) {
    r.h = 60 * (c[2] - c[0]) / (mx - mn) + 120;
  } else if (mx == c[2]) {
    r.h = 60 * (c[0] - c[1]) / (mx - mn) + 240;
  } else {
    abort();
  }
  if (r.h < 0) r.h += 360;
  r.s = mx == 0 ? 0 : (mx - mn) / mx;
  r.v = mx;
  return r;
}

byte toPuyo(float* c, int x, int y) {
  HSV hsv = toHSV(c);
  int o = EMPTY;
/*
  printf("%d %d %f %f %f %f %f %f\n",
         x, y, c[0], c[1], c[2], hsv.h, hsv.s, hsv.v);
*/
  if (hsv.h > 192.1 && hsv.h < 232.1) {
    o = BLUE;
  } else if (hsv.s > 0.3 && hsv.s < 0.8) {
    if (hsv.h > 334 || hsv.h < 14) {
      o = RED;
    } else if (hsv.h > 14 && hsv.h < 54 && hsv.v > 95) {
      o = YELLOW;
    } else if (hsv.h > 79 && hsv.h < 119 && c[2] > 13) {
      o = GREEN;
    } else if (hsv.h > 292.7 && hsv.h < 332.7) {
      o = PURPLE;
    } else  if (hsv.h > 290 && c[2] > 60) {
      o = PURPLE;
    }
  } else {
    if (c[2] > 30) {
      o = OJAMA;
    }
  }
  return o;
}

void showColor(float* c, int x, int y) {
  HSV hsv = toHSV(c);
  printf("%d %d %f %f %f %f %f %f\n",
         x, y, c[0], c[1], c[2], hsv.h, hsv.s, hsv.v);
}

float colorDiff(float* c1, float* c2) {
  float d = 0;
  for (int i = 0; i < 3; i++) {
    float v = c1[i] - c2[i];
    d += v * v;
  }
  return sqrt(d);
}

void removeFlyingPuyo(LF* f) {
  for (int x = 1; x <= 6; x++) {
    bool empty_found = false;
    for (int y = 1; y <= 12; y++) {
      if (f->Get(x, y) == EMPTY)
        empty_found = true;
      else if (empty_found)
        f->Set(x, y, EMPTY);
    }
  }
}

void emitHistory(const vector<Hist*>& history) {
  int num_colors[12];
  memset(num_colors, 0, sizeof(num_colors));

  for (size_t i = 0; i < history.size(); i++) {
    Hist* h = history[i];
    for (int x = 1; x <= 6; x++) {
      bool empty_found = false;
      for (int y = 1; y <= 12; y++) {
        if (h->f.Get(x, y) == EMPTY)
          empty_found = true;
        else if (empty_found)
          h->f.Set(x, y, EMPTY);
      }
    }
  }

  for (size_t i = 0; i < history.size(); i++) {
    Hist* h = history[i];
    for (int y = 1; y <= 12; y++) {
      for (int x = 1; x <= 6; x++) {
        num_colors[h->f.Get(x, y)]++;
      }
    }
  }

  int num_fewest_color = 1000;
  byte fewest_color = 0;
  for (byte c = RED; c <= GREEN + 1; c++) {
    if (c == GREEN + 1) {
      c = PURPLE;
    }
    if (num_fewest_color > num_colors[c]) {
      num_fewest_color = num_colors[c];
      fewest_color = c;
    }
  }

  for (size_t i = 0; i < history.size(); i++) {
    Hist* h = history[i];
    for (int y = 1; y <= 12; y++) {
      for (int x = 1; x <= 6; x++) {
        if (h->f.Get(x, y) == PURPLE) {
          if (fewest_color == PURPLE) {
            printf("error: %d at %d %d\n", h->f.Get(x, y), x, y);
            h->f.Set(x, y, EMPTY);
          } else {
            h->f.Set(x, y, fewest_color);
          }
        } else if (h->f.Get(x, y) == fewest_color) {
          h->f.Set(x, y, EMPTY);
          printf("error: %d at %d %d\n", h->f.Get(x, y), x, y);
        }
      }
    }
    if (h->p[0] == PURPLE)
      h->p[0] = fewest_color;
    if (h->p[1] == PURPLE)
      h->p[1] = fewest_color;
  }

  const char* kCharMap = "05??1234";

  FILE* fp = fopen(FLAGS_out.c_str(), "ab");
  for (size_t i = 0; i < history.size() - 2; i++) {
    for (int y = 12; y >= 1; y--) {
      for (int x = 1; x <= 6; x++) {
        fputc(kCharMap[history[i]->f.Get(x, y)], fp);
      }
    }
    fputc(' ', fp);
    for (int j = 0; j < 3; j++) {
      fputc(kCharMap[history[i+j]->p[0]], fp);
      fputc(kCharMap[history[i+j]->p[1]], fp);
    }
    fputc(' ', fp);
    for (int y = 12; y >= 1; y--) {
      for (int x = 1; x <= 6; x++) {
        fputc(kCharMap[history[i+1]->f.Get(x, y)], fp);
      }
    }
    fputc('\n', fp);

  }
  fprintf(fp, "=== end ===\n");
  fclose(fp);

  static int num_matches = 0;
  fprintf(stderr, "match %d finished\n", ++num_matches);

  if (FLAGS_num_matches) {
    if (FLAGS_num_matches == num_matches) {
      exit(0);
    }
  }
}

void initBB(float bw, float bh) {
  bb_initialized = true;

  for (int x = 0; x < W; x++) {
    for (int y = 0; y < H; y++) {
      Box* bb = &BB[x][y];
      bb->sx = 2;
      bb->sy = 2;
      bb->dx = (int)bw - 2;
      bb->dy = (int)bh - 2;

      if (x == 9) {
        bb->sx = 6;
        bb->dx = 14;
      } else if (x == 10) {
        bb->sx = 13;
        bb->dx = 18;
      } else if (x == 11) {
        bb->sx = 6;
        bb->dx = (int)bw;
      }
    }
  }
}

void calcRGB(AVFrame* pFrame, int x, int y, float bw, float bh,
             float* r, float* g, float* b) {
  int px = (int)ceil(x * bw);
  int py = (int)ceil(y * bh);
  Box* bb = &BB[x][y];
  float c = *r = *g = *b = 0;
  int is = bb->sx;
  int ie = bb->dx;
  for (int j = bb->sy; j < bb->dy; j++) {
    unsigned char* p = pFrame->data[0] + (py + j) * pFrame->linesize[0];
    for (int i = is; i < ie; i++) {
      *r += p[(px+i)*3];
      *g += p[(px+i)*3+1];
      *b += p[(px+i)*3+2];
      c++;
    }
  }
  *r /= c;
  *g /= c;
  *b /= c;
}

void fillRGB(AVFrame* pFrame, int x, int y, float bw, float bh,
             float(*rgb)[H][W][3]) {
  float r = 0, g = 0, b = 0;
  calcRGB(pFrame, x, y, bw, bh, &r, &g, &b);
  (*rgb)[y][x][0] = r;
  (*rgb)[y][x][1] = g;
  (*rgb)[y][x][2] = b;
}

void fillAllRGB(AVFrame* pFrame, float bw, float bh,
                float(*rgb)[H][W][3]) {
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      fillRGB(pFrame, x, y, bw, bh, rgb);
      //printf("%d %d %f %f %f\n", x, y, r, g, b);
    }
  }
}

void setField(AVFrame* pFrame, float bw, float bh,
              LF* f, int* num_ojama, int* num_color, float(*rgb)[H][W][3]) {
  fillAllRGB(pFrame, bw, bh, rgb);

  for (int y = 0; y < 12; y++) {
    for (int x = 0; x < 6; x++) {
      float* c = (*rgb)[y+1][x+g_field_x];
      byte o = toPuyo(c, x, y);
      if (o == OJAMA)
        (*num_ojama)++;
      if (o != EMPTY)
        (*num_color)++;
      f->Set(x+1, 12-y, o);
    }
  }
}

void showFrame(AVFrame* pFrame, int width, int height, int saveFrameCount) {
  float bw = (float)width / W;
  float bh = (float)height / H;

  if (!bb_initialized)
    initBB(bw, bh);

  float rgb[H][W][3];
  LF f;
  int num_ojama = 0, num_color = 0;
  //setField(pFrame, bw, bh, &f, &num_ojama, &num_color, &rgb);

  fillRGB(pFrame, g_next_x, 4, bw, bh, &rgb);
  fillRGB(pFrame, g_next_x, 3, bw, bh, &rgb);
  fillRGB(pFrame, g_next_next_x, 5, bw, bh, &rgb);
  fillRGB(pFrame, g_next_next_x, 4, bw, bh, &rgb);

  byte next[6];
  next[0] = EMPTY;
  next[1] = EMPTY;
  next[2] = toPuyo(rgb[4][g_next_x], g_next_x, 4);
  next[3] = toPuyo(rgb[3][g_next_x], g_next_x, 3);
  next[4] = toPuyo(rgb[5][g_next_next_x], g_next_next_x, 5);
  next[5] = toPuyo(rgb[4][g_next_next_x], g_next_next_x, 4);

  //printf("state=%d\n", state);

  State nstate = state;
  switch (state) {
  case CHAOS: {
    setField(pFrame, bw, bh, &f, &num_ojama, &num_color, &rgb);
    if (num_ojama + num_color == 0) {
      nstate = START;
    }
    break;
  }
  case START: {
    setField(pFrame, bw, bh, &f, &num_ojama, &num_color, &rgb);
    if (num_ojama + num_color == 0 &&
        next[2] && next[3] &&
        next[2] != OJAMA && next[3] != OJAMA) {
      nstate = NEXT_READY;
    }
    break;
  }
  case NEXT_READY: {
    if (!next[2] && !next[3]) {
      nstate = NEXT_EMPTY;
      setField(pFrame, bw, bh, &f, &num_ojama, &num_color, &rgb);

      removeFlyingPuyo(&f);
      num_color = f.countColorPuyo();
      //num_ojama = f.countPuyo() - num_color;
      if (num_color && !num_ojama && !expected_plans.empty()) {
        bool ok = false;
        for (size_t i = 0; i < expected_plans.size(); i++) {
          if (f.isEqual(expected_plans[i].field)) {
            ok = true;
            break;
          }
        }
        if (!ok) {
          LOG(INFO) << "Inconsistent! color=" << num_color
                    << " ojama=" << num_ojama << ": " << f.GetDebugOutput();
          nstate = NEXT_READY;
          break;
        }
      }

      string next_str;
      next_str.push_back(prev_next[2]);
      next_str.push_back(prev_next[3]);
      f.FindAvailablePlans(next_str, 1, &expected_plans);
    }
    break;
  }
  case NEXT_EMPTY: {
    if (next[2] && next[3] &&
        next[2] != OJAMA && next[3] != OJAMA &&
        colorDiff(prev_rgb[4][g_next_x], rgb[4][g_next_x]) < 2 &&
        colorDiff(prev_rgb[3][g_next_x], rgb[3][g_next_x]) < 2) {
      nstate = NEXT_READY;
      setField(pFrame, bw, bh, &f, &num_ojama, &num_color, &rgb);
    }
    break;
  }
  }
  if (!next[2] && !next[3] && !next[4] && !next[5]) {
    setField(pFrame, bw, bh, &f, &num_ojama, &num_color, &rgb);
    if (num_ojama + num_color == 0) {
      nstate = START;
    }
  }

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      for (int i = 0; i < 3; i++) {
        prev_rgb[y][x][i] = rgb[y][x][i];
      }
    }
  }

  if (state != nstate) {
    printf("%d => %d\n", state, nstate);
    state = nstate;
    for (int y = 0; y < 12; y++) {
      for (int x = 0; x < 6; x++) {
        float* c = rgb[y+1][x+1];
        showColor(c, x, y);
      }
    }
    showColor(rgb[4][g_next_x], g_next_x, 4);
    showColor(rgb[3][g_next_x], g_next_x, 3);

    if (state == NEXT_READY) {
      for (int i = 0; i < 6; i++) {
        prev_next[i] = next[i];
      }
    }
    if (state == NEXT_EMPTY) {
      //LF::GetDebugOutputForNext(prev_next);
      printf("%s\n", f.GetDebugOutput().c_str());

      Hist* h = new Hist;
      h->f = f;
      h->p[0] = prev_next[2];
      h->p[1] = prev_next[3];
      history.push_back(h);
    }
    //F::showNext(next);
    //f.show();

    if (state == START) {
      if (history.size() > 2) {
        emitHistory(history);
      }
      for (size_t i = 0; i < history.size(); i++) {
        delete history[i];
      }
      history.clear();
      puts("=== GAME START ===");
    }
  }

  if (scr) {
    fillAllRGB(pFrame, bw, bh, &rgb);

    for (int y = 0; y < height; y++) {
      unsigned char* p = pFrame->data[0]+y * pFrame->linesize[0];
      for (int x = 0; x < width; x++) {
        unsigned char r = p[x*3];
        unsigned char g = p[x*3+1];
        unsigned char b = p[x*3+2];
        putpixel(scr, x, y, SDL_MapRGB(scr->format, r, g, b));
      }
    }

    for (int x = 0; x < W; x++) {
      for (int y = 0; y < H; y++) {
        float* c = rgb[y][x];
        byte o = toPuyo(c, x, y);
        Uint32 col;
        switch (o) {
        case OJAMA:
          col = SDL_MapRGB(scr->format, 255, 255, 255); break;
        case RED:
          col = SDL_MapRGB(scr->format, 255, 0, 0); break;
        case BLUE:
          col = SDL_MapRGB(scr->format, 0, 0, 255); break;
        case YELLOW:
          col = SDL_MapRGB(scr->format, 255, 255, 0); break;
        case GREEN:
          col = SDL_MapRGB(scr->format, 0, 255, 130); break;
        case PURPLE:
          col = SDL_MapRGB(scr->format, 255, 0, 255); break;
        default:
          col = SDL_MapRGB(scr->format, 130, 130, 130); break;
        }

        Box* bb = &BB[x][y];
        int bx = (int)ceil(x*bw);
        int by = (int)ceil(y*bh);
        for (int i = bb->sx; i < bb->dx; i++) {
          putpixel(scr, bx + i, by + bb->sy - 1, col);
          putpixel(scr, bx + i, by + bb->dy + 1, col);
        }
        for (int i = bb->sy; i < bb->dy; i++) {
          putpixel(scr, bx + bb->sx - 1, by + i, col);
          putpixel(scr, bx + bb->dx + 1, by + i, col);
        }
      }
    }

#if 0
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < W; x++) {
        putpixel(scr, (int)ceil(x * bw), y,
                 SDL_MapRGB(scr->format, 0, 0, 255));
      }

      putpixel(scr, (int)ceil(9 * bw) + 5, y,
               SDL_MapRGB(scr->format, 255, 0, 0));
      putpixel(scr, (int)ceil(9 * bw) + 15, y,
               SDL_MapRGB(scr->format, 255, 0, 0));
      putpixel(scr, (int)ceil(10 * bw) + 12, y,
               SDL_MapRGB(scr->format, 255, 0, 0));
      putpixel(scr, (int)ceil(10 * bw) + 19, y,
               SDL_MapRGB(scr->format, 255, 0, 0));

      putpixel(scr, (int)ceil(11 * bw) + 5, y,
               SDL_MapRGB(scr->format, 255, 0, 0));
      putpixel(scr, (int)ceil(11 * bw) + (int)bw, y,
               SDL_MapRGB(scr->format, 255, 0, 0));
/*
    if (x == 9) {
      is = 6;
      ie = 14;
    } else if (x == 10) {
      // from 168 to 176
      is = 10;
      ie = 18;
    } else if (x == 11) {
      is = 6;
      ie = (int)bw - 1;
    }
*/
    }
#endif

    SDL_Flip(scr);
  }
}

int main(int argc, char *argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  if (argc != 2) {
    error("specify the input");
  }

  if (FLAGS_is_2p) {
    g_next_x = 11, g_next_next_x = 10;
    g_field_x = 13;
  } else {
    g_next_x = 8, g_next_next_x = 9;
    g_field_x = 1;
  }

  unlink(FLAGS_out.c_str());

  if (FLAGS_show_video) {
    SDL_Init(SDL_INIT_VIDEO);

    scr = SDL_SetVideoMode(PIC_WIDTH, PIC_HEIGHT, 32, SDL_SWSURFACE);
    assert(scr);
  }

  // Registr all formats and codesc
  av_register_all();

  AVFormatContext *pFormatCtx = NULL;
  const char *filename = argv[1];


  // Open video file

  // dplicated:av_open_input_file

  // if (av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL) != 0) {
  if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
    fprintf(stderr, "Couldn't open file\n");
    return -1;
  }


  // Retrieve stream infomation
  if (av_find_stream_info(pFormatCtx) < 0) {
    fprintf(stderr, "Couldn't find stream infomation\n");
    return -1;
  }


  // Dump infomation about file onto standard error

  // duplicated: dump_format

  // dump_format(pFormatCtx, 0, filename, false);
  av_dump_format(pFormatCtx, 0, filename, false);


  // Find the first video stream
  int videoStream = -1;
  int streamLength = pFormatCtx->nb_streams;
  for (int i = 0; i < streamLength; i++) {
    if (AVMEDIA_TYPE_VIDEO == pFormatCtx->streams[i]->codec->codec_type) {
      videoStream = i;
      break;
    }
  }
  if (videoStream == -1) {
    fprintf(stderr, "Didn't find a video stream\n");
    return -1;
  }


  // Get apointer to the codec context for the video stream
  AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;


  // Find the decoder for the video stream
  AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec == NULL) {
    fprintf(stderr, "Codec not found\n");
    return -1;
  }


  // Inform the codec that we can handle truncated bitstreams -- i.e.,

  // bitstreams where frame boundries can fall in the middle of packets
  if (pCodec->capabilities & CODEC_CAP_TRUNCATED) {
    pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;
  }


  // Open codec
  if (avcodec_open(pCodecCtx, pCodec) < 0) {
    fprintf(stderr, "Couldn't open codec\n");
    return -1;
  }


  // Hack to correct wrong frame rates that seem to be generated by some
  // codes
  //if (pCodecCtx->frame_rate > 1000 && pCodecCtx->frame_rate_base == 1) {
  //  pCodecCtx->frame_rate_base = 1000;
  //}


  // Allocate video frame
  AVFrame *pFrame = avcodec_alloc_frame();


  // Allocate an AVFrame structure
  AVFrame *pFrameRGB = avcodec_alloc_frame();
  if (pFrameRGB == NULL) {
    fprintf(stderr, "Couldn't allocate frame memory\n");
    return -1;
  }


  // Determin required buffer size and allocate buffer
  int numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                                    pCodecCtx->height);

  /* changed from 'new' to 'calloc' for C language. */

  // buffer = new uint8_t[[numBytes];
  uint8_t *buffer = (uint8_t*)calloc(sizeof(uint8_t), numBytes);


  // Assign appropriate parts of buffer to image planes in pFrameRGB
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width,
                 pCodecCtx->height);


  // read frames and first five frames to disk
  AVPacket packet;
  int frameFinished;
  int saveFrameCount = 1;
  static struct SwsContext *swsCtx;
  /* SWS means SoftWare Scaling */


  bool stop = false;
  /* packet is allocated in the av_read_frame function. */
  while (1) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_QUIT:
        return 0;
      case SDL_KEYDOWN:
        stop = !stop;
      }
    }
    if (stop) {
      SDL_Delay(100);
      continue;
    }

    if (av_read_frame(pFormatCtx, &packet) < 0)
      break;

    if (packet.stream_index == videoStream) {
      // duplicated
      // avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, packet.data, packet.size);
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

      if (frameFinished) {
        /* Allocates and returns a SwsContext. */
        swsCtx = sws_getCachedContext(swsCtx,
                                      pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                      pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24,
                                      SWS_BICUBIC,
                                      NULL,
                                      /* source filter */
                                      NULL,
                                      /* destination filter */
                                      NULL);

        sws_scale(swsCtx,
                  (const uint8_t *const *)pFrame->data,
                  pFrame->linesize,
                  0, pCodecCtx->height,
                  pFrameRGB->data, pFrameRGB->linesize);
        //fprintf(stdout, ".");

        // Save the frame to disk;
#if 0
        if (saveFrameCount % 100 == 0) {
          saveFrame(pFrameRGB,
                    pCodecCtx->width, pCodecCtx->height, saveFrameCount);
        }
        saveFrameCount++;
#endif
        showFrame(pFrameRGB,
                  pCodecCtx->width, pCodecCtx->height, saveFrameCount);
      }
    }
    av_free_packet(&packet);
  }

  /* Free the SwsContext */
  sws_freeContext(swsCtx);

  // Free the RGB image

  // delete [] buffer;

  /* changed from 'delete' to 'free' for C language. */
  free(buffer);

  // Free the YUV frame
  av_free(pFrame);

  // Close the codec
  avcodec_close(pCodecCtx);

  // Closes the video file
  av_close_input_file(pFormatCtx);

  SDL_Quit();

  return 0;
}
