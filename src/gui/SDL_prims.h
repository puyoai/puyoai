#ifndef __SDL_prims_h
#define __SDL_prims_h

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

int SDL_DrawPixel(SDL_Surface *sfc, int x, int y, Uint32 colour);
int SDL_DrawHLine(SDL_Surface *sfc, int x1, int y1, int x2, Uint32 colour);
int SDL_DrawVLine(SDL_Surface *sfc, int x1, int y1, int y2, Uint32 colour);
int SDL_DrawLine(SDL_Surface *sfc, int x1, int y1, int x2, int y2, Uint32 colour);
int SDL_FillLine(SDL_Surface *sfc, int x1, int y1, int x2, int y2, unsigned int width, Uint32 colour);
int SDL_DrawRect(SDL_Surface *sfc, SDL_Rect *rect, Uint32 colour);
int SDL_DrawCircle(SDL_Surface *sfc, int x, int y, int radius, Uint32 colour);
int SDL_FillCircle(SDL_Surface *sfc, int x, int y, int radius, Uint32 colour);
int SDL_DrawPolygon(SDL_Surface *sfc, SDL_Point *vertices, int nVertices, Uint32 colour);
int SDL_FillPolygon(SDL_Surface *sfc, SDL_Point *vertices, int nVertices, Uint32 colour);

#ifdef __cplusplus
}
#endif

#endif /* __SDL_prims_h */
