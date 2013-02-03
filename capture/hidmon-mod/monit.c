/*********************************************************************
 *	HID_Monit
 *********************************************************************
 *
 * コマンドヘルプは 'h' もしくは '?' です.

アドレス範囲指定にて、
 ADDRESS < ADDRESS2 を満たさないときは ADDRESS2 をバイト数とみなします.
コマンドやアドレスの区切りは空白もしくは ',' のどちらでも可.

 ADDRESS は16進数、もしくはポート名も可.

 *********************************************************************
 */

#define VERSION "0.2"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if _LINUX_   /* @@@ by senshu */
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "monit.h"
#include "hidasp.h"
#include "util.h"
#include "gr.h"

void cmdPortPrintOne(char *name, int adrs, int val);
#if _LINUX_
extern int kbhit(void);
extern int set_term(int mode);
#endif

#include "portlist.h"
extern PortList portList[];

#define DEFAULT_SERIAL	"0000"

int hidmon_mode = 1;            /* hidmon を実行中 */

const char *HELP_USAGE = {
#if _LINUX_   /* @@@ by senshu */
    "* HID_Monit Ver " VERSION " for Linux (" __DATE__ ")\n"
#else
    "* HID_Monit Ver " VERSION " (" __DATE__ ")\n"
#endif
        "Usage is\n"
        "  monit [Options]\n"
        "Options\n"
        "  -p[:XXXX]   ...  select Serial number (default=0000).\n"
        "  -l          ...  command echo to Log file.\n"
        "  -i<file>    ...  input script file to execute.\n"
        "  -v          ...  print verbose.\n"
};

const char *HELP_MESSAGES = {
    "* HID_Monit Ver " VERSION "\n"
        "Command List\n" " d  <ADDRESS1> <ADDRESS2>    Dump Memory(RAM)\n"
#if 0
        " dr <ADDRESS1> <ADDRESS2>    Dump Memory(EEPROM)\n"
        " dp <ADDRESS1> <ADDRESS2>    Dump Memory(PGMEM)\n"
#endif
    " e  <ADDRESS1> <DATA>        Edit Memory\n"
        " f  <ADDRESS1> <ADDRESS2> <DATA> Fill Memory\n"
        " p ?                         Print PortName-List\n"
        " p .                         Print All Port (column format)\n"
        " p *                         Print All Port (dump format)\n"
        " p <PortName>                Print PortAddress and data\n"
        " p <PortName> <DATA>         Write Data to PortName\n"
        " p <PortName>.<bit> <DATA>   Write Data to PortName.bit\n"
        " sleep <n>                   sleep <n> mSec\n"
        " bench <CNT>                 HID Write Speed Test\n"
        " poll <portName> <CNT>       continuous polling port\n"
        " poll  *  <CNT>              continuous polling port A,B,D\n"
        " graph <portName>            Graphic display\n"
        " q                           Quit to SHELL\n"
};

//
//  コマンド解析用ワーク.
//
#define	MAX_CMDBUF	4096
static char cmdbuf[MAX_CMDBUF];
static char *arg_ptr[128];
static int arg_val[128];
static int arg_hex[128];
static int arg_cnt;
static char isExit = 0;         //1:コマンド入力ループを抜ける.

static int adrs = 0;
static int adrs2 = 0;
static int memcnt = 0;
static int arena = 0;

static char usb_serial[128];    /* 使用するHIDaspxのシリアル番号を格納 */


//
//  オプション処理ワーク.
//
static char verbose_mode = 0;   //1:デバイス情報をダンプ.
static FILE *script_fp = NULL;  //外部ファイルからコマンドを読み込む.
static int log_mode = 0;        //端末ログをファイルに書き込む.

/**********************************************************************
 *		文字列 p を 文字 c で分割し、args[] 配列に入れる。
 *		文字列は分断されることに注意。
 *		空白もスプリッターになるようにしているので注意
 **********************************************************************
 */
static int splitchr = ' ';
static int is_spc(int c)
{
    if (c == splitchr)
        return 1;
    if (c == ' ')
        return 1;
    if (c == '\t')
        return 1;
    return (0);
}

/**********************************************************************
 *		文字列 *p をデリミタ文字 c あるいは空白で分割し
 *             *args[] ポインタに入れる
 *		分割数 n を返す.
 **********************************************************************
 */
static int split_str(char *p, int c, char **args)
{
    int argc = 0;
    int qq = 0;
    splitchr = c;

    while (1) {
        while (is_spc(*p))
            p++;
        if (*p == 0)
            break;

        if (*p != 0x22) {
            args[argc++] = p;   /* 項目の先頭文字 */

            while (*p) {        /* 区切り文字まで読み進める */
                if (is_spc(*p))
                    break;
                p++;
            }

            if (is_spc(*p)) {   /* 区切り文字であれば */
                *p = 0;
                p++;            /* NULLで切る */
            }
        } else {
            qq = *p++;
            args[argc++] = p;   /* 項目の先頭文字 */

            while (*p) {        /* 区切り文字まで読み進める */
                if (*p == qq)
                    break;
                p++;
            }
            if (*p == qq) {     /* 区切り文字であれば */
                *p = 0;
                p++;            /* NULLで切る */
            }
        }
    }
    return argc;
}


/*********************************************************************
 *	文字 c が空白文字なら1を返す.
 *********************************************************************
 */
int is_space(int c)
{
    if ((c == ' ') || (c == '\t'))
        return 1;
    return 0;
}

/*********************************************************************
 *	文字列 *buf の先頭余白を読み飛ばす.
 *********************************************************************
 */
char *sp_skip(char *buf)
{
    while (*buf) {
        if (is_space(*buf) == 0)
            return buf;
        buf++;
    }
    return buf;
}

/*********************************************************************
 *	文字列 *buf を2進値 '0101_1111' と見なせるかどうかチェックする.
 *********************************************************************
 */
int radix2scanf(char *s, int *p)
{
    int rc = 0;
    int c;

    // 2進値を書く場合の条件 = 10文字以下、かつ、 _ を含んでいること.
    // 0b ではじめる方法もあるが、16進値と区別がつかないので,出来れば_を書く.

    if ((strlen(s) <= 10) && (strchr(s, '_'))) {
        while (1) {
            c = *s++;
            if (c == 0)
                break;          //文字列終端.
            if ((c == '0') || (c == '1')) {
                rc = rc << 1;   // 2倍.
                rc |= (c - '0');    // '1' ならLSBをOn.
            } else if (c != '_') {
                return (-1);    //許容できない文字が来たので失敗.
            }
        }
        *p = rc;                //2進値を返す.
        return 0;               // OK.
    }
    return (-1);
}

/*********************************************************************
 *	arg_ptr[] の文字列をHEX数値とみなして値に変換し arg_hex[]に入れる
 *********************************************************************
 */
void scan_args(int arg_cnt)
{
    int i, v;
    for (i = 0; i < arg_cnt; i++) {
        v = portAddress(arg_ptr[i]);    //シンボル名による解決.
        if (v) {
            arg_hex[i] = v;
            arg_val[i] = v;
        } else {
            v = (-1);
            sscanf(arg_ptr[i], "%x", &v);
            arg_hex[i] = v;

            radix2scanf(arg_ptr[i], &arg_hex[i]);

            v = (-1);
            sscanf(arg_ptr[i], "%d", &v);
            arg_val[i] = v;
        }
    }
}

/*********************************************************************
 *	文字列 *buf から、パラメータを読み取り、( adrs , memcnt ) を決める
 *********************************************************************
 */
int get_arg(char *buf)
{
    arena = AREA_RAM;
    if (*buf == 'p') {
        buf++;
        arena = AREA_PGMEM;
    } else if (*buf == 'r') {
        buf++;
        arena = AREA_EEPROM;
    }
    memcnt = 64;
    adrs2 = (-1);

    buf = sp_skip(buf);
    if (*buf == 0)
        return 0;

    arg_cnt = split_str(buf, ',', arg_ptr);
    scan_args(arg_cnt);

    if (arg_cnt >= 1) {
        adrs = arg_hex[0];
    }
    if (arg_cnt >= 2) {
        adrs2 = arg_hex[1];
        if (adrs2 != (-1)) {
            memcnt = adrs2 - adrs + 1;
        }
        if (memcnt < 0) {
            memcnt = adrs2;
        }
    }
    return arg_cnt;
}

/*********************************************************************
 *	文字列 *buf がコマンド名 *cmd を含んでいるかどうか調べる.
 *********************************************************************
 */
int str_comp(char *buf, char *cmd)
{
    while (*cmd) {
        if (*buf != *cmd) {
            return 1;
        }                       // 不一致.
        cmd++;
        buf++;
    }
    return 0;
}

/*********************************************************************
 *
 *********************************************************************
 */
void usage(void)
{
    fputs(HELP_USAGE, stdout);
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdHelp(char *buf)
{
    fputs(HELP_MESSAGES, stdout);
}


/*********************************************************************
 *	1bitだけOnのマスクパタンから、bit番号(0..7)に変換.
 *********************************************************************
 */
static int get_bpos(int mask)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (mask & 1)
            return i;
        mask >>= 1;
    }
    return (-1);
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdQuestion(char *buf)
{
    int v;
    if (get_arg(buf) < 1) {
        cmdHelp("");
    } else {
        v = portAddress(arg_ptr[0]);
        if (v)
            printf("%s=0x%x\n", arg_ptr[0], v);
    }
}

void cmdPortPrintOne(char *name, int adrs, int val)
{
    char tmp[32];
    printf("%8s(%02x) %02x %s\n", name, adrs, val, radix2str(tmp, val));
}

void cmdPortPrintOneCr(int count, char *name, int adrs, int val)
{
    char tmp[32];
    printf("%5d: %8s(%02x) %02x %s\r", count, name, adrs, val, radix2str(tmp, val));
}

void cmdPortPrintAllCr(int count, unsigned char *pinbuf)
{
    char tmp1[32];
    char tmp2[32];
    char tmp3[32];
    int pina = pinbuf[9];
    int pinb = pinbuf[6];
    int pind = pinbuf[0];

    printf("%5d: PINA %02x %s | PINB %02x %s | PIND %02x %s \r",
           count,
           pina,
           radix2str(tmp1, pina)
           , pinb, radix2str(tmp2, pinb)
           , pind, radix2str(tmp3, pind)
        );
}

void cmdPortPrintOne_b(char *name, int adrs, int val, int mask)
{
    int bpos = 0;
    char bitlist[16] = "____ ____";
    if (mask == 0) {
        cmdPortPrintOne(name, adrs, val);
    } else {
        bpos = get_bpos(mask);
        if (bpos < 0) {
            cmdPortPrintOne(name, adrs, val);
        } else {
            bpos = 7 - bpos;
            if (bpos >= 4)
                bpos++;

            if (val & mask)
                bitlist[bpos] = '1';
            else
                bitlist[bpos] = '0';
            printf("%8s(%02x) %02x %s\n", name, adrs, val, bitlist);
        }
    }
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdPort(char *buf)
{
    int v, n, mask = 0;
    n = get_arg(buf);           //引数の数.
    if (n != 0) {
        v = portAddress_b(arg_ptr[0], &mask);   //ポート名称からアドレスを算出.
        if (v) {                // アドレスが存在する.
            if (n < 2) {        // 1個のポートを読み取る.
                int val = UsbPeek(v, 0);
                cmdPortPrintOne_b(arg_ptr[0], v, val, mask);
            } else {            // p <adrs> <data>
                UsbPoke_b(v, 0, arg_hex[1], mask);  //書き込み.
                int val = UsbPeek(v, 0);
                cmdPortPrintOne_b(arg_ptr[0], v, val, mask);    //再読み込み.
            }
            return;
        } else if (strcmp(arg_ptr[0], "*") == 0) {  //ダンプ形式でポート表示する.
            PrintPortAll(0);
            return;
        } else if (strcmp(arg_ptr[0], ".") == 0) {  //段組形式でポート表示する.
            PrintPortColumn();
            return;
        } else if (strcmp(arg_ptr[0], "?") == 0) {  //ポート名称の一覧を出す.
            PrintPortNameList();
            return;
        }
    }
    PrintPortAll(1);            //よく参照するポートだけ表示する.
}

/*********************************************************************
 *	ポート内容をポーリングする.
 *********************************************************************
 */
void cmdPoll(char *buf)
{
    int v, n, count, i;
    int kbhit_NG, hit;

    hit = 0;
#if _LINUX_
    kbhit_NG = set_term(0);           // init_term
#else
    kbhit_NG = 0;    /* 0 == OK, -1 == NG */
#endif

    n = get_arg(buf);           // 引数の数.
    if (n != 0) {
        if (n < 2) {            // 1個のポートを読み取る.
            count = 1000;
        } else {
            count = arg_val[1];
        }

        v = portAddress(arg_ptr[0]);    //ポート名称からアドレスを算出.
        if (v) {                        // アドレスが存在する.
            UsbSetPoll(v, 0);
            for (i = 0; i < count; i++) {
                int val = UsbPoll();
                cmdPortPrintOneCr(count-i, arg_ptr[0], v, val);
                if (!kbhit_NG && kbhit()) {
                    hit = 1;
                    break;
                }
            }
        } else {
            //  A,B,D全ポート読み.
            unsigned char portbuf[16];
            int adr = 0x30;     // PORTD
            int size = 10;      // 0x30?0x39

            for (i = 0; i < count; i++) {
                int rc = UsbRead(adr, arena, portbuf, size);
                if (rc != size) {
                    printf("poll error %d.\n", rc);
                    return;
                }
                cmdPortPrintAllCr(count-i, portbuf);
                if (!kbhit_NG && kbhit()) {
                    hit = 1;
                    break;
                }
            }
        }
    }
    printf("\n\n");
#if _LINUX_
    if (!kbhit_NG) {
        if (hit) {
            getchar();
        }
      set_term(1);                // restore term
    }
#else
    if (!kbhit_NG) {
        if (hit) {
            getch();
        }
    }
#endif
}

/*********************************************************************
 *	ポート内容をポーリングする.
 *********************************************************************
 */
#define	SCREEN_W	768
#define	SCREEN_X0	32
#define	SCREEN_H	480

#define	SIG_COLOR	0x00f000
#define	SIG_COLOR2	0x004000
#define	BACK_COLOR	0x000000
#define	FRAME_COLOR	0x504000

int calc_ypos(int i)
{
    int off;
    if (i >= 4)
        off = 24;
    else
        off = 0;

    int y1 = 80 + i * 48 + off;
    return y1;
}

/*********************************************************************
 *	1サンプルの1ビット分を描画
 *********************************************************************
 *	v    : 0 か 非0   信号レベル.
 *  diff : 0 か 非0   信号反転が起きた.
 */
void plot_signal(int x, int y, int v, int diff)
{
    int col0, col1, col2;

    if (v) {
        col0 = FRAME_COLOR;
        col1 = SIG_COLOR;
    } else {
        col0 = SIG_COLOR;
        col1 = BACK_COLOR;
    }

    if (diff) {
        col2 = SIG_COLOR2;
    } else {
        col2 = BACK_COLOR;
    }

    if (diff)
        gr_vline(x, y - 1, x, y - 32, col2);    //縦の線.
    if (v) {
        gr_pset(x, y - 32, col1);   // Hの色.
    } else {
        gr_pset(x, y - 1, col0);    // Lの色.
    }
    gr_pset(x, y, FRAME_COLOR); // ベースライン赤.
}

/*********************************************************************
 *	1サンプルを描画.
 *********************************************************************
 */
void draw_sample(int x, int y, int val)
{
    static int old_val;
    int i, y1;
    int m = 1;
    int diff = val ^ old_val;   //前回と値が変わった?

    for (i = 0; i < 8; i++, m <<= 1) {
        y1 = calc_ypos(i);
        plot_signal(x, y + y1, val & m, diff & m);
    }
    old_val = val;
}

/*********************************************************************
 *	グラフ表示内の文字を書く.
 *********************************************************************
 */
void draw_PortName(int adr)
{
    int i, y1;
    char string[128];
    int color = 0x80ff00;
    char *port = "-";

    if (adr == 0x39)
        port = "A";
    if (adr == 0x36)
        port = "B";
    if (adr == 0x30)
        port = "D";

    for (i = 0; i < 8; i++) {
        sprintf(string, "P%s%d", port, i);
        y1 = calc_ypos(i);
        gr_puts(4, y1 - 20, string, color);
    }
}

/*********************************************************************
 *	グラフ表示.(仮)
 *********************************************************************
 */
void cmdGraph(char *buf)
{
#if _LINUX_
    printf("graph command not support.\n");
    return;
#else
    int x = SCREEN_X0;
    int adr = 0x36;             // default PINB
    int n = get_arg(buf);       //引数の数.
    if (n) {
        adr = portAddress(arg_ptr[0]);  //ポート名称からアドレスを算出.
    }

    gr_init(SCREEN_W, SCREEN_H, 32, 0);
    draw_PortName(adr);

    UsbSetPoll(adr, 0);
    do {
        int val = UsbPoll();
        gr_vline(x, 0, x, SCREEN_H - 1, 0); //縦の線.
        draw_sample(x, 0, val);
        x++;
        if (x >= SCREEN_W)
            x = SCREEN_X0;
        gr_vline(x, 0, x, SCREEN_H - 1, 0xffff00);  //縦の線.
    } while (gr_break() == 0);
    gr_close();
#endif
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdQuit(char *buf)
{
    printf("Bye.\n");
    isExit = 1;
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdDump(char *buf)
{
    get_arg(buf);
    UsbDump(adrs, arena, (memcnt + 7) & (-8));
    adrs += memcnt;
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdEdit(char *buf)
{
    int mask = 0;
    int argc = get_arg(buf);

    if (argc >= 2) {
        UsbPoke(adrs, arena, adrs2, mask);
    } else if (argc >= 1) {
        int val = UsbPeek(adrs, arena);
        printf("%06x %02x\n", adrs, val);
    }
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdFill(char *buf)
{
    int i;
    if (get_arg(buf) >= 3) {
        for (i = adrs; i <= adrs2; i++) {
            UsbPoke(i, arena, arg_hex[2], 0);
        }
    }
}

/*********************************************************************
 *
 *********************************************************************
 */
void cmdBench(char *buf)
{
    int cnt = 1000;
    int packet = 8;
    int argc = get_arg(buf);

    if (argc >= 1) {
        cnt = arg_val[0];
    }
    if (argc >= 2) {
        packet = arg_val[1];
    }
//  printf("%d,%d\n",cnt,packet);
    UsbBench(cnt, packet);
}

#if 0
/*********************************************************************
 *
 *********************************************************************
 */
#ifndef FOSC
#define FOSC 12000000           // MCUクロック周波数
#endif

// ATtiny USART Baud Rate Register High UBBRH[11:8]
#define UBRRH   _SFR_IO8(0x02)

// ATtiny USART Control and Status Register C UCSRC
#define UCSRC   _SFR_IO8(0x03)

#define UMSEL   6
#define UPM1    5
#define UPM0    4
#define USBS    3
#define UCSZ1   2
#define UCSZ0   1
#define UCPOL   0

UCSRC = (1<<USBS)|(3<<UCSZ0);     フレーム形式設定(8ビット,2ストップ ビット)

#define BAUD 9600             	// 目的USARTボーレート速度
#define MYUBRR FOSC/16/BAUD-1 	// 目的UBRR値
#endif

/*********************************************************************
 *
 *********************************************************************
 */
void cmdSleep(char *buf)
{
#ifdef _LINUX_
    extern int _sleep(int ms);
#endif

    int cnt = 1000;
    int argc = get_arg(buf);
    if (argc >= 1) {
        cnt = arg_val[0];
    }

    _sleep(cnt);
}

/*********************************************************************
 *
 *********************************************************************
 */


CmdList cmdlist[] = {
    {"h", cmdHelp} ,
    {"?", cmdQuestion} ,
    {"bench", cmdBench} ,
    {"sleep", cmdSleep} ,
    {"poll", cmdPoll} ,
    {"graph", cmdGraph} ,
    {"exit", cmdQuit} ,
    {"q", cmdQuit} ,
    {"d", cmdDump} ,
    {"m", cmdEdit} ,
    {"e", cmdEdit} ,
    {"f", cmdFill} ,
    {"p", cmdPort} ,
    {NULL, cmdQuit} ,
};



/*********************************************************************
 *	モニタコマンド解析と実行.
 *********************************************************************
 */
void cmdMonit(char *buf)
{
    CmdList *t;
    for (t = cmdlist; t->name != NULL; t++) {
        if (str_comp(buf, t->name) == 0) {
            buf += strlen(t->name);
            t->func(buf);
            return;
        }
    }
    /*error */
}

/*********************************************************************
 *	行末のCR/LF を削除する.
 *********************************************************************
 */
void chop_crlf(char *p)
{
    while (*p) {
        if ((*p == '\n') || (*p == '\r')) {
            *p = 0;
            return;
        }
        p++;
    }
}

/*********************************************************************
 *	1行入力. ファイルもしくはコンソールの両方を許容.
 *********************************************************************
 */
static void get_line(char *buf)
{
    char *rc;
    if (script_fp != NULL) {
        rc = fgets(buf, MAX_CMDBUF, script_fp); //ファイルから入力.
        if (rc == NULL) {       //ファイル末もしくはエラー.
            buf[0] = 0;         //空行を返す.
            fclose(script_fp);
            script_fp = NULL;
        } else {
            chop_crlf(buf);     //行末のCR/LF を削除する.
            printf("%s\n", buf);    //コマンドエコーする.
        }
    } else {
        rc = fgets(buf, MAX_CMDBUF, stdin); //コンソールから入力.
        chop_crlf(buf);         //行末のCR/LF を削除する.
        if (log_mode) {
            printf("%s\n", buf);    //コマンドエコーする.
        }
    }
}

/*********************************************************************
 *	メイン
 *********************************************************************
 */
int main(int argc, char **argv)
{
#if _LINUX_
    extern char *strupr(char *);
    char *readline_cmd;
    using_history();
    read_history(NULL);
#endif
    //オプション解析.
    Getopt(argc, argv, "i");
    if (IsOpt('h') || IsOpt('?') || IsOpt('/')) {
        usage();
        exit(0);
    }
    if (IsOpt('i')) {
        script_fp = fopen(Opt('i'), "rb");
    }
    if (IsOpt('l')) {
        log_mode = 1;
    }
    if (IsOpt('v')) {           //デバイス情報をダンプする.
        verbose_mode = 1;
    }
    strcpy(usb_serial, DEFAULT_SERIAL); // 省略時
    if (IsOpt('p')) {
        char *s;
        s = Opt('p');
        if (s) {
            if (*s == ':')
                s++;
            if (*s == '?') {
                hidasp_list("hidmon");
                exit(0);
            } else if (isdigit(*s)) {
                if (s) {
                    int n, l;
                    l = strlen(s);
                    if (l < 4 && isdigit(*s)) {
                        n = atoi(s);
                        if ((0 <= n) && (n <= 999)) {
                            sprintf(usb_serial, "%04d", n);
                        } else {
                            if (l == 1) {
                                usb_serial[3] = s[0];
                            } else if (l == 2) {
                                usb_serial[2] = s[0];
                                usb_serial[3] = s[1];
                            } else if (l == 3) {
                                usb_serial[1] = s[0];
                                usb_serial[2] = s[1];
                                usb_serial[3] = s[2];
                            }
                        }
                    } else {
                        strncpy(usb_serial, s, 4);
                    }
                }
            } else if (*s == '\0') {
                strcpy(usb_serial, DEFAULT_SERIAL); // -pのみの時
            } else {
                strncpy(usb_serial, s, 4);
            }
            strupr(usb_serial);
        }
    }
    //初期化.
    if (UsbInit(verbose_mode, 0, usb_serial) == 0) {
        exit(1);
    }
#if 1                           // add by senshu
#define PIND	0x30
    {
        int pind;
        pind = hidPeekMem(PIND);
        if (pind & (1 << 2)) {  // PD2 check
            printf("HIDaspx is Programmer mode.\n");
        } else {
            printf("HIDaspx is USB-IO mode.\n");
        }
    }
#endif

#if _LINUX_
    if (script_fp) {
      do {                       // script exec.
          printf("AVR> ");
          get_line(cmdbuf);
          cmdMonit(cmdbuf);
      } while (isExit == 0);
    } else {                     //モニターモード.
      while ((readline_cmd = readline("AVR>")) != NULL) {
          add_history(readline_cmd);
          cmdMonit(readline_cmd);
          free(readline_cmd);
          if (isExit)  break;
      }
    }
    write_history(NULL);
#else
    do {                        //モニターモード.
        printf("AVR> ");
        get_line(cmdbuf);
        cmdMonit(cmdbuf);
    } while (isExit == 0);
#endif

    UsbExit();

    if (script_fp != NULL)
        fclose(script_fp);

    return 0;
}

/*********************************************************************
 *
 *********************************************************************
 */
void report_update(void)
{
}
