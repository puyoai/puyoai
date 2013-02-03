/* portlist.c */

#ifdef _LINUX_
#include <unistd.h>
#define stricmp strcasecmp
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "monit.h"
#include "portlist.h"
#include "util.h"

/*********************************************************************
 *	ポート名称	,  アドレス , 属性（バイト幅、よく参照されるかどうか）
 *********************************************************************
 */
PortList portList[] = {
    {"DIDR", _SFR_IO8(0x01)},
    {"UBRRH", _SFR_IO8(0x02)},
    {"UCSRC", _SFR_IO8(0x03)},
    {"ACSR", _SFR_IO8(0x08)},
    {"UBRRL", _SFR_IO8(0x09)},
    {"UCSRB", _SFR_IO8(0x0A)},
    {"UCSRA", _SFR_IO8(0x0B)},
    {"UDR", _SFR_IO8(0x0C)},
    {"RXB", _SFR_IO8(0x0C)},
    {"TXB", _SFR_IO8(0x0C)},
    {"USICR", _SFR_IO8(0x0D)},
    {"USISR", _SFR_IO8(0x0E)},
    {"USIDR", _SFR_IO8(0x0F)},
    {"PIND", _SFR_IO8(0x10) | QQ},
    {"DDRD", _SFR_IO8(0x11)},
    {"PORTD", _SFR_IO8(0x12) | QQ},
    {"GPIOR0", _SFR_IO8(0x13)},
    {"GPIOR1", _SFR_IO8(0x14)},
    {"GPIOR2", _SFR_IO8(0x15)},
    {"PINB", _SFR_IO8(0x16) | QQ},
    {"DDRB", _SFR_IO8(0x17)},
    {"PORTB", _SFR_IO8(0x18) | QQ},
    {"PINA", _SFR_IO8(0x19) | QQ},
    {"DDRA", _SFR_IO8(0x1A)},
    {"PORTA", _SFR_IO8(0x1B) | QQ},
    {"EECR", _SFR_IO8(0x1C)},
    {"EEDR", _SFR_IO8(0x1D)},
    {"EEAR", _SFR_IO8(0x1E)},
    {"EEARL", _SFR_IO8(0x1E)},
    {"PCMSK", _SFR_IO8(0x20)},
    {"WDTCSR", _SFR_IO8(0x21)},
    {"TCCR1C", _SFR_IO8(0x22)},
    {"GTCCR", _SFR_IO8(0x23)},
    {"ICR1", _SFR_IO16(0x24)},
    {"ICR1L", _SFR_IO8(0x24)},
    {"ICR1H", _SFR_IO8(0x25)},
    {"CLKPR", _SFR_IO8(0x26)},
    {"OCR1B", _SFR_IO16(0x28)},
    {"OCR1BL", _SFR_IO8(0x28)},
    {"OCR1BH", _SFR_IO8(0x29)},
    {"OCR1", _SFR_IO16(0x2A)},
    {"OCR1L", _SFR_IO8(0x2A)},
    {"OCR1H", _SFR_IO8(0x2B)},
    {"OCR1A", _SFR_IO16(0x2A)},
    {"OCR1AL", _SFR_IO8(0x2A)},
    {"OCR1AH", _SFR_IO8(0x2B)},
    {"TCNT1", _SFR_IO16(0x2C)},
    {"TCNT1L", _SFR_IO8(0x2C)},
    {"TCNT1H", _SFR_IO8(0x2D)},
    {"TCCR1B", _SFR_IO8(0x2E)},
    {"TCCR1A", _SFR_IO8(0x2F)},
    {"TCCR0A", _SFR_IO8(0x30)},
    {"OSCCAL", _SFR_IO8(0x31)},
    {"TCNT0", _SFR_IO8(0x32)},
    {"TCCR0B", _SFR_IO8(0x33)},
    {"MCUSR", _SFR_IO8(0x34)},
    {"MCUCR", _SFR_IO8(0x35)},
    {"OCR0A", _SFR_IO8(0x36)},
    {"SPMCSR", _SFR_IO8(0x37)},
    {"TIFR", _SFR_IO8(0x38)},
    {"TIMSK", _SFR_IO8(0x39)},
    {"EIFR", _SFR_IO8(0x3A)},
    {"GIMSK", _SFR_IO8(0x3B)},
    {"OCR0B", _SFR_IO8(0x3C)},

    {NULL, 0, 0}
};

/*********************************************************************
 *	大文字小文字を区別しない文字列比較＋bit属性検出
 *********************************************************************
 */
int str_casecmpx(char *s, char *t, int *pmask)
{
    int bit;
    while (*t) {
        if (tolower(*s) != tolower(*t))
            return 1;           //不一致.
        s++;
        t++;
    }
    if (*s == 0) {
        *pmask = 0;
        return 0;               //一致.
    }
    if (*s == '.') {
        s++;
        if (s[1] == 0) {
            bit = s[0] - '0';
            if ((bit >= 0) && (bit < 8)) {
                *pmask = (1 << bit);
                return 0;       // port.bit (bitは0〜7) 一致.
            }
        }
    }
    return 1;                   //不一致.
}

/*********************************************************************
 *	ポート名称からアドレスを求める.
 *********************************************************************
 */
int portAddress(char *s)
{
    PortList *p = portList;
    while (p->name) {
        if (stricmp(s, p->name) == 0)
            return p->adrs;
        p++;
    }
    return 0;
}

/*********************************************************************
 *	ポート名称からアドレスとビットマスクを求める.
 *********************************************************************
 */
int portAddress_b(char *s, int *pmask)
{
    PortList *p = portList;
    while (p->name) {
        if (str_casecmpx(s, p->name, pmask) == 0) {
            return p->adrs;
        }
        p++;
    }
    return 0;
}

#define	COLS 4

/*********************************************************************
 *	ポート一覧表を表示する.
 *********************************************************************
 */
void PrintPortNameList(void)
{
    PortList *p = portList;
    int i, k;
    int m = 0;
    char buf[128][128];

    memset(buf, 0, sizeof(buf));

    while (p->name) {
        sprintf(buf[m], "%8s = 0x%02x ", p->name, p->adrs);
        m++;
        p++;
    }
    //段組みしたい.
    k = (m + COLS - 1) / COLS;  //COLS段にした場合の行数.
    for (i = 0; i < k; i++) {
        printf("%s%s%s%s\n", buf[i], buf[i + k], buf[i + k * 2],
               buf[i + k * 3]);
    }
}

/*********************************************************************
 *	数値 d を二進数文字列に変換し buf に格納する.
 *		bufと同じアドレスを返す.
 *********************************************************************
 */
char *radix2str(char *buf, int d)
{
    char *rc = buf;
    int i, m, c = ' ';
    m = 0x80;
    for (i = 0; i < 8; i++) {
        if (d & m)
            c = '1';
        else
            c = '0';
        *buf++ = c;
        m >>= 1;
        if (i == 3)
            *buf++ = '_';
    }
    *buf = 0;
    return rc;
}

/*********************************************************************
 *	段組表示によるポート値出力.
 *********************************************************************
 */
void PrintPortColumn(void)
{
    PortList *p = portList;
    int i, k;
    int m = 0;
    static char buf[128][128];  //結果バッファ（段組）

    char tmp[128];              //２進数化するバッファ.

    uchar data[0x20 + 0x40];    //ポート読み取り値.
    UsbRead(0x20, AREA_RAM, data + 0x20, 0x40);
//  memdump(data+0x20,0x40,0x20);return;

    memset(buf, 0, sizeof(buf));

    while (p->name) {
        if (p->attr & 1) {
            sprintf(buf[m], "%6s = %s ", p->name,
                    radix2str(tmp, data[p->adrs]));
            m++;
//          sprintf(buf[m],"%6s = %x ",p->name,p->adrs);
//          m++;
        }
        p++;
    }
    //段組みしたい.
    k = (m + COLS - 1) / COLS;  //COLS段にした場合の行数.
    for (i = 0; i < k; i++) {
        printf("%s%s%s%s\n", buf[i], buf[i + k], buf[i + k * 2],
               buf[i + k * 3]);
    }
}

/*********************************************************************
 *	ポート内容を全部ダンプする.
 *********************************************************************
 */
void PrintPortAll(int mask)
{
    PortList *p = portList;
    uchar data[0x20 + 0x40];    //ポート読み取り値.
    UsbRead(0x20, AREA_RAM, data + 0x20, 0x40);
//  memdump(data+0x20,0x40,0x20);return;
    while (p->name) {
        if (p->attr & 1) {
            if ((mask == 0) || (p->attr & QQ)) {
                cmdPortPrintOne(p->name, p->adrs, data[p->adrs]);
            }
        }
        p++;
    }
}

/*********************************************************************
 *
 *********************************************************************
 */
