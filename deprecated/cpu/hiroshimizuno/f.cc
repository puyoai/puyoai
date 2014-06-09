#include <iostream>
#include <sys/time.h>

inline void Step(unsigned char* field) {
  __asm__ __volatile__(
//------------------------------------------------------------------------------
// Initialize [i]
//------------------------------------------------------------------------------
"pcmpeqb xmm6, xmm6;"  // xmm6(noncolor) <- 11111111
"pxor    xmm7, xmm7;"  // xmm7(noncolor) <- 00000000
"pxor    xmm8, xmm8;"  // xmm8(noncolor) <- 00000000
"pabsb   xmm9, xmm6;"  // xmm9(bit)      <- 00000001
"movdqa  xmm0, XMMWORD PTR [%0     ];"
"movdqa  xmm1, XMMWORD PTR [%0 + 16];"
"movdqa  xmm2, XMMWORD PTR [%0 + 32];"
"movdqa  xmm3, XMMWORD PTR [%0 + 48];"
"movdqa  xmm4, XMMWORD PTR [%0 + 64];"
"movdqa  xmm5, XMMWORD PTR [%0 + 80];"
"psllw   xmm9, 7;"     // xmm9(bit)      <- 10000000

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
"                     movdqa  xmm7, xmm1 ; movdqa  xmm8, xmm2 ;"
"                     pcmpeqb xmm7, xmm0 ; pcmpeqb xmm8, xmm1 ;"
"                     pand    xmm7, xmm11; pand    xmm8, xmm12;"
"                     paddusb xmm7, xmm10; paddusb xmm8, xmm11;"
"                     pand    xmm7, xmm9 ; pand    xmm8, xmm9 ;"
"                     por     xmm10,xmm7 ; por     xmm11,xmm8 ;"
"                     por     xmm11,xmm7 ; por     xmm12,xmm8 ;"
// Find cores horizontally, the right half.
"movdqa  xmm6, xmm3 ; movdqa  xmm7, xmm4 ; movdqa  xmm8, xmm5 ;"
"pcmpeqb xmm6, xmm2 ; pcmpeqb xmm7, xmm3 ; pcmpeqb xmm8, xmm4 ;"
"pand    xmm6, xmm13; pand    xmm7, xmm14; pand    xmm8, xmm15;"
"paddusb xmm6, xmm12; paddusb xmm7, xmm13; paddusb xmm8, xmm14;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ; pand    xmm8, xmm9 ;"  // This also makes sure xmm{678} != ColorPuyo.
"por     xmm12,xmm6 ; por     xmm13,xmm7 ; por     xmm14,xmm8 ;"
"por     xmm13,xmm6 ; por     xmm14,xmm7 ; por     xmm15,xmm8 ;"

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
"psrlw   xmm8, 6   ;"  // threshold(xmm8) <-- 00000010  [g]
"pxor    xmm8, xmm9;"  // threshold(xmm8) <-- 10000010  [g]

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
"pcmpeqb xmm6, xmm8 ; pcmpeqb xmm7, xmm8 ; pcmpeqb xmm10,xmm8;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ; pand    xmm10,xmm9;"
"por     xmm0, xmm6 ; por     xmm1, xmm7 ; por     xmm2,xmm10;"
"pslldq  xmm6, 1    ; pslldq  xmm7, 1    ; pslldq  xmm10,1   ;"
"por     xmm0, xmm6 ; por     xmm1, xmm7 ; por     xmm2,xmm10;"
// Erase OjamaPuyo vertically, the right half.
"movdqa  xmm6, xmm3 ; movdqa  xmm7, xmm4 ; movdqa  xmm10,xmm5;"
"psrldq  xmm6, 1    ; psrldq  xmm7, 1    ; psrldq  xmm10,1   ;"
"pxor    xmm6, xmm3 ; pxor    xmm7, xmm4 ; pxor    xmm10,xmm5;"
"pcmpeqb xmm6, xmm8 ; pcmpeqb xmm7, xmm8 ; pcmpeqb xmm10,xmm8;"
"pand    xmm6, xmm9 ; pand    xmm7, xmm9 ; pand    xmm10,xmm9;"
"por     xmm3, xmm6 ; por     xmm4, xmm7 ; por     xmm5,xmm10;"
"pslldq  xmm6, 1    ; pslldq  xmm7, 1    ; pslldq  xmm10,1   ;"
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
// ReturnResults
//------------------------------------------------------------------------------
"movdqa  XMMWORD PTR [%0     ], xmm0;"
"movdqa  XMMWORD PTR [%0 + 16], xmm1;"
"movdqa  XMMWORD PTR [%0 + 32], xmm2;"
"movdqa  XMMWORD PTR [%0 + 48], xmm3;"
"movdqa  XMMWORD PTR [%0 + 64], xmm4;"
"movdqa  XMMWORD PTR [%0 + 80], xmm5;"
  : "+r" (field)
  :
  : "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11",
    "xmm12", "xmm13", "xmm14", "xmm15"
  );
}

void Ojama(unsigned char* field) {
  __asm__ __volatile__(
  "pcmpeqb xmm7, xmm7;"
  "pxor    xmm8, xmm8;"
  "psubb   xmm8, xmm7;"
  "psllw   xmm8, 3;"
  "movdqa  xmm9, xmm8;"
  "psllw   xmm8, 4;"
  "por     xmm9, xmm8;"
  "movdqa  xmm0, XMMWORD PTR [%0];"
  "movdqa  xmm1, XMMWORD PTR [%0 + 16];"
  "movdqa  xmm2, xmm1;"  // 5 para.
  "pxor    xmm2, xmm0;"
  "pand    xmm2, xmm9;"
  "pcmpeqb xmm2, xmm9;"
  "pand    xmm2, xmm8;"
  "por     xmm0, xmm2;"
  "por     xmm1, xmm2;"
  "palignr xmm2, xmm0, 1;"  // heighest byte is unknown.
  "pxor    xmm2, xmm0;"
  "pand    xmm2, xmm9;"
  "pcmpeqb xmm2, xmm9;"
  "pand    xmm2, xmm8;"
  "por     xmm0, xmm2;"
  "pslldq  xmm2, 1;"
  "por     xmm0, xmm2;"
  "palignr xmm2, xmm1, 1;"  // heighest byte is unknown.
  "pxor    xmm2, xmm1;"
  "pand    xmm2, xmm9;"
  "pcmpeqb xmm2, xmm9;"
  "pand    xmm2, xmm8;"
  "movdqa  XMMWORD PTR [%0 + 64], xmm2;"
  "por     xmm1, xmm2;"
  "pslldq  xmm2, 1;"
  "por     xmm1, xmm2;"
  "movdqa  XMMWORD PTR [%0 + 32], xmm0;"
  "psllw   xmm3, 4;"
  "movdqa  XMMWORD PTR [%0 + 48], xmm1;"
  "pand    xmm3, xmm0;"
  : "+r" (field)
  :
  : "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11",
    "xmm12", "xmm13", "xmm14", "xmm15"
  );
}

void Rotate(unsigned char* column) {
  __asm__ __volatile__(
  "movdqa  xmm0, XMMWORD PTR [%0 + 16];"
  "movdqa  xmm1, xmm0;"
  "palignr xmm1, xmm1, 1;"
  "psrldq  xmm0, 1;"
  "movdqa  XMMWORD PTR [%0 + 32], xmm0;"
  "movdqa  XMMWORD PTR [%0 + 48], xmm1;"
  : "+r" (column)
  :
  : "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11",
    "xmm12", "xmm13", "xmm14", "xmm15"
  );
}

void Drop(unsigned char* column) {
  __asm__ __volatile__(
  // TODO(hiroshimizuno): Set 0xFF to empty cells at first, or mask all cells with 0x07 at the end.
  "movdqa  xmm0, XMMWORD PTR [%0];"
  "pxor    xmm1, xmm1;"
  "pcmpeqb xmm9, xmm9;"  // xmm9 minus-shared
  "psubb   xmm1, xmm9;"
  "pmaxsb  xmm0, xmm9;"
  "psllw   xmm1, 4;"     // xmm1 mask-shared
  "movdqa  xmm3, xmm1;"  // xmm3 vanish
  "pand    xmm3, xmm0;"
  "movdqa  XMMWORD PTR [%0 + 16], xmm0;"
  "pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3;"
  "pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3;"
  "pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3;"
  "pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3; pslldq xmm3,1; paddusb xmm0,xmm3;"
  "movdqa  XMMWORD PTR [%0 + 32], xmm0;"
  "movdqa xmm3,xmm0;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "psrldq xmm3,1; psubb xmm3,xmm1; pminub xmm0,xmm3;"
  "movdqa  XMMWORD PTR [%0 + 64], xmm3;"
  "movdqa  XMMWORD PTR [%0 + 80], xmm0;"
  : "+r" (column)
  :
  : "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11",
    "xmm12", "xmm13", "xmm14", "xmm15"
  );
}

void Simulate(unsigned char* field) {
  __asm__ __volatile__(

  // Initialize registers. [i]
  "pcmpeqb xmm7, xmm7;" // minus (xmm7) <- 11111111
  "pxor    xmm9, xmm9;" // three (xmm9) <- 00000000
  "movdqa  xmm0, XMMWORD PTR [%0     ];"
  "movdqa  xmm1, XMMWORD PTR [%0 + 16];"
  "movdqa  xmm2, XMMWORD PTR [%0 + 32];"
  "movdqa  xmm3, XMMWORD PTR [%0 + 48];"
  "movdqa  xmm4, XMMWORD PTR [%0 + 64];"
  "movdqa  xmm5, XMMWORD PTR [%0 + 80];"
  /* [i] */ "psubb   xmm9, xmm7;"  // three (xmm9) <- 00000001  // free minus (xmm7)
  /* [i] */ "movdqa  xmm8, xmm9;"  // bit   (xmm8) <- 00000001
  /* [i] */ "psllw   xmm8, 1;"     // bit   (xmm8) <- 00000010
  /* [i] */ "pxor    xmm9, xmm8;"  // three (xmm9) <- 00000011

  // Count horizontal hands. [h]
  "movdqa  xmm11,xmm1; movdqa  xmm12,xmm2; movdqa  xmm13,xmm3; movdqa  xmm14,xmm4; movdqa  xmm15,xmm5;"
  "pcmpeqb xmm11,xmm0; pcmpeqb xmm12,xmm1; pcmpeqb xmm13,xmm2; pcmpeqb xmm14,xmm3; pcmpeqb xmm15,xmm4;"
  "pand    xmm11,xmm9; pand    xmm12,xmm9; pand    xmm13,xmm9; pand    xmm14,xmm9; pand    xmm15,xmm9;"
  /* [h] */ "movdqa  xmm10, xmm11;"
  /* [h] */ "paddusb xmm11, xmm12;"
  /* [h] */ "paddusb xmm12, xmm13;"
  /* [h] */ "paddusb xmm13, xmm14;"
  /* [h] */ "paddusb xmm14, xmm15;"

  // Count vertical hands, the left third.
  "movdqa  xmm6, xmm0; movdqa  xmm7, xmm1;"  // palignr?
  "pslldq  xmm6, 1;    pslldq  xmm7, 1;"     // palignr?
  "pcmpeqb xmm6, xmm0; pcmpeqb xmm7, xmm1;"
  "pand    xmm6, xmm9; pand    xmm7, xmm9;"
  "paddusb xmm10,xmm6; paddusb xmm11,xmm7;"
  "psrldq  xmm6, 1;    psrldq  xmm7, 1;"
  "paddusb xmm10,xmm6; paddusb xmm11,xmm7;"
  // Count vertical hands, the center third.
  "movdqa  xmm6, xmm2; movdqa  xmm7, xmm3;"  // palignr?
  "pslldq  xmm6, 1;    pslldq  xmm7, 1;"     // palignr?
  "pcmpeqb xmm6, xmm2; pcmpeqb xmm7, xmm3;"
  "pand    xmm6, xmm9; pand    xmm7, xmm9;"
  "paddusb xmm12,xmm6; paddusb xmm13,xmm7;"
  "psrldq  xmm6, 1;    psrldq  xmm7, 1;"
  "paddusb xmm12,xmm6; paddusb xmm13,xmm7;"
  // Count vertical hands, the right third.
  "movdqa  xmm6, xmm4; movdqa  xmm7, xmm5;"  // palignr?
  "pslldq  xmm6, 1;    pslldq  xmm7, 1;"     // palignr?
  "pcmpeqb xmm6, xmm4; pcmpeqb xmm7, xmm5;"
  "pand    xmm6, xmm9; pand    xmm7, xmm9;"
  "paddusb xmm14,xmm6; paddusb xmm15,xmm7;"
  "psrldq  xmm6, 1;    psrldq  xmm7, 1;"
  "paddusb xmm14,xmm6; paddusb xmm15,xmm7;"

  // Find cores, horizontally.
  "psllw   xmm8, 1;"  // bit   (xmm8) <- 00000100
  "pxor xmm0,xmm10; pxor xmm1, xmm11; pxor xmm2,xmm12; pxor xmm3,xmm13; pxor xmm4,xmm14; pxor xmm5,xmm15;"
  "movdqa  xmm11,xmm1; movdqa  xmm12,xmm2; movdqa  xmm13,xmm3; movdqa  xmm14,xmm4; movdqa  xmm15,xmm5;"
  "pcmpeqb xmm11,xmm0; pcmpeqb xmm12,xmm1; pcmpeqb xmm13,xmm2; pcmpeqb xmm14,xmm3; pcmpeqb xmm15,xmm4;"
  "pand    xmm11,xmm0; pand    xmm12,xmm1; pand    xmm13,xmm2; pand    xmm14,xmm3; pand    xmm15,xmm4;"
  "pand    xmm11,xmm8; pand    xmm12,xmm8; pand    xmm13,xmm8; pand    xmm14,xmm8; pand    xmm15,xmm8;"
  "psllw   xmm11,1   ; psllw   xmm12,1   ; psllw   xmm13,1   ; psllw   xmm14,1   ; psllw   xmm15,1   ;"
  "por     xmm0,xmm11; por     xmm1,xmm12; por     xmm2,xmm13; por     xmm3,xmm14; por     xmm4,xmm15;"
  "por     xmm1,xmm11; por     xmm2,xmm12; por     xmm3,xmm13; por     xmm4,xmm14; por     xmm5,xmm15;"
  // Find cores, vertically.  // palignr?
  "movdqa  xmm10,xmm0; movdqa xmm11,xmm1; movdqa xmm12,xmm2; movdqa xmm13,xmm3; movdqa xmm14,xmm4; movdqa xmm15,xmm5;"
  "pslldq  xmm10,1   ; pslldq xmm11,1   ; pslldq xmm12,1   ; pslldq xmm13,1   ; pslldq xmm14,1   ; pslldq xmm15,1   ;"
  "pcmpeqb xmm10,xmm0;pcmpeqb xmm11,xmm1;pcmpeqb xmm12,xmm2;pcmpeqb xmm13,xmm3;pcmpeqb xmm14,xmm4;pcmpeqb xmm15,xmm5;"
  "pand    xmm10,xmm0; pand   xmm11,xmm1; pand   xmm12,xmm2; pand   xmm13,xmm3; pand   xmm14,xmm4; pand   xmm15,xmm5;"
  "pand    xmm10,xmm8; pand   xmm11,xmm8; pand   xmm12,xmm8; pand   xmm13,xmm8; pand   xmm14,xmm8; pand   xmm15,xmm8;"
  "psllw   xmm10,1   ; psllw  xmm11,1   ; psllw  xmm12,1   ; psllw  xmm13,1   ; psllw  xmm14,1   ; psllw  xmm15,1   ;"
  "por     xmm0,xmm10; por    xmm1,xmm11; por    xmm2,xmm12; por    xmm3,xmm13; por    xmm4,xmm14; por    xmm5,xmm15;"
  "psrldq  xmm10,1   ; psrldq xmm11,1   ; psrldq xmm12,1   ; psrldq xmm13,1   ; psrldq xmm14,1   ; psrldq xmm15,1   ;"
  "por     xmm0,xmm10; por    xmm1,xmm11; por    xmm2,xmm12; por    xmm3,xmm13; por    xmm4,xmm14; por    xmm5,xmm15;"

  // Reset junk bits.
  "por     xmm9, xmm8;"  // seven     (xmm9) <- 00000111  // free three (xmm9)  // bit (xmm8) == 00000100
  "pcmpeqb xmm7, xmm7;"  // colorcore (xmm7) <- 11111111
  "psllw   xmm8, 1;"     // bit       (xmm8) <- 00001000
  "pxor    xmm7, xmm9;"  // colorcore (xmm7) <- 11111000
  "pand xmm0,xmm7; pand xmm1,xmm7; pand xmm2,xmm7; pand xmm3,xmm7; pand xmm4,xmm7; pand xmm5,xmm7;"
  "psllw   xmm9, 4;"     // seven     (xmm9) <- 01110000

  // Erase horizontally.
  "movdqa  xmm11,xmm1; movdqa  xmm12,xmm2; movdqa  xmm13,xmm3; movdqa  xmm14,xmm4; movdqa  xmm15,xmm5;"
  "pxor    xmm11,xmm0; pxor    xmm12,xmm1; pxor    xmm13,xmm2; pxor    xmm14,xmm3; pxor    xmm15,xmm4;"
  "pcmpeqb xmm11,xmm8; pcmpeqb xmm12,xmm8; pcmpeqb xmm13,xmm8; pcmpeqb xmm14,xmm8; pcmpeqb xmm15,xmm8;"  // xmm8=1000
  "pand    xmm11,xmm8; pand    xmm12,xmm8; pand    xmm13,xmm8; pand    xmm14,xmm8; pand    xmm15,xmm8;"  // xmm8=1000
  "por     xmm0,xmm11; por     xmm1,xmm12; por     xmm2,xmm13; por     xmm3,xmm14; por     xmm4,xmm15;"
  "por     xmm1,xmm11; por     xmm2,xmm12; por     xmm3,xmm13; por     xmm4,xmm14; por     xmm5,xmm15;"
  // Erase vertically.  // palignr?
  "movdqa  xmm10,xmm0; movdqa xmm11,xmm1; movdqa xmm12,xmm2; movdqa xmm13,xmm3; movdqa xmm14,xmm4; movdqa xmm15,xmm5;"
  "pslldq  xmm10,1   ; pslldq xmm11,1   ; pslldq xmm12,1   ; pslldq xmm13,1   ; pslldq xmm14,1   ; pslldq xmm15,1   ;"
  "pxor    xmm10,xmm0; pxor   xmm11,xmm1; pxor   xmm12,xmm2; pxor   xmm13,xmm3; pxor   xmm14,xmm4; pxor   xmm15,xmm5;"
  "pcmpeqb xmm10,xmm8;pcmpeqb xmm11,xmm8;pcmpeqb xmm12,xmm8;pcmpeqb xmm13,xmm8;pcmpeqb xmm14,xmm8;pcmpeqb xmm15,xmm8;"
  "pand    xmm10,xmm8; pand   xmm11,xmm8; pand   xmm12,xmm8; pand   xmm13,xmm8; pand   xmm14,xmm8; pand   xmm15,xmm8;"
  "por     xmm0,xmm10; por    xmm1,xmm11; por    xmm2,xmm12; por    xmm3,xmm13; por    xmm4,xmm14; por    xmm5,xmm15;"
  "psrldq  xmm10,1   ; psrldq xmm11,1   ; psrldq xmm12,1   ; psrldq xmm13,1   ; psrldq xmm14,1   ; psrldq xmm15,1   ;"
  "por     xmm0,xmm10; por    xmm1,xmm11; por    xmm2,xmm12; por    xmm3,xmm13; por    xmm4,xmm14; por    xmm5,xmm15;"

  /*
  // Initialize stairs.
  "pcmpeqb xmm7, xmm7;" // minus  (xmm7) <- 11111111
  "pxor    xmm6, xmm6;" // one    (xmm6) <- 00000000
  "psubb   xmm6, xmm7;" // one    (xmm6) <- 00000001  // free minus (xmm7)
  "pxor    xmm7, xmm7;" // stairs (xmm7) <- 00000000
  "pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6;"
  "pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6;"
  "pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6;"
  "pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6;"
  "pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6; pslldq xmm6,1; paddusb xmm7,xmm6;"
  "pcmpeqb xmm6, xmm6;" // cleaner (xmm6) <- 11111111
  "pxor    xmm6, xmm8;" // cleaner (xmm6) <- 11110111

  // Drop, the left half.
  "movdqa  xmm10,xmm0 ; movdqa  xmm11,xmm1 ; movdqa  xmm12,xmm2 ;"
  "pand    xmm0, xmm6 ; pand    xmm1, xmm6 ; pand    xmm2, xmm6 ;"
  "pand    xmm10,xmm8 ; pand    xmm11,xmm8 ; pand    xmm12,xmm8 ;"
  "psrlw   xmm10,3    ; psrlw   xmm11,3    ; psrlw   xmm12,3    ;"
  "movdqa  xmm13,xmm9 ; movdqa  xmm14,xmm9 ; movdqa  xmm15,xmm9 ;"
  // Repeat 12.5 times.
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  // Shuffle twice.
  "movdqa  xmm10,xmm13; movdqa  xmm11,xmm14; movdqa  xmm12,xmm15;"
  "paddusb xmm10,xmm7 ; paddusb xmm11,xmm7 ; paddusb xmm12,xmm7 ;"
  "pshufb  xmm13,xmm10; pshufb  xmm14,xmm11; pshufb  xmm15,xmm12;"
  "pxor    xmm10,xmm10; pxor    xmm11,xmm11; pxor    xmm12,xmm12;"
  "pcmpeqb xmm10,xmm13; pcmpeqb xmm11,xmm14; pcmpeqb xmm12,xmm15;"
  "por     xmm13,xmm10; por     xmm14,xmm11; por     xmm15,xmm12;"
  "paddusb xmm13,xmm7 ; paddusb xmm14,xmm7 ; paddusb xmm15,xmm7 ;"
  "pshufb  xmm0, xmm13; pshufb  xmm1, xmm14; pshufb  xmm2, xmm15;"
  "movdqa  xmm3,xmm13; movdqa  xmm4,xmm14; movdqa  xmm5,xmm15;"//debug

  // Drop, the right half.
  "movdqa  xmm10,xmm3 ; movdqa  xmm11,xmm4 ; movdqa  xmm12,xmm5 ;"
  "pand    xmm3, xmm6 ; pand    xmm4, xmm6 ; pand    xmm5, xmm6 ;"
  "pand    xmm10,xmm8 ; pand    xmm11,xmm8 ; pand    xmm12,xmm8 ;"
  "psrlw   xmm10,3    ; psrlw   xmm11,3    ; psrlw   xmm12,3    ;"
  "movdqa  xmm13,xmm9 ; movdqa  xmm14,xmm9 ; movdqa  xmm15,xmm9 ;"
  // Repeat 12.5 times.
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  "pslldq  xmm10,1    ; pslldq  xmm11,1    ; pslldq  xmm12,1    ;"
  "paddusb xmm13,xmm10; paddusb xmm14,xmm11; paddusb xmm15,xmm12;"
  // Shuffle twice.
  "movdqa  xmm10,xmm13; movdqa  xmm11,xmm14; movdqa  xmm12,xmm15;"
  "paddusb xmm10,xmm7 ; paddusb xmm11,xmm7 ; paddusb xmm12,xmm7 ;"
  "pshufb  xmm13,xmm10; pshufb  xmm14,xmm11; pshufb  xmm15,xmm12;"
  "pxor    xmm10,xmm10; pxor    xmm11,xmm11; pxor    xmm12,xmm12;"
  "pcmpeqb xmm10,xmm13; pcmpeqb xmm11,xmm14; pcmpeqb xmm12,xmm15;"
  "por     xmm13,xmm10; por     xmm14,xmm11; por     xmm15,xmm12;"
  "paddusb xmm13,xmm7 ; paddusb xmm14,xmm7 ; paddusb xmm15,xmm7 ;"
  "pshufb  xmm3, xmm13; pshufb  xmm4, xmm14; pshufb  xmm5, xmm15;"
  */

  // down, max, cmpeq, up, not, and
  // Drop.
  // [field]   0123456789ABCDEF
  // [vanish]  01xxx56x89xxCDEF
  // [result]  015689CDEF------
  // [reverse] ------015689CDEF
  // [rdiff]   ------6633220000
  // [diff]    0033446666------
  // [support] 0001233344456666
  //
  // [dumb]    0155556889CCCDEF
  //
  // (addu + shl) * 12.5 times
  // [xs]      0001233344456666
  // (shr + cmp0 + adds + cmp0 + and + or) * 12.5 times
  //
  // couter = 0001233344456666;
  // for (int i = 0; i < 12; ++i) {    // INLINE
  //   shifted = field >> (i * 8);
  //   for (int b = 0; b < 16; ++b) {  // SIMD
  //     if (shifted[b] != 0) {
  //       counter[b] -= 1;            // SIGNED
  //     }
  //     if (counter[b] == 0) {        // UNSIGNED
  //       field[b] = shifted[b];
  //     }
  //   }
  // }
  //
  // COUNTER  SHIFTED
  //    6   [6*4 + fF]
  //    6   [6*4 + fE]
  //    6   [6*4 + fD]
  //    6   [6*4 + fC]
  //    5   [0xFF - 0]
  //    4   [0xFF - 0]
  //    4   [4*4 + f9]      [0xFF - 4]    [fF]
  //    4   [4*4 + f8]      [4*4 + f8]    [fE]
  //    3   [0xFF - 0]      [3*4 + f8]    [fD]
  //    3   [3*4 + f6]      [0xFF - 4]    [fC]
  //    3   [3*4 + f5]  **  [2*4 + f6]    [f9]
  //    2   [0xFF - 0]  FF  [2*4 + f5]    [f8]
  //    1   [0xFF - 0]  FF  [0xFF - 4]    [f6]
  //    0   [0xFF - 0]  FF  [0xFF - 4]    [f5]
  //    0   [0*4 + f1] [f1] [0xFF - 4]    [f1]
  //    0   [0*4 + f0] [f0] [0*4 + f1]    [f0]
  // for (int i = 0; i < 12; ++i) {    // INLINE
  //   shifted = field >> (i * 8);
  //   for (int b = 0; b < 16; ++b) {  // SIMD
  //     if (shifted[b] != 0) {
  //       counter[b] -= 1;            // SIGNED
  //     }
  //     if (counter[b] == 0) {        // UNSIGNED
  //       field[b] = shifted[b];
  //     }
  //   }
  // }
  //
  // [result]  015689CDEF------
  // [0]       01xxx56x89xxCDEF
  // [1]       1xxx56x89xxCDEF-
  // [2]       xxx56x89xxCDEF--
  // [3]       xx56x89xxCDEF---
  // [4]       x56x89xxCDEF----
  // [5]       56x89xxCDEF-----
  // [6]       6x89xxCDEF------

  // For debugging.
  /*
  "psrlw   xmm7,  4;"
  "psrlw   xmm8,  4;"
  "psrlw   xmm9,  4;"
  "psrlw   xmm10, 4;"
  "psrlw   xmm11, 4;"
  "psrlw   xmm12, 4;"
  "movdqa  xmm1, xmm7;"
  "movdqa  xmm2, xmm8;"
  "movdqa  xmm3, xmm9;"
  "movdqa  xmm4, xmm10;"
  "movdqa  xmm5, xmm11;"
  "movdqa  xmm6, xmm12;"
  */
  // Updates field.
  "movdqa  XMMWORD PTR [%0     ], xmm0;"
  "movdqa  XMMWORD PTR [%0 + 16], xmm1;"
  "movdqa  XMMWORD PTR [%0 + 32], xmm2;"
  "movdqa  XMMWORD PTR [%0 + 48], xmm3;"
  "movdqa  XMMWORD PTR [%0 + 64], xmm4;"
  "movdqa  XMMWORD PTR [%0 + 80], xmm5;"
  : "+r" (field)
  :
  : "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11",
    "xmm12", "xmm13", "xmm14", "xmm15"
  );
}

void PrintField(unsigned char field[16]) {
  for (int x = 0; x < 6; ++x) {
    for (int y = 0; y < 16; ++y) {
      int p = field[x * 16 + y];
      //if (p == 3 || p == 5) { p = 0; }
      //if (p & 0x08) { p = 7; } else { p >>= 4; }
      /*
      if ((p & 0x70) == 0x70) {
        p &= 0xF;
        std::cout << std::hex << p << std::dec;
      } else {
        p >>= 4;
        std::cout << p;
      }
      */
      if (p < 0x10) { std::cout << "0"; }
      std::cout << std::hex << p << std::dec << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

long long time() {
  struct timeval t;
  gettimeofday(&t, NULL);
  long long l = t.tv_sec;
  l *= 1000 * 1000;
  l += t.tv_usec;
  return l;
}

int main() {
  /*
  unsigned char __attribute__((aligned (16))) field[96] = {
    1, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  unsigned char __attribute__((aligned (16))) field[96] = {
    1, 2, 4, 8, 4, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 4, 8, 8, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 2, 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 4, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 2, 4, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  unsigned char __attribute__((aligned (16))) field[96] = {
    2, 0x83, 0x83, 0x83, 3, 4, 0x82, 1, 2, 0x81, 0x81, 1, 0, 0, 0, 0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  */
  unsigned char __attribute__((aligned (16))) field[96] = {
    6, 0, 0, 0, 2, 2, 2, 8, 8, 4, 4, 8, 8, 6, 6, 6,
    0, 0, 0, 0, 2, 6, 6, 6, 6, 1, 4, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 1, 1, 1, 1, 1, 4, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 8, 8, 1, 8, 8, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  /*
  for (int x = 0; x < 6; ++x) {
    for (int y = 0; y < 16; ++y) {
      if (field[x * 16 + y] == 4) { field[x * 16 + y] = 3; }
    }
  }
  */
  PrintField(field);
  //Rotate(field);
  //Drop(field);
  //Simulate(field);
  long long start = time();
  for (int i = 0; i < 1000; ++i) {
    Step(field);
  }
  long long end = time();
  PrintField(field);
  std::cout << "[TIME] " << end - start << std::endl;
  return 0;
}
