//  ====================================================
//  C言語:簡易グラフィックライブラリ dummy
//  ====================================================
#ifdef _LINUX_
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

#include <stdio.h>
#include <ctype.h>

char f_hid_libusb = 0;

//  ====================================================
//  ウィンドウタイトル文字列を設定する.
//  ====================================================
int gr_settitle(char *name)
{
    return 0;
}


//  ====================================================
//  (x,y)座標に点(color)を打つ.
//  ====================================================
void gr_pset(int x, int y, int color)
{
}

//  ====================================================
//  線分を描画する.
//  ====================================================
void gr_line(int x1, int y1, int x2, int y2, int c)
{
}

//  ====================================================
//  水平ライン(x1,y1) - (x2,y2) を引く (y2は無視)
//  ====================================================
void gr_vline(int x1, int y1, int x2, int y2, int c)
{
}

//  ====================================================
//  BOXFILL(x1,y1) - (x2,y2),c
//  ====================================================
void gr_boxfill(int x1, int y1, int x2, int y2, int c)
{
}


//  ====================================================
//  BOX(x1,y1) - (x2,y2),c
//  ====================================================
void gr_box(int x1, int y1, int x2, int y2, int c)
{
}

//  ====================================================
//  描画領域を色(color)で全クリア.
//  ====================================================
void gr_cls(int color)
{
}

//  ====================================================
//  (x,y)座標に色(color)で文字列(*s)を描画する.
//  ====================================================
void gr_puts(int x, int y, char *s, int color)
{
}

//  ====================================================
//
//  ====================================================
void gr_textout(int x, int y, char *s)
{
}

//  ====================================================
//  画面フリップ.
//  ====================================================
int gr_flip(int flag)
{
    return 0;
}

//  ====================================================
//  再描画を促すと同時に、終了チェックの値を返す.
//  ====================================================
int gr_break(void)
{
    return 0;
}

//  ====================================================
//
//  ====================================================
int hitanykey(void)
{
    return 0;
}


//  ====================================================
//  グラフィック終了処理.
//  ====================================================
void gr_close(void)
{
}


//  ====================================================
//  グラフィック初期化.
//  ====================================================
void gr_init(int width, int height, int bpp, int color)
{
}



//  ====================================================
//  MS-DOSにあって、Linuxに無い?ライブラリ
//  ====================================================

#if 0
//  微妙に仕様が違いますが・・・.
int stricmp(const char *s, const char *t)
{
    while (*s) {
        if (toupper(*s) != toupper(*t)) {
            return 1;
        }
        s++;
        t++;
    }
    if (*t == 0)
        return 0;
    else
        return 1;
}
#endif

//  大文字化?
char *strupr(char *s)
{
    char *s0 = s;

    while (*s) {
        *s = toupper(*s);
        s++;
    }
    return s0;
}

int hidasp_list(char *string)
{
    fprintf(stderr, 
    	"hidasp_list not support. Use 'usbtool list'.\n"
    );
    return -1;
}

int set_term(int mode);
int test_until_kbhit(void);

int set_term(int mode)
{                               /* mode=0: init, 1:=restore */
    static struct termios ttyd_save;
    struct termios ttyd;

    if (mode == 0) {
        if (tcgetattr(0, &ttyd) == -1) {
            perror("tcgetattr");
            return (-1);
        }
        //memcpy(&ttyd_save, &ttyd, sizeof(ttyd));
        ttyd_save = ttyd;
        /* set input mode: no_signal at break, no strip */
        ttyd.c_iflag &= ~(BRKINT | ISTRIP | IXON);
        /* set local mode: disable Erase & Kill, no Special controls */
        ttyd.c_lflag &= ~(ICANON | IEXTEN | ECHO | ECHOE | ECHOK | ECHONL); //ISIG
        ttyd.c_cc[VMIN] = 1;    /* min. charcter */
        ttyd.c_cc[VTIME] = 0;   /* timout, 0.1 sec unit */
        if (tcsetattr(0, TCSANOW, &ttyd) == -1) {
            perror("tcsetattr");
            return (-1);
        }
        return (0);
    } else {                    /* mode=1: restore termio */
        if (tcsetattr(0, TCSANOW, &ttyd_save) == -1) {
            perror("tcsetattr");
            return (-1);
        }
        return (0);
    }
}


/* kbhit: return number of char(s) in stdin at this moment.
   application may do n times getchar to clean stdin buffer
*/
int kbhit(void)
{
    int n, rv;
    rv = ioctl(0, FIONREAD, &n);
    if (rv == -1 || n == 0)
        return (0);
    else
        return (n);
}

/* until_kbhit is like  !kbhit().
   but, this is useful for while(until_kbhit) application
   cuurently, until_kbhit processing time is much fast than kbhit.
   untill_kbhit's process_time is hoped as short as possible.
*/
int until_kbhit(void)
{
    int n, rv;
    rv = ioctl(0, FIONREAD, &n);
    if (rv == -1 || n == 0)
        return (1);
    else
        return (0);
}

int _sleep(int ms)
{
    usleep(ms * 1000);
    return 0;
}
