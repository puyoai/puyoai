/*********************************************************************
 *	USB経由のターゲットメモリーアクセス
 *********************************************************************
 *API:
in    UsbInit(int verbose,int enable_bulk, char *serial);  // 初期化.
int   UsbExit(void);                 // 終了.
void  UsbBench(int cnt,int psize);    // ベンチマーク

void  UsbPoke(int adr,int arena,int data,int mask);   // 書き込み
int   UsbPeek(int adr,int arena);                     // 1バイト読み出し
int   UsbRead(int adr,int arena,uchar *buf,int size); // 連続読み出し
void  UsbDump(int adr,int arena,int size);            // メモリーダンプ
 *
 *内部関数:
void  dumpmem(int adr,int cnt);
void  pokemem(int adr,int data0,int data1);
void  memdump_print(void *ptr,int len,int off);
 */

#ifdef	_DOS_
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if _LINUX_
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "hidasp.h"
#include "monit.h"
#include "portlist.h"
#include "util.h"

#define	VERBOSE	0

#define	if_V	if(VERBOSE)

static char verbose_mode = 0;
static int HidLength1 = 7;
static int HidLength = 31;

// ReportID:4  POLLING PORTが実装済である.
static int pollcmd_implemented;

//  保護ポート
#define	PROTECTED_PORT_MAX	3
static int protected_ports[PROTECTED_PORT_MAX] = { -1, -1, -1 };

#define	PROTECTED_PORT_MASK	0x38    // 0011_1000


#define	ID4		4
#define	LENGTH4	5


/****************************************************************************
 *	メモリー内容をダンプ.
 ****************************************************************************
 *	void *ptr : ダンプしたいデータ.
 *	int   len : ダンプしたいバイト数.
 *	int   off : ターゲット側の番地表示.
 */
void memdump_print(void *ptr, int len, int off)
{
    unsigned char *p = (unsigned char *) ptr;
    int i, j, c;

    for (i = 0; i < len; i++) {
        if ((i & 15) == 0)
            printf("%06x", (int) p - (int) ptr + off);
        printf(" %02x", *p);
        p++;
        if ((i & 15) == 15) {
#if	1                           // ASCII DUMP
            printf("  ");
            for (j = 0; j < 16; j++) {
                c = p[j - 16];
                if (c < ' ')
                    c = '.';
                if (c >= 0x7f)
                    c = '.';
                printf("%c", c);
            }
#endif
            printf("\n");
        }
    }
    printf("\n");
}



/*********************************************************************
 *	AVRデバイスに処理コマンドを送って、必要なら結果を受け取る.
 *********************************************************************
 *	cmdBuf   *cmd : 処理コマンド
 *	uchar    *buf : 結果を受け取る領域.
 *  int reply_len : 結果の必要サイズ. (0の場合は結果を受け取らない)
 *戻り値
 *		0	: 失敗
 *	   !0   : 成功
 */
int QueryAVR(cmdBuf * cmd, uchar * buf, int reply_len)
{
    int rc = 0;
    int report_id = cmd->report_id;
    int size = cmd->size;
    char *s;
    rc = hidWriteBuffer((char *) cmd, HidLength1);
    if (rc == 0) {
        printf("hidWrite error\n");
        exit(1);
    }
    if (reply_len) {
        rc = hidReadBuffer((char *) cmd, reply_len);
        if (rc == 0) {
            if (report_id != 4) {
                printf("hidRead error %d\n", report_id);
                exit(1);
            }
        }
        s = (char *) cmd;
        memcpy(buf, s + 1, size);
    }
    return rc;
}

/*********************************************************************
 *	ターゲット側のメモリー内容を取得する
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	int           size :読み取りサイズ.
 *	unsigned char *buf :受け取りバッファ.
 *	注意: HID Report Length - 1 より長いサイズは要求してはならない.
 */
int dumpmem(int adr, int arena, int size, unsigned char *buf)
{
    cmdBuf cmd;

    memset(&cmd, 0, sizeof(cmdBuf));
    cmd.cmd = CMD_PEEK;         //| arena;
    cmd.size = size;
    cmd.adr[0] = adr;
    cmd.adr[1] = adr >> 8;

    if (QueryAVR(&cmd, buf, 1 + size) == 0)
        return 0;               //失敗.
    return size;
}

/*********************************************************************
 *	ターゲット側のメモリー内容(1バイト)を書き換える.
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	int          data0 :書き込みデータ.      (OR)
 *	int          data1 :書き込みビットマスク.(AND)
 *注意:
 *	ファーム側の書き込みアルゴリズムは以下のようになっているので注意.

	if(data1) {	//マスク付の書き込み.
		*adr = (*adr & data1) | data0;
	}else{			//べた書き込み.
		*adr = data0;
	}

 */
int pokemem(int adr, int arena, int data0, int data1)
{
    cmdBuf cmd;
    unsigned char buf[16];      // dummy

    memset(&cmd, 0, sizeof(cmdBuf));
    cmd.cmd = CMD_POKE;         //| arena;
    cmd.size = 1;
    cmd.adr[0] = adr;
    cmd.adr[1] = adr >> 8;
    cmd.data[0] = data0;
    cmd.data[1] = data1;
    return QueryAVR(&cmd, buf, 0);
}

/*********************************************************************
 *	ターゲットとの接続チェック(ベンチマークを兼ねる)
 *********************************************************************
 *	int i : 0〜255 の数値.
 *戻り値
 *		エコーバックされた i の値.
 */
int hid_ping(int i)
{
    cmdBuf cmd;
    unsigned char buf[128];     // dummy
    int rc;

    memset(&cmd, i, sizeof(cmdBuf));
    cmd.cmd = CMD_PING;
    QueryAVR(&cmd, buf, 0);

    rc = hidReadBuffer((char *) buf, HidLength);
    if (rc == 0) {
        printf("hidRead error\n");
        exit(1);
    }
    return buf[2] & 0xff;
}

/*********************************************************************
 *	転送速度ベンチマーク
 *********************************************************************
 *	int cnt  : PINGを打つ回数.
 *	int psize: PINGパケットの大きさ(現在はサポートされない)
 */
#if _LINUX_
double timeval_diff(struct timeval end, struct timeval start) 
{
    struct timeval interval;

    if (start.tv_usec > end.tv_usec) {
        end.tv_usec += 1000000;
        end.tv_sec--;
    }

    interval.tv_usec = end.tv_usec - start.tv_usec;
    interval.tv_sec  = end.tv_sec  - start.tv_sec;

    return interval.tv_sec + interval.tv_usec/1e6;
}

void UsbBench(int cnt, int psize)
{
    int i, n, rc;
    long long total = 0;

    struct timeval now, start;
    double time1, rate;

    int nBytes = HidLength1 + HidLength;    //現在固定.
    // 1回のPingは 7byteのQueryと31バイトのリターン.

    printf("hid write start\n");
    gettimeofday(&start, NULL);

    for (i = 0; i < cnt; i++) {
        n = i & 0xff;
        rc = hid_ping(n);
        if (rc != n) {
            printf("hid ping mismatch error (%x != %x)\n", rc, n);
            exit(1);
        }
        total += nBytes;
    }

    gettimeofday(&now, NULL);
    time1 = timeval_diff(now, start);

    if (time1 <= 0.001) {
        time1 = 0.001;
    }

    rate = total / time1;

    if (total > (1024 * 1024)) {
        printf("hid write end, %8.3lf MB/%8.2lf s,  %6.3lf kB/s\n",
               (double) total / (1024 * 1024), time1 , rate / 1024.0);
    } else if (total > 1024) {
        printf("hid write end, %8.3lf kB/%8.2lf s,  %6.3lf kB/s\n",
               (double) total / 1024, time1, rate / 1024.0);
    } else {
        printf("hid write end, %8d B/%8d ms,  %6.3lf kB/s\n",
               (int) total, (int)(time1*1000), rate / 1024.0);
    }
}
#else
void UsbBench(int cnt, int psize)
{
    int i, n, rc;
    int time1, rate;
    long long total = 0;
    int nBytes = HidLength1 + HidLength;    //現在固定.
    // 1回のPingは 7byteのQueryと31バイトのリターン.

    printf("hid write start\n");
    int time0 = clock();

    for (i = 0; i < cnt; i++) {
        n = i & 0xff;
        rc = hid_ping(n);
        if (rc != n) {
            printf("hid ping mismatch error (%x != %x)\n", rc, n);
            exit(1);
        }
        total += nBytes;
    }

    time1 = clock() - time0;
    if (time1 == 0) {
        time1 = 2;
    }
    rate = (int) ((total * 1000) / time1);

    if (total > 1024 * 1024) {
        printf("hid write end, %8.3lf MB/%8.2lf s,  %6.3lf kB/s\n",
               (double) total / (1024 * 1024), (double) time1 / 1000,
               (double) rate / 1024);
    } else if (total > 1024) {
        printf("hid write end, %8.3lf kB/%8.2lf s,  %6.3lf kB/s\n",
               (double) total / 1024, (double) time1 / 1000,
               (double) rate / 1024);
    } else {
        printf("hid write end, %8d B/%8d ms,  %6.3lf kB/s\n",
               (int) total, time1, (double) rate / 1024);
    }
}
#endif

/********************************************************************
 *	ターゲット側メモリーをダンプする.
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	int            cnt :読み取りサイズ.
 */
void UsbDump(int adr, int arena, int cnt)
{
    unsigned char buf[16];
    int size;
    int rc;
    while (cnt) {
        size = cnt;
        if (size >= 8)
            size = 8;
        rc = dumpmem(adr, arena, size, buf);    //メモリー内容の読み出し.

        if (rc != size)
            return;
        memdump_print(buf, size, adr);  //結果印字.
        adr += size;
        cnt -= size;
    }
}

/*********************************************************************
 *	ターゲット側メモリー連続読み出し
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	uchar         *buf :読み取りバッファ.
 *	int           size :読み取りサイズ.
 *戻り値
 *	-1    : 失敗
 *	正の値: 成功.
 */
int UsbRead(int adr, int arena, uchar * buf, int size)
{
    int rc = size;
    int len;
    while (size) {
        len = size;
        if (len >= 8)
            len = 8;
        int rc = dumpmem(adr, arena, len, buf);
        if (rc != len) {
            return -1;
        }
        adr += len;             // ターゲットアドレスを進める.
        buf += len;             // 読み込み先バッファを進める.
        size -= len;            // 残りサイズを減らす.
    }
    return rc;
}

/*********************************************************************
 *	ターゲット側メモリー1バイト読み出し
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *戻り値
 *	メモリー内容の値.
 */
int UsbPeek(int adr, int arena)
{
    unsigned char buf[16];
    int size = 1;
    int rc = UsbRead(adr, arena, buf, size);
    if (rc != size) {
        return -1;
    }
    return buf[0];
}

static int poll_port = 0;

/*********************************************************************
 *	ターゲット側メモリー1バイト連続読み出しのセットアップ
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *戻り値
 *	メモリー内容の値.
 */
int UsbSetPoll_slow(int adr, int arena)
{
    poll_port = adr;
    return 1;
}

/*********************************************************************
 *	ターゲット側メモリー1バイト連続読み出し
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *戻り値
 *	メモリー内容の値.
 */
int UsbPoll_slow()
{
    return UsbPeek(poll_port, 0);
}

/*********************************************************************
 *	ターゲット側メモリー1バイト連続読み出しのセットアップ
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *戻り値
 *	メモリー内容の値.
 */
int UsbSetPoll(int adr, int arena)
{
    unsigned char buf[128];
    unsigned char rd_buf[128];

    if (pollcmd_implemented == 0) {
        return UsbSetPoll_slow(adr, arena);
    }

    memset(buf, 0, sizeof(buf));

    buf[0] = ID4;
    buf[1] = CMD_SETPAGE;
    buf[2] = 0;                 // page_mode
    buf[3] = adr;               // page_addr
    buf[4] = adr >> 8;          // page_addr_h

    if (QueryAVR((cmdBuf *) buf, rd_buf, 0) == 0)
        return 0;               //失敗.
    return 1;
}

/*********************************************************************
 *	ターゲット側メモリー1バイト連続読み出し
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *戻り値
 *	メモリー内容の値.
 */
int UsbPoll()
{
    char buf[128];              // dummy
    int rc;

    if (pollcmd_implemented == 0) {
        return UsbPoll_slow();
    }

    rc = hidReadPoll(buf, LENGTH4, ID4);
    if (rc == 0) {
#if	0
        printf("hidRead error\n");
        exit(1);
#else
        return -1;
#endif
    }
    return buf[1] & 0xff;
}



/*********************************************************************
 *	ターゲット側のメモリー内容(1バイト)を書き換える.
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	int           data :書き込みデータ.
 *	int           mask :書き込み有効ビットマスク.
 *注意
 *  mask  = 0 の場合は全ビット有効 (直書きする)
 *	mask != 0 の場合は、maskビット1に対応したdataのみ更新し、他は変更しない.
 */
int usbPoke(int adr, int arena, int data, int mask)
{
    int data0, data1;
    if (mask == 0) {
        // マスク不要の直書き.
        data0 = data;
        data1 = 0;
    } else {
        // マスク処理を含む書き込み.
        // 書き込みデータの有効成分は mask のビットが 1 になっている部分のみ.
        // mask のビットが 0 である部分は、元の情報を保持する.

        data0 = data & mask;    // OR情報.
        data1 = 0xff & (~mask); // AND情報.

        // マスク書き込みのロジックは以下.
        // *mem = (*mem & data1) | data0;
    }

    return pokemem(adr, arena, data0, data1);
}

/*********************************************************************
 *	ターゲット側のメモリー内容(1バイト)を書き換える.
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	int           data :書き込みデータ.
 *	int           mask :書き込み有効ビットマスク.
 *注意
 *  mask  = 0 の場合は全ビット有効 (直書きする)
 *	mask != 0 の場合は、maskビット1に対応したdataのみ更新し、他は変更しない.
 */
int UsbPoke(int adr, int arena, int data, int mask)
{
    int i;
    int f = 0;
    for (i = 0; i < PROTECTED_PORT_MAX; i++) {
        if (adr == protected_ports[i])
            f = 1;
    }

    if (f == 0)
        return usbPoke(adr, arena, data, mask);

    if (mask == 0)
        mask = 0xff;
    mask &= ~PROTECTED_PORT_MASK;   // USB D+,D-,PULLUP ビットの書き換えをマスク.
    data &= ~PROTECTED_PORT_MASK;

    if (adr == protected_ports[0]) {
        mask = 0;
    }                           // pind はmask writeしてはいけない.

    return usbPoke(adr, arena, data, mask);
}

/*********************************************************************
 *	1ビットの書き込み
 *********************************************************************
 *	int            adr :ターゲット側のメモリーアドレス
 *	int          arena :ターゲット側のメモリー種別(現在未使用)
 *	int            bit :書き込みデータ(1bit) 0 もしくは 1
 *	int           mask :書き込み有効ビットマスク.
 */
void UsbPoke_b(int adr, int arena, int bit, int mask)
{
    int data = 0;
    if (mask == 0) {
        //1バイトの書き込み.
        UsbPoke(adr, arena, bit, 0);
    } else {
        //1ビットの書き込み.
        if (bit)
            data = 0xff;
        UsbPoke(adr, arena, data, mask);
    }
}

/*********************************************************************
 *
 *********************************************************************
 */
void local_init(void)
{
    protected_ports[0] = portAddress("pind");
    protected_ports[1] = portAddress("portd");
    protected_ports[2] = portAddress("ddrd");
}

/*********************************************************************
 *
 *********************************************************************
 */
void UsbCheckPollCmd(void)
{
    pollcmd_implemented = 1;    // とりあえず実装されていると仮定.
    {
        int gtccr = portAddress("gtccr");   // LSBのみ1か0のポート.
        int data;
        int rc = UsbSetPoll(gtccr, 0);
        if (rc == 0) {          // 失敗.
            pollcmd_implemented = 0;    // 実装されていないと判定.
            return;
        }

        data = UsbPoll();
        if ((data == (-1)) || (data & 0xfe)) {
            pollcmd_implemented = 0;    // 実装されていないと判定.
        }
    }
}

/*********************************************************************
 *	初期化
 *********************************************************************
 */
int UsbInit(int verbose, int enable_bulk, char *serial)
{
    verbose_mode = verbose;
    if (hidasp_init(serial) & HIDASP_MODE_ERROR) {
        printf("error: [%s] device not found!\n", serial);
        return 0;
    } else {
        local_init();
        UsbCheckPollCmd();
        return 1;               // OK.
    }
}



/*********************************************************************
 *	終了
 *********************************************************************
 */
int UsbExit(void)
{
    hidasp_close();
    return 0;
}
