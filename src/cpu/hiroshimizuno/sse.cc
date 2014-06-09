#include "sse.h"

int Simulate(unsigned char field[96], int steps) {
  int vanished;
  __asm__ __volatile__(
//------------------------------------------------------------------------------
// Initialize [i]
//------------------------------------------------------------------------------
"pcmpeqb xmm6, xmm6;"  // xmm6(noncolor) <- 11111111
"pxor    xmm7, xmm7;"  // xmm7(noncolor) <- 00000000
"pxor    xmm8, xmm8;"  // xmm8(noncolor) <- 00000000
"pabsb   xmm9, xmm6;"  // xmm9(bit)      <- 00000001
"movdqa  xmm0, XMMWORD PTR [%2     ];"
"movdqa  xmm1, XMMWORD PTR [%2 + 16];"
"movdqa  xmm2, XMMWORD PTR [%2 + 32];"
"movdqa  xmm3, XMMWORD PTR [%2 + 48];"
"movdqa  xmm4, XMMWORD PTR [%2 + 64];"
"movdqa  xmm5, XMMWORD PTR [%2 + 80];"
"psllw   xmm9, 7;"     // xmm9(bit)      <- 10000000

"start:"
//------------------------------------------------------------------------------
// CountHands [h]
// [ASSUMES] xmm{678}[0] != ColorPuyo.  Because they must be different from 12 dan-me.
// [TODO]    Add workaround for 14 dan-me bug in Puyo2 and Fever.
//------------------------------------------------------------------------------
// Count horizontal hands.
"movdqa  xmm11,xmm1 ; movdqa  xmm12,xmm2 ; movdqa  xmm13,xmm3 ; movdqa  xmm14,xmm4 ; movdqa  xmm15,xmm5 ;"
"pcmpeqb xmm11,xmm0 ; pcmpeqb xmm12,xmm1 ; pcmpeqb xmm13,xmm2 ; pcmpeqb xmm14,xmm3 ; pcmpeqb xmm15,xmm4 ;"
"pabsb   xmm11,xmm11; pabsb   xmm12,xmm12; pabsb   xmm13,xmm13; pabsb   xmm14,xmm14; pabsb   xmm15,xmm15;"
"movdqa  xmm10, xmm11;"  // [h]
"paddusb xmm11, xmm12;"  // [h]
"paddusb xmm12, xmm13;"  // [h]
"paddusb xmm13, xmm14;"  // [h]
"paddusb xmm14, xmm15;"  // [h]
// Count vertical hands, the left half.
"palignr xmm6, xmm0, 1; palignr xmm7, xmm1, 1; palignr xmm8, xmm2, 1;"
"pcmpeqb xmm6, xmm0   ; pcmpeqb xmm7, xmm1   ; pcmpeqb xmm8, xmm2   ;"
"psubb   xmm10,xmm6   ; psubb   xmm11,xmm7   ; psubb   xmm12,xmm8   ;"
"pslldq  xmm6, 1      ; pslldq  xmm7, 1      ; pslldq  xmm8, 1      ;"
"psubb   xmm10,xmm6   ; psubb   xmm11,xmm7   ; psubb   xmm12,xmm8   ;"
// Count vertical hands, the right half.
"palignr xmm6, xmm3, 1; palignr xmm7, xmm4, 1; palignr xmm8, xmm5, 1;"
"pcmpeqb xmm6, xmm3   ; pcmpeqb xmm7, xmm4   ; pcmpeqb xmm8, xmm5   ;"
"psubb   xmm13,xmm6   ; psubb   xmm14,xmm7   ; psubb   xmm15,xmm8   ;"
"pslldq  xmm6, 1      ; pslldq  xmm7, 1      ; pslldq  xmm8, 1      ;"
"psubb   xmm13,xmm6   ; psubb   xmm14,xmm7   ; psubb   xmm15,xmm8   ;"

//------------------------------------------------------------------------------
// FindCoresHorizontally
// [ASSUMES] xmm9[every] == 0b10000000.
//------------------------------------------------------------------------------
"psllw xmm10,5; psllw xmm11,5; psllw xmm12,5; psllw xmm13,5; psllw xmm14,5; psllw xmm15,5;"
// Find cores horizontally, the left half.
"movdqa  xmm6, xmm1 ; movdqa  xmm7, xmm2 ; movdqa  xmm8, xmm3 ;"
"pcmpeqb xmm6, xmm0 ; pcmpeqb xmm7, xmm1 ; pcmpeqb xmm8, xmm2 ;"
"pand    xmm6, xmm11; pand    xmm7, xmm12; pand    xmm8, xmm13;"
"paddusb xmm6, xmm10; paddusb xmm7, xmm11; paddusb xmm8, xmm12;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ; pand    xmm8, xmm9 ;"  // This also makes sure xmm8 != ColorPuyo.
"por     xmm10,xmm6 ; por     xmm11,xmm7 ; por     xmm12,xmm8 ;"
"por     xmm11,xmm6 ; por     xmm12,xmm7 ;"  // por xmm13,xmm8 should be done later because xmm13 is still needed.
// Find cores horizontally, the right half.
"movdqa  xmm6, xmm4 ; movdqa  xmm7, xmm5 ;"
"pcmpeqb xmm6, xmm3 ; pcmpeqb xmm7, xmm4 ;"
"pand    xmm6, xmm14; pand    xmm7, xmm15;"
"paddusb xmm6, xmm13; paddusb xmm7, xmm14;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ;"  // This also makes sure xmm{67} != ColorPuyo.
"por     xmm13,xmm6 ; por     xmm14,xmm7 ;"
"por     xmm14,xmm6 ; por     xmm15,xmm7 ;"
"por     xmm13,xmm8 ;"

//------------------------------------------------------------------------------
// FindCoresVertically [c]
// [ASSUMES] xmm{678}[0] != ColorPuyo.  Because they must be different from 12 dan-me.
// [ASSUMES] xmm9[every] == 0b10000000.
//------------------------------------------------------------------------------
// Find cores vertically, the left sixth.
"palignr xmm8, xmm10,1;"
"palignr xmm6, xmm0, 1;"
"paddusb xmm8, xmm10  ;"
"pcmpeqb xmm6, xmm0   ;"
"pand    xmm10,xmm9   ;"
"pand    xmm8, xmm9   ;"
"por     xmm0, xmm10  ;"
"pand    xmm6, xmm8   ;"
"por     xmm0, xmm6   ;"
"pslldq  xmm6, 1      ;"
"por     xmm0, xmm6   ;"
// Find cores vertically, the center third.
"palignr xmm8, xmm11,1; palignr xmm10,xmm12,1;"  // Note xmm10 is safe to use.
"palignr xmm6, xmm1, 1; palignr xmm7, xmm2, 1;"
"paddusb xmm8, xmm11  ; paddusb xmm10,xmm12  ;"
"pcmpeqb xmm6, xmm1   ; pcmpeqb xmm7, xmm2   ;"
"pand    xmm11,xmm9   ; pand    xmm12,xmm9   ;"
"pand    xmm8, xmm9   ; pand    xmm10,xmm9   ;"
"por     xmm1, xmm11  ; por     xmm2, xmm12  ;"
"pand    xmm6, xmm8   ; pand    xmm7, xmm10  ;"
"por     xmm1, xmm6   ; por     xmm2, xmm7   ;"
"pslldq  xmm6, 1      ; pslldq  xmm7, 1      ;"
"por     xmm1, xmm6   ; por     xmm2, xmm7   ;"
// Find cores vertically, the right half.
"palignr xmm10,xmm13,1; palignr xmm11,xmm14,1; palignr xmm12,xmm15,1;"  // Note xmm{10-12} are safe to use.
"palignr xmm6, xmm3, 1; palignr xmm7, xmm4, 1; palignr xmm8, xmm5, 1;"
"paddusb xmm10,xmm13  ; paddusb xmm11,xmm14  ; paddusb xmm12,xmm15  ;"
"pcmpeqb xmm6, xmm3   ; pcmpeqb xmm7, xmm4   ; pcmpeqb xmm8, xmm5   ;"
"pand    xmm13,xmm9   ; pand    xmm14,xmm9   ; pand    xmm15,xmm9   ;"  // This also assures xmm{13-15} are 0 or 0x80.
"pand    xmm10,xmm9   ; pand    xmm11,xmm9   ; pand    xmm12,xmm9   ;"  // This also assures xmm{10-12} are 0 or 0x80.
"por     xmm3, xmm13  ; por     xmm4, xmm14  ; por     xmm5, xmm15  ;"
"pand    xmm6, xmm10  ; pand    xmm7, xmm11  ; pand    xmm8, xmm12  ;"
"por     xmm3, xmm6   ; por     xmm4, xmm7   ; por     xmm5, xmm8   ;"
"pslldq  xmm6, 1      ; pslldq  xmm7, 1      ; pslldq  xmm8, 1      ;"
"por     xmm3, xmm6   ; por     xmm4, xmm7   ; por     xmm5, xmm8   ;"

//------------------------------------------------------------------------------
// Erase
// [ASSUMES] xmm{10-15}[0] not in (0x80 + ColorPuyo).
// [ASSUMES] xmm9[every] == 0b10000000.
//------------------------------------------------------------------------------
// Erase horizontally.
"movdqa  xmm11,xmm1; movdqa  xmm12,xmm2; movdqa  xmm13,xmm3; movdqa  xmm14,xmm4; movdqa  xmm15,xmm5;"
"pxor    xmm11,xmm0; pxor    xmm12,xmm1; pxor    xmm13,xmm2; pxor    xmm14,xmm3; pxor    xmm15,xmm4;"
"pcmpeqb xmm11,xmm9; pcmpeqb xmm12,xmm9; pcmpeqb xmm13,xmm9; pcmpeqb xmm14,xmm9; pcmpeqb xmm15,xmm9;"
"pand    xmm11,xmm9; pand    xmm12,xmm9; pand    xmm13,xmm9; pand    xmm14,xmm9; pand    xmm15,xmm9;"
"por     xmm0,xmm11; por     xmm1,xmm12; por     xmm2,xmm13; por     xmm3,xmm14; por     xmm4,xmm15;"
"por     xmm1,xmm11; por     xmm2,xmm12; por     xmm3,xmm13; por     xmm4,xmm14; por     xmm5,xmm15;"
// Erase vertically.
"palignr xmm10, xmm0, 1; palignr xmm11, xmm1, 1; palignr xmm12, xmm2, 1;"
"palignr xmm13, xmm3, 1; palignr xmm14, xmm4, 1; palignr xmm15, xmm5, 1;"
"pxor    xmm10,xmm0; pxor   xmm11,xmm1; pxor   xmm12,xmm2; pxor   xmm13,xmm3; pxor   xmm14,xmm4; pxor   xmm15,xmm5;"
"pcmpeqb xmm10,xmm9;pcmpeqb xmm11,xmm9;pcmpeqb xmm12,xmm9;pcmpeqb xmm13,xmm9;pcmpeqb xmm14,xmm9;pcmpeqb xmm15,xmm9;"
"pand    xmm10,xmm9; pand   xmm11,xmm9; pand   xmm12,xmm9; pand   xmm13,xmm9; pand   xmm14,xmm9; pand   xmm15,xmm9;"
"por     xmm0,xmm10; por    xmm1,xmm11; por    xmm2,xmm12; por    xmm3,xmm13; por    xmm4,xmm14; por    xmm5,xmm15;"
"pslldq  xmm10,1   ; pslldq xmm11,1   ; pslldq xmm12,1   ; pslldq xmm13,1   ; pslldq xmm14,1   ; pslldq xmm15,1   ;"
"por     xmm0,xmm10; por    xmm1,xmm11; por    xmm2,xmm12; por    xmm3,xmm13; por    xmm4,xmm14; por    xmm5,xmm15;"

//------------------------------------------------------------------------------
// GenerateMasks [g]
// [ASSUMES] xmm9[every] == 0b10000000.
//------------------------------------------------------------------------------
"movdqa  xmm8, xmm9;"  // threshold(xmm8) <-- 10000000  [g]
"pcmpeqb xmm7, xmm7;"  // minus    (xmm7) <-- 11111111  [g]
"movdqa  xmm6, xmm9;"  // threshold(xmm6) <-- 10000000  [g]
"psrlw   xmm8, 6   ;"  // threshold(xmm8) <-- 00000010  [g]
"pxor    xmm8, xmm9;"  // threshold(xmm8) <-- 10000010  [g]
"psrlw   xmm6, 7   ;"  // threshold(xmm6) <-- 00000001  [g]

//------------------------------------------------------------------------------
// PreserveOjamaAndEmpty
// [ASSUMES] xmm9[every] == 0b10000000.
// [ASSUMES] xmm8[every] == 0b10000010.
//------------------------------------------------------------------------------
"movdqa  xmm10,xmm8; movdqa xmm11,xmm8; movdqa xmm12,xmm8; movdqa xmm13,xmm8; movdqa xmm14,xmm8; movdqa xmm15,xmm8;"
"pcmpgtb xmm10,xmm0;pcmpgtb xmm11,xmm1;pcmpgtb xmm12,xmm2;pcmpgtb xmm13,xmm3;pcmpgtb xmm14,xmm4;pcmpgtb xmm15,xmm5;"
"pand    xmm10,xmm9; pand   xmm11,xmm9; pand   xmm12,xmm9; pand   xmm13,xmm9; pand   xmm14,xmm9; pand   xmm15,xmm9;"
"pxor    xmm0,xmm10; pxor   xmm1,xmm11; pxor   xmm2,xmm12; pxor   xmm3,xmm13; pxor   xmm4,xmm14; pxor   xmm5,xmm15;"

//------------------------------------------------------------------------------
// CountVanishingPuyo
// [ASSUMES] eax or ecx are not touched anywhere else.
// [ASSUMES] xmm6[every] == 0b00000001.
//------------------------------------------------------------------------------
"pxor   xmm10,xmm10; pxor  xmm11,xmm11; pxor  xmm12,xmm12; pxor  xmm13,xmm13; pxor  xmm14,xmm14; pxor  xmm15,xmm15;"
"pcmpgtb xmm10,xmm0;pcmpgtb xmm11,xmm1;pcmpgtb xmm12,xmm2;pcmpgtb xmm13,xmm3;pcmpgtb xmm14,xmm4;pcmpgtb xmm15,xmm5;"
"pand    xmm10,xmm6; pand   xmm11,xmm6; pand   xmm12,xmm6; pand   xmm13,xmm6; pand   xmm14,xmm6; pand   xmm15,xmm6;"
"paddusb xmm10,xmm11; paddusb xmm12,xmm13; paddusb xmm14,xmm15;"
"paddusb xmm10,xmm12; paddusb xmm10,xmm14;"
"pxor    xmm11,xmm11;"
"psadbw  xmm11,xmm10;"
"palignr xmm12,xmm11,8;"
"paddusb xmm12,xmm11;"
"movd    %0,   xmm12;"
"jecxz   loopend;"
"test    eax,  eax  ;"
"jnz     vanish;"
"loopend:"
"movdqa  XMMWORD PTR [%2     ], xmm0;"
"movdqa  XMMWORD PTR [%2 + 16], xmm1;"
"movdqa  XMMWORD PTR [%2 + 32], xmm2;"
"movdqa  XMMWORD PTR [%2 + 48], xmm3;"
"movdqa  XMMWORD PTR [%2 + 64], xmm4;"
"movdqa  XMMWORD PTR [%2 + 80], xmm5;"
"ret;"
"vanish:"
"sub     ecx,  1    ;"

//------------------------------------------------------------------------------
// ReturnWhenSkippingDrop
// [ASSUMES] eax or ecx are not touched anywhere else.
//------------------------------------------------------------------------------

//"sub ecx, 1;"
//"movd    %1,   xmm0;"
//"jecxz   vanish; ret;"
//"vanish:"


//------------------------------------------------------------------------------
// Ojama
// [ASSUMES] xmm9[every] == 0b10000000.
// [ASSUMES] xmm8[every] == 0b10000010.
// [ASSUMES] xmm7[every] == 0b11111111.
// [TODO] Find out a way to use palign for vertical erase.
// [TODO] Find out a better way to avoid "(0x81 xor ColorPuyo) & 0x81" case.
//------------------------------------------------------------------------------
"paddsb  xmm8, xmm7;"  // ojama(xmm8) <-- 10000001
// Calculate vanishing OjamaPuyo horizontally.
"movdqa  xmm11,xmm1 ; movdqa  xmm12,xmm2 ; movdqa  xmm13,xmm3 ; movdqa  xmm14,xmm4 ; movdqa  xmm15,xmm5 ;"
"pxor    xmm11,xmm0 ; pxor    xmm12,xmm1 ; pxor    xmm13,xmm2 ; pxor    xmm14,xmm3 ; pxor    xmm15,xmm4 ;"
"pand    xmm11,xmm8 ; pand    xmm12,xmm8 ; pand    xmm13,xmm8 ; pand    xmm14,xmm8 ; pand    xmm15,xmm8 ;"
"pcmpeqb xmm11,xmm8 ; pcmpeqb xmm12,xmm8 ; pcmpeqb xmm13,xmm8 ; pcmpeqb xmm14,xmm8 ; pcmpeqb xmm15,xmm8 ;"
"pand    xmm11,xmm9 ; pand    xmm12,xmm9 ; pand    xmm13,xmm9 ; pand    xmm14,xmm9 ; pand    xmm15,xmm9 ;"
// Erase OjamaPuyo vertically, the left half.
"movdqa  xmm6, xmm0 ; movdqa  xmm7, xmm1 ; movdqa  xmm10,xmm2;"
"psrldq  xmm6, 1    ; psrldq  xmm7, 1    ; psrldq  xmm10,1   ;"
"pxor    xmm6, xmm0 ; pxor    xmm7, xmm1 ; pxor    xmm10,xmm2;"
"pand    xmm6, xmm8 ; pand    xmm7, xmm8 ; pand    xmm10,xmm8;"
"pcmpeqb xmm6, xmm8 ; pcmpeqb xmm7, xmm8 ; pcmpeqb xmm10,xmm8;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ; pand    xmm10,xmm9;"
"por     xmm0, xmm6 ; por     xmm1, xmm7 ; por     xmm2,xmm10;"
"pslldq  xmm6, 1    ; pslldq  xmm7, 1    ; pslldq  xmm10,1   ;"
"por     xmm0, xmm6 ; por     xmm1, xmm7 ; por     xmm2,xmm10;"
// Erase OjamaPuyo vertically, the right half.
"movdqa  xmm6, xmm3 ; movdqa  xmm7, xmm4 ; movdqa  xmm10,xmm5;"
"psrldq  xmm6, 1    ; psrldq  xmm7, 1    ; psrldq  xmm10,1   ;"
"pxor    xmm6, xmm3 ; pxor    xmm7, xmm4 ; pxor    xmm10,xmm5;"
"pand    xmm6, xmm8 ; pand    xmm7, xmm8 ; pand    xmm10,xmm8;"
"pcmpeqb xmm6, xmm8 ; pcmpeqb xmm7, xmm8 ; pcmpeqb xmm10,xmm8;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ; pand    xmm10,xmm9;"
"por     xmm3, xmm6 ; por     xmm4, xmm7 ; por     xmm5,xmm10;"
"pslldq  xmm6, 1    ; pslldq  xmm7, 1    ; pslldq  xmm10,1   ;"  // This also makes sure xmm{67} != ColorPuyo.
"por     xmm3, xmm6 ; por     xmm4, xmm7 ; por     xmm5,xmm10;"
// Erase OjamaPuyo horizontally.
"por     xmm0, xmm11; por     xmm1, xmm12; por     xmm2, xmm13; por     xmm3, xmm14; por     xmm4, xmm15;"
"por     xmm1, xmm11; por     xmm2, xmm12; por     xmm3, xmm13; por     xmm4, xmm14; por     xmm5, xmm15;"

//------------------------------------------------------------------------------
// Drop
// [ASSUMES] xmm9[every] == 0b10000000.
//------------------------------------------------------------------------------
// Rotate field, initialize masks, and masks vanished cells.
"pcmpeqb xmm7, xmm7;"  // mask(xmm7) <- 11111111
"psrlw   xmm9, 3   ;"  // bit (xmm9) <- 00010000
"pmaxsb  xmm0, xmm7   ; pmaxsb  xmm1, xmm7   ; pmaxsb  xmm2, xmm7   ;"
"pmaxsb  xmm3, xmm7   ; pmaxsb  xmm4, xmm7   ; pmaxsb  xmm5, xmm7   ;"
"palignr xmm0, xmm0, 4; palignr xmm1, xmm1, 4; palignr xmm2, xmm2, 4;"
"palignr xmm3, xmm3, 4; palignr xmm4, xmm4, 4; palignr xmm5, xmm5, 4;"
"paddsb  xmm7, xmm9;"  // mask(xmm7) <- 00001111
// Calculate the number of vanished cells under each cell.
"movdqa  xmm10,xmm0;movdqa  xmm11,xmm1;movdqa  xmm12,xmm2;movdqa  xmm13,xmm3;movdqa  xmm14,xmm4;movdqa  xmm15,xmm5;"
"pand    xmm10,xmm9;pand    xmm11,xmm9;pand    xmm12,xmm9;pand    xmm13,xmm9;pand    xmm14,xmm9;pand    xmm15,xmm9;"
// Iterate 12 times.
// TODO(hiroshimizuno): Use 1+2+4+8.
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
"pslldq  xmm10,1   ;pslldq  xmm11,1   ;pslldq  xmm12,1   ;pslldq  xmm13,1   ;pslldq  xmm14,1   ;pslldq  xmm15,1   ;"
"paddusb xmm0,xmm10;paddusb xmm1,xmm11;paddusb xmm2,xmm12;paddusb xmm3,xmm13;paddusb xmm4,xmm14;paddusb xmm5,xmm15;"
// Calculate the dropped field.
"movdqa  xmm10,xmm0;movdqa  xmm11,xmm1;movdqa  xmm12,xmm2;movdqa  xmm13,xmm3;movdqa  xmm14,xmm4;movdqa  xmm15,xmm5;"
// Iterate 12 times.
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
"psrldq  xmm10,1   ;psrldq  xmm11,1   ;psrldq  xmm12,1   ;psrldq  xmm13,1   ;psrldq  xmm14,1   ;psrldq  xmm15,1   ;"
"psubb   xmm10,xmm9;psubb   xmm11,xmm9;psubb   xmm12,xmm9;psubb   xmm13,xmm9;psubb   xmm14,xmm9;psubb   xmm15,xmm9;"
"pminub  xmm0,xmm10;pminub  xmm1,xmm11;pminub  xmm2,xmm12;pminub  xmm3,xmm13;pminub  xmm4,xmm14;pminub  xmm5,xmm15;"
// Rotate field back to the original format.
"movdqa  xmm10,xmm7;movdqa  xmm11,xmm7;movdqa  xmm12,xmm7;movdqa  xmm13,xmm7;movdqa  xmm14,xmm7;movdqa  xmm15,xmm7;"
"pcmpgtb xmm10,xmm0;pcmpgtb xmm11,xmm1;pcmpgtb xmm12,xmm2;pcmpgtb xmm13,xmm3;pcmpgtb xmm14,xmm4;pcmpgtb xmm15,xmm7;"
"pand    xmm0,xmm10;pand    xmm1,xmm11;pand    xmm2,xmm12;pand    xmm3,xmm13;pand    xmm4,xmm14;pand    xmm5,xmm15;"
"palignr xmm0, xmm0,12; palignr xmm1, xmm1,12; palignr xmm2, xmm2,12;"
"palignr xmm3, xmm3,12; palignr xmm4, xmm4,12; palignr xmm5, xmm5,12;"

//------------------------------------------------------------------------------
// Repeat
// [ASSUMES] xmm{678}[0] != ColorPuyo.  In order to use palign instead of movdqa and shrldq.
// [ASSUMES] xmm9[every] == 0b10000000.
//------------------------------------------------------------------------------
"psllw   xmm9, 3   ;"  // bit (xmm9) <- 10000000
"jmp start;"
  : "=a" (vanished), "+c" (steps)
  : "r" (field)
  : "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11",
    "xmm12", "xmm13", "xmm14", "xmm15",
    "memory", "cc"
  );
  return vanished;
}
