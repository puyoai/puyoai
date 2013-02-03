//  ====================================================
//  Ｃ言語:簡易グラフィックライブラリ(Win32版)
//  ====================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef	_DOS_
#include <windows.h>
#endif
//  ====================================================
//  定義.
//  ====================================================
//#define   RGB_(r,g,b) (((b)>>3)|(((g)>>3)<<5)|(((r)>>3)<<10)) // 16bppのときの色.

//  ====================================================
//  インターフェース.
//  ====================================================
void gr_init(int width, int height, int bpp, int color);    //初期化.
void gr_exit(int rc);           //終了.

void gr_cls(int color);         //画面クリア.
void gr_pset(int x, int y, int color);  //ドット打ち.
int *gr_point(int x, int y);
void gr_line(int x0, int y0, int x1, int y1, int color);    //線引き.
void gr_hline(int x0, int y0, int x1, int y1, int color);   //線引き.
void gr_vline(int x0, int y0, int x1, int y1, int color);   //線引き.
void gr_box(int x0, int y0, int width, int height, int color);  //箱(枠のみ).
void gr_boxfill(int x0, int y0, int width, int height, int color);  //箱(内部塗りつぶし).
void gr_puts(int x, int y, char *s, int color); // 文字列描画.

int gr_flip(int flag);          //描画完了処理.
void gr_close(void);            //窓close
int gr_break(void);             //終了チェック.
int hitanykey(void);            //キー入力チェック.

#if	0                           //未実装.

#define	FLIP_NOWAIT	0
#define	FLIP_WAIT	1

typedef struct {
    int id;
    int width;
    int height;
    //

    //
} gr_bitmap;

gr_bitmap *gr_loadbmp(char *filename);  //bmpファイルの読み込み.
void gr_putbmp(int x, int y, gr_bitmap * bitmap);   //bmpファイルの表示.
void gr_putsprite(int x, int y, gr_bitmap * bitmap);    //透明処理付きの表示.
#endif

//  ====================================================
