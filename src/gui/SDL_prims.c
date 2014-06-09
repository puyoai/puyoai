/* SDL_prims.c -- 2D graphical primitives for SDL
 * 
 * Copyright (c) 2008 Ian Piumarta
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the 'Software'),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, provided that the above copyright notice(s) and this
 * permission notice appear in all copies of the Software and that both the
 * above copyright notice(s) and this permission notice appear in supporting
 * documentation.
 * 
 * THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
 * 
 * Last edited: 2008-06-12 12:32:57 by piumarta on WINDOWS-XP.piumarta.com
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SDL_prims.h"

#define abs(N)		(((N) < 0) ? -(N) : (N))
#define sgn(N)		(((N) < 0) ? -1 : ((N) > 0 ? 1 : 0))
#define swap(T, A, B)	do { T tmp= A;  A= B;  B= tmp; } while (0)
#define min(A, B)	(((A) < (B) ? (A) : (B)))
#define max(A, B)	(((A) > (B) ? (A) : (B)))
#define clamp(A, X, B)	min(max(A, X), B)

#define CLIPX0(S)	((S)->clip_rect.x)
#define CLIPY0(S)	((S)->clip_rect.y)
#define CLIPW(S)	((S)->clip_rect.w)
#define CLIPH(S)	((S)->clip_rect.h)
#define CLIPX1(S)	(CLIPX0(S) + CLIPW(S))
#define CLIPY1(S)	(CLIPY0(S) + CLIPH(S))

#define CLIPX(S, X)	clamp(CLIPX0(S), X, CLIPX1(S) - 1)
#define CLIPY(S, Y)	clamp(CLIPY0(S), Y, CLIPY1(S) - 1)

#define INCLIPX(S, X)	((CLIPX0(S) <= (X)) && ((X) < CLIPX1(S)))
#define INCLIPY(S, Y)	((CLIPY0(S) <= (Y)) && ((Y) < CLIPY1(S)))
#define INCLIP(S, X, Y)	(INCLIPX(S, X) && INCLIPY(S, Y))

/* ---------------------------------------------------------------- */
/* DrawPixel							    */
/* ---------------------------------------------------------------- */

static inline int DrawPixel8(SDL_Surface *s, int x, int y, Uint32 c)
{
  *((Uint8 *)(s->pixels + s->pitch * y + x))= (Uint8)c;
  return 0;
}

static inline int DrawPixel16(SDL_Surface *s, int x, int y, Uint32 c)
{
  *((Uint16 *)(s->pixels + s->pitch * y + 2 * x))= (Uint16)c;
  return 0;
}

static inline int DrawPixel32(SDL_Surface *s, int x, int y, Uint32 c)
{
  *((Uint32 *)(s->pixels + s->pitch * y + 4 * x))= (Uint32)c;
  return 0;
}

int SDL_DrawPixel(SDL_Surface *s, int x, int y, Uint32 c)
{
  if (INCLIP(s, x, y))
    {
      switch (s->format->BytesPerPixel)
	{
	case 1: return DrawPixel8 (s, x, y, c);
	case 2: return DrawPixel16(s, x, y, c);
	case 3:
#	        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
	        colour <<= 8;
#	        endif
	case 4: return DrawPixel32(s, x, y, c);
	}
    }
  return -1;
}

/* ---------------------------------------------------------------- */

/* Cohen-Sutherland clipping algorithm.
 * James D. Foley, Andries van Dam, et al, "Computer Graphics:
 * Principles and Practice", Addison-Wesley, 1995.  ISBN 0201848406.
 * (Section 3.12.3.) */

enum {
  ClipLeft   = 0x1,
  ClipRight  = 0x2,
  ClipBottom = 0x4,
  ClipTop    = 0x8
};

#define CLIP_INSIDE(C)		(!(C))
#define CLIP_REJECT(A, B)	((A) & (B))
#define CLIP_ACCEPT(A, B)	(!((A)|(B)))

static int clipEncode(int x, int y, int l, int t, int r, int b)
{
  int code= 0;
  if      (x < l) code |= ClipLeft;
  else if (x > r) code |= ClipRight;
  if      (y < t) code |= ClipTop;
  else if (y > b) code |= ClipBottom;
  return code;
}

static int clipLine(SDL_Surface *dst, int *x1, int *y1, int *x2, int *y2)
{
  int visible= 0;
  int l= dst->clip_rect.x;
  int r= dst->clip_rect.x + dst->clip_rect.w - 1;
  int t= dst->clip_rect.y;
  int b= dst->clip_rect.y + dst->clip_rect.h - 1;

  for (;;)
    {
      int code1= clipEncode(*x1, *y1, l, t, r, b);
      int code2= clipEncode(*x2, *y2, l, t, r, b);
      if (CLIP_ACCEPT(code1, code2))
	{
	  visible= 1;
	  break;
	}
      else if (CLIP_REJECT(code1, code2))
	break;
      else
	{
	  double m;
	  if (CLIP_INSIDE(code1))
	    {
	      swap(int, *x1, *x2);
	      swap(int, *y1, *y2);
	      swap(int, code1, code2);
	    }
	  m= ((*x2 != *x1) ? (*y2 - *y1) / (double)(*x2 - *x1) : 1.0L);
	  if      (code1 & ClipLeft)	{ 		  *y1 += (int)((l - *x1) * m);  *x1= l; }
	  else if (code1 & ClipRight)	{ 		  *y1 += (int)((r - *x1) * m);  *x1= r; }
	  else if (code1 & ClipBottom)	{ if (*x2 != *x1) *x1 += (int)((b - *y1) / m);  *y1= b; }
	  else if (code1 & ClipTop)	{ if (*x2 != *x1) *x1 += (int)((t - *y1) / m);  *y1= t; }
	}
    }
  return visible;
}

/* Dynamic differential analyser for lines.
 * Jack E. Bresenham, "Algorithm for computer control of a digital
 * plotter", IBM Systems Journal 4(1), January 1965, pp. 25--30.
 * (Special-cased for horiz/vert; general case optimised to remove
 * conditionals from inner loop.) */

#define DrawLine(N, M)									\
static int DrawHLine##N(SDL_Surface *s, int x1, int y1, int x2, Uint32 colour)		\
{											\
  Uint8 *p= s->pixels + s->pitch * y1;							\
  Uint##M c= (Uint##M)colour;								\
  while (x1 <= x2)									\
    {											\
      *((Uint##M *)(p + ((M)>>3) * x1))= c;						\
      ++x1;										\
    }											\
  return 0;										\
}											\
											\
static int DrawVLine##N(SDL_Surface *s, int x1, int y1, int y2, Uint32 colour)		\
{											\
  Uint8 *p= s->pixels + ((M)>>3) * x1;							\
  Uint##M c= (Uint##M)colour;								\
  while (y1 <= y2)									\
    {											\
      *((Uint##M *)(p + s->pitch * y1))= c;						\
      ++y1;										\
    }											\
  return 0;										\
}											\
											\
static int DrawLine##N(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 colour)	\
{											\
  Uint##M c= (Uint##M)colour;								\
  int nx= x2 - x1, ax= abs(nx);								\
  int ny= y2 - y1, ay= abs(ny);								\
  if (ax >= ay)	/* horizontal */							\
    {											\
      int    x= x1, dx= sgn(nx);							\
      double y= y1, dy= (double)(y2 - y1) / (double)ax;					\
      while (ax-- >= 0)									\
	{										\
	  *((Uint##M *)(s->pixels + s->pitch * (int)(y) + ((M)>>3) * x))= c;		\
	  x += dx;									\
	  y += dy;									\
	}										\
    }											\
  else		/* vertical */								\
    {											\
      int    y= y1, dy= sgn(ny);							\
      double x= x1, dx= (double)(x2 - x1) / (double)ay;					\
      while (ay-- >= 0)									\
	{										\
	  *((Uint##M *)(s->pixels + s->pitch * y + ((M)>>3) * (int)(x)))= c;		\
	  y += dy;									\
	  x += dx;									\
	}										\
    }											\
  return 0;										\
}

DrawLine( 8,  8)
DrawLine(16, 16)
DrawLine(32, 32)

#undef DrawLine

int SDL_DrawHLine(SDL_Surface *s, int x1, int y1, int x2, Uint32 colour)
{
  if (INCLIPY(s, y1) && (INCLIPX(s, x1) || INCLIPX(s, x2)))
    {
      if (x1 > x2) swap(int, x1, x2);
      x1= CLIPX(s, x1);
      x2= CLIPX(s, x2);
      switch (s->format->BytesPerPixel)
	{
	case 1: return DrawHLine8 (s, x1, y1, x2, colour);
	case 2: return DrawHLine16(s, x1, y1, x2, colour);
	case 3:
#		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		colour <<= 8;
#		endif
	case 4: return DrawHLine32(s, x1, y1, x2, colour);
	}
    }
  return -1;
}

int SDL_DrawVLine(SDL_Surface *s, int x1, int y1, int y2, Uint32 colour)
{
  if (INCLIPX(s, x1) && (INCLIPY(s, y1) || INCLIPY(s, y2)))
    {
      if (y1 > y2) swap(int, y1, y2);
      y1= CLIPY(s, y1);
      y2= CLIPY(s, y2);
      switch (s->format->BytesPerPixel)
	{
	case 1: return DrawVLine8 (s, x1, y1, y2, colour);
	case 2: return DrawVLine16(s, x1, y1, y2, colour);
	case 3:
#		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		colour <<= 8;
#		endif
	case 4: return DrawVLine32(s, x1, y1, y2, colour);
	}
    }
  return -1;
}

int SDL_DrawLine(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 colour)
{
  if (x1 == x2) return SDL_DrawVLine(s, x1, y1, y2, colour);
  if (y1 == y2) return SDL_DrawHLine(s, x1, y1, x2, colour);
  if (clipLine(s, &x1, &y1, &x2, &y2))
    {
      switch (s->format->BytesPerPixel)
	{
	case 1: return DrawLine8 (s, x1, y1, x2, y2, colour);
	case 2: return DrawLine16(s, x1, y1, x2, y2, colour);
	case 3:
#		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		colour <<= 8;
#		endif
	case 4: return DrawLine32(s, x1, y1, x2, y2, colour);
	}
    }
  return -1;
}

/* ---------------------------------------------------------------- */
/* FillLine							    */
/* ---------------------------------------------------------------- */

int SDL_FillLine(SDL_Surface *s, int x1, int y1, int x2, int y2, unsigned int w, Uint32 colour)
{
  if (w == 0) return 0;
  if (w == 1) return SDL_DrawLine(s, x1, y1, x2, y2, colour);
  if (clipLine(s, &x1, &y1, &x2, &y2))
    {
      w >>= 1;
      if (x1 == x2)	/* vertical */
	{
	  SDL_Point v[4]= { { x1 - w, y1 }, { x1 - w, y2 }, { x1 + w, y2 }, { x1 + w, y1 } };
	  return SDL_FillPolygon(s, v, 4, colour);
	}
      if (y1 == y2)	/* horizontal */
	{
	  SDL_Point v[4]= { { x1, y1 -w }, { x1, y2 + w }, { x1, y2 + w }, { x1, y1 - w } };
	  return SDL_FillPolygon(s, v, 4, colour);
	}
      double slope= (double)(y1 - y2) / (double)(x1 - x2);
      double angle= atan(slope);
      double width= (double)w;
      int dx= (int)rint(width * sin(angle));
      int dy= (int)rint(width * cos(angle));
      SDL_Point v[4]= { { x1 - dx, y1 + dy }, { x2 - dx, y2 + dy }, { x2 + dx, y2 - dy }, { x1 + dx, y1 - dy } };
      return SDL_FillPolygon(s, v, 4, colour);      
    }
  return -1;
}

/* ---------------------------------------------------------------- */
/* DrawRect							    */
/* ---------------------------------------------------------------- */

int SDL_DrawRect(SDL_Surface *s, SDL_Rect *rect, Uint32 colour)
{
  int x1= rect->x, y1= rect->y, x2= x1 + rect->w, y2= y1 + rect->h;
  if (( SDL_DrawLine(s, x1, y1, x1, y2, colour))
      | SDL_DrawLine(s, x1, y2, x2, y2, colour)
      | SDL_DrawLine(s, x2, y2, x2, y1, colour)
      | SDL_DrawLine(s, x2, y1, x1, y1, colour))
    return -1;
  return 0;
}

/* ---------------------------------------------------------------- */

/* Midpoint circle algorithm.
 * J. R. Van Aken, "An Efficient Ellipse Drawing Algorithm",
 * CG&A 4(9), September 1984, pp. 24--35. */

#define DrawCircle(N, M)							\
static inline void xDrawPixel##N(SDL_Surface *s, int x, int y, Uint##M c)	\
{										\
  if (INCLIP(s, x, y))								\
    *((Uint##M *)(s->pixels + (s->pitch * y) + (((M) >> 3) * x)))= c;		\
}										\
										\
int DrawCircle##N(SDL_Surface *s, int x0, int y0, int radius, Uint32 colour)	\
{										\
  int f= 1 - radius;								\
  int dx= 0;									\
  int dy= -2 * radius;								\
  int x= 0;									\
  int y= radius;								\
  xDrawPixel##M(s, x0, y0 + radius, colour);					\
  xDrawPixel##M(s, x0, y0 - radius, colour);					\
  xDrawPixel##M(s, x0 + radius, y0, colour);					\
  xDrawPixel##M(s, x0 - radius, y0, colour);					\
  while (x < y)									\
    {										\
      if (f >= 0)								\
	{									\
	  y--;									\
	  dy += 2;								\
	  f += dy;								\
	}									\
      x++;									\
      dx += 2;									\
      f += dx + 1;								\
      xDrawPixel##M(s, x0 + x, y0 + y, colour);					\
      xDrawPixel##M(s, x0 - x, y0 + y, colour);					\
      xDrawPixel##M(s, x0 + x, y0 - y, colour);					\
      xDrawPixel##M(s, x0 - x, y0 - y, colour);					\
      xDrawPixel##M(s, x0 + y, y0 + x, colour);					\
      xDrawPixel##M(s, x0 - y, y0 + x, colour);					\
      xDrawPixel##M(s, x0 + y, y0 - x, colour);					\
      xDrawPixel##M(s, x0 - y, y0 - x, colour);					\
    }										\
  return 0;									\
}

DrawCircle( 8,  8)
DrawCircle(16, 16)
DrawCircle(32, 32)

#undef DrawCircle

/* ---------------------------------------------------------------- */
/* DrawCircle							    */
/* ---------------------------------------------------------------- */

int SDL_DrawCircle(SDL_Surface *s, int x0, int y0, int radius, Uint32 colour)
{
  if (radius > 0
      && (x0 + radius >= CLIPX0(s) || CLIPX1(s) - 1 <= x0 - radius)
      && (y0 + radius >= CLIPY0(s) || CLIPY1(s) - 1 <= y0 - radius))
    {
      switch (s->format->BytesPerPixel)
	{
	case 1: return DrawCircle8 (s, x0, y0, radius, colour);
	case 2: return DrawCircle16(s, x0, y0, radius, colour);
	case 3:
#		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		colour <<= 8;
#		endif
	case 4: return DrawCircle32(s, x0, y0, radius, colour);
	}
    }
  return -1;
}

/* ---------------------------------------------------------------- */
/* FillCircle							    */
/* ---------------------------------------------------------------- */

/* See DrawCircle above. */

#define FillCircle(N, M)								\
static void xDrawLine##N(SDL_Surface *s, int x0, int x1, int y, Uint##M c)		\
{											\
  if (!INCLIPY(s, y)) return;								\
  if (x0 < CLIPX0(s)) x0= CLIPX0(s);							\
  if (x1 >= CLIPX1(s)) x1= CLIPX1(s) - 1;						\
  Uint##M *p= (Uint##M *)(s->pixels + s->pitch * y + ((M) >> 3) * x0);			\
  x1 -= x0;										\
  while (x1-- >= 0)									\
    *p++= c;										\
}											\
											\
int FillCircle##N(SDL_Surface *s, int x0, int y0, int radius, Uint##M colour)		\
{											\
  int f= 1 - radius;									\
  int ddF_x= 0;										\
  int ddF_y= -2 * radius;								\
  int x= 0;										\
  int y= radius;									\
  xDrawPixel##N(s, x0, y0 + radius, colour);						\
  xDrawPixel##N(s, x0, y0 - radius, colour);						\
  xDrawLine##N(s, x0 - radius, x0 + radius, y0, colour);				\
  while (x < y)										\
    {											\
      if (f >= 0)									\
	{										\
	  y--;										\
	  ddF_y += 2;									\
	  f += ddF_y;									\
	}										\
      x++;										\
      ddF_x += 2;									\
      f += ddF_x + 1;									\
      xDrawLine##N(s, x0 - x, x0 + x, y0 + y, colour);					\
      xDrawLine##N(s, x0 - x, x0 + x, y0 - y, colour);					\
      xDrawLine##N(s, x0 - y, x0 + y, y0 + x, colour);					\
      xDrawLine##N(s, x0 - y, x0 + y, y0 - x, colour);					\
    }											\
  return 0;										\
}

FillCircle( 8,  8)
FillCircle(16, 16)
FillCircle(32, 32)

#undef FillCircle

int SDL_FillCircle(SDL_Surface *s, int x0, int y0, int radius, Uint32 colour)
{
  if (radius > 0
      && (x0 + radius >= CLIPX0(s) || CLIPX1(s) - 1 <= x0 - radius)
      && (y0 + radius >= CLIPY0(s) || CLIPY1(s) - 1 <= y0 - radius))
    {
      switch (s->format->BytesPerPixel)
	{
	case 1: return FillCircle8 (s, x0, y0, radius, colour);
	case 2: return FillCircle16(s, x0, y0, radius, colour);
	case 3:
#		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		colour <<= 8;
#		endif
	case 4: return FillCircle32(s, x0, y0, radius, colour);
	}
    }
  return -1;
}

/* ---------------------------------------------------------------- */
/* DrawPolygon							    */
/* ---------------------------------------------------------------- */

int SDL_DrawPolygon(SDL_Surface *s, SDL_Point *v, int n, Uint32 c)
{
  if (n == 0) return 0;
  if (n == 1) return SDL_DrawPixel(s, v->x, v->y, c);
  int i;
  for (i= 1;  i < n;  ++i)
    SDL_DrawLine(s, v[i-1].x, v[i-1].y, v[i].x, v[i].y, c);
  SDL_DrawLine(s, v[n-1].x, v[n-1].y, v[0].x, v[0].y, c);
  return 0;
}

/* ---------------------------------------------------------------- */
/* FillPolygon							    */
/* ---------------------------------------------------------------- */

/* Trivial scan-line fill algorithm.  For each scan line intersecting
 * the polygon, make an LR ordered list of crossing points.  Successive
 * pairs of points define runs of pixels lying within the polygon. */

int SDL_FillPolygon(SDL_Surface *s, SDL_Point *v, int n, Uint32 c)
{
  if (n == 0) return 0;
  if (n == 1) return SDL_DrawPixel(s, v->x, v->y, c);
  int nxs, *xs= alloca(sizeof(int) * n);
  int y, i, j, k;
  int y0= v[0].y, y1= y0;
  for (i= 1;  i < n;  ++i)
    {
      y= v[i].y;
      if (y < y0) y0= y;
      if (y > y1) y1= y;
    }
  if (y0 < CLIPY0(s)) y0= CLIPY0(s);
  if (y1 >= CLIPY1(s)) y1= CLIPY1(s) - 1;
  for (y= y0;  y <= y1;  ++y)
    {
      nxs= 0;
      j= n - 1;
      for (i= 0;  i < n;  j= i++)
	if ((v[i].y < y && y <= v[j].y) || (v[j].y < y && y <= v[i].y))
	  {
	    xs[nxs++]= (int)rint(v[i].x + ((double)y - v[i].y) / ((double)v[j].y - v[i].y) * ((double)v[j].x - v[i].x));
	    for (k= nxs - 1;  k && xs[k-1] > xs[k];  --k)
	      swap(int, xs[k-1], xs[k]);
	  }
      for (i= 0;  i < nxs;  i += 2)
	SDL_DrawHLine(s, xs[i], y, xs[i+1], c);
    }
  return 0;
}
