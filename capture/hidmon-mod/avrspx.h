#ifndef __AVRSPX_H
#define __AVRSPX_H

#define HIDASP
#ifndef HIDASP_ONLY
#define USBASP
#define WONLY
#define RSCR
#define ORIGINAL
#endif

#if defined(WONLY) || defined(RSCR) || defined(ORIGINAL)
#define USE_COMPORT
#endif

#ifdef WIN32
#include <windows.h>
#else
//typedef unsigned long       DWORD;
//typedef int                 BOOL;
//typedef unsigned char       BYTE;
//typedef unsigned short      WORD;
#include <unistd.h>
#undef F_LOCK                   // FIXME!!
#define kbhit() getchar()
#define Sleep(t) usleep((t)*1000)
#define _stricmp strcasecmp
#define	HANDLE	 int
#endif

//----------------------------------------------------------------------------
#ifdef __BORLANDC__
# pragma warn -8012
# pragma warn -8060
# pragma warn -8065
#endif

#ifdef _MSC_VER
# pragma warning(disable : 4244)
# pragma warning(disable : 4267)
# pragma warning(disable : 4996)
#endif

#ifndef bool
# define bool	char
# define true	1
# define false	0
#endif

#ifndef _WINDEF_
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#endif                          /* _WINDEF_ */



#define MESS_OUT(str)	fputs(str, stdout)  // add by senshu
#define MESS(str)	fputs(str, stderr)

#define FUSEFILE "fuse.txt"
#if 0
#define INIFILE "avrsp.ini"
#endif


/* Program error codes */

enum {
    RC_FAIL = 1,
    RC_FILE,
    RC_INIT,
    RC_DEV,
    RC_SYNTAX,
    RC_OPT_ERR,
};


/* Buffer size for flash/eeprom */

#define	MAX_FLASH	(256*1024)  /* Max Flash size */
#define	MAX_EEPROM	(  4*1024)  /* Max EEPROM size */
#define	PIPE_WINDOW	256         /* Pipe window for SPI bridge (must be power of 2) */


/* Device property structure */

enum _devid {                   /* Device ID */
    N0000, L0000,               /* Unknown, Locked */
    S1200, S2313, S4414, S8515, S2333, S4433, S4434, S8535, S2323, S2343,
    T12, T13, T22, T25, T45, T85, T24, T44, T84, T26, T261, T461, T861, T2313, T15,
    M161, M162, M8515, M8535, M163, M323, M48, M48P, M88, M88P, M168, M168P, M328P,
    M8, M16, M32, M164P, M324P, M644P, M1284P, M644, M325, M3250, M165, M169, M603,
    M645, M6450, M103, M64, M128, M640, M1280, M1281, M2560, M2561, M325P, M3250P, M324PA,
    CAN32, CAN64, CAN128, PWM2, PWM216
};

#if 1                           /* by senshu */
//#define N0000       0x000000
//#define L0000       0x000102
#define AT90S1200   0x1E9001
#define TINY12      0x1E9005
#endif

typedef struct _DEVPROP {
    char *Name;                 /* Device name */
    char ID;                    /* Device ID */
    BYTE Sign[3];               /* Device signature bytes */
    DWORD FlashSize;            /* Flash memory size in unit of byte */
    WORD FlashPage;             /* Flash page size (0 is byte-by-byte) */
    DWORD EepromSize;           /* EEPROM size in unit of byte */
    WORD EepromPage;            /* EEPROM page size (0 is byte-by-byte) @@@ */
    WORD FlashWait;             /* Wait time for flash write */
    WORD EepromWait;            /* Wait time for EEPROM write */
    BYTE PollData;              /* Polling data value */
    BYTE LockData;              /* Default lock byte (program LB1 and LB2) */
    char FuseType;              /* Device specific fuse type */
    char Cals;                  /* Number of calibration bytes */
    BYTE FuseMask[3];           /* Valid fuse bit mask [low, high, ext] */
    WORD DocNumber;             /* http://www.avrfreaks.net/ @@@ by senshu */
    char *part_id;              /* Avrdude part's ID @@@ by senshu */
} DEVPROP;

#if 1
extern const DEVPROP DevLst[];  /* Device property list */
#endif

/* Device programming commands */

#define C_EN_PRG1	0xAC
#define C_EN_PRG2	0x53
#define C_ERASE1	0xAC
#define C_ERASE2	0x80
#define C_LD_ADRX	0x4D
#define C_WR_PRGL	0x40
#define C_WR_PRGH	0x48
#define C_WR_PAGE	0x4C
#define C_RD_PRGL	0x20
#define C_RD_PRGH	0x28
#define C_WR_EEP	0xC0
#define C_RD_EEP	0xA0
#define C_WR_FLB	0xAC
#define C_WR_FLBL	0xA0
#define C_WR_FLBH	0xA8
#define C_WR_FLBX	0xA4
#define C_WR_FLBK	0xE0
#define C_RD_FLB1	0x58
#define C_RD_FB1	0x50
#define C_RD_SIG	0x30
#define C_RD_CAL	0x38


/* Byte read/write ID */

#define FLASH		0
#define FLASH_NS	1
#define EEPROM		2
#define SIGNATURE	3


/* Fuse write ID */

#define F_LOW		0
#define	F_HIGH		1
#define	F_EXTEND	2
#define	F_LOCK		3


/* spi_rcvr() argument */

#define	RM_SYNC		0
#define	RM_ASYNC	1


/* Physical port properties */

typedef struct {
    WORD PortClass;             /* Port class */
    WORD PortNum;               /* Port number (1..)  */
#if 1
    int Baud;                   /* Baud rate (for SPI bridge) */
    int Delay;                  /* I/O delay */
    char *SerialNumber;         /*@@@ usbasp serial no. by t.k */
#else
    DWORD Baud;                 /* Baud rate (for SPI bridge) */
    WORD Delay;                 /* I/O delay */
#endif
    char *Info1, *Info2;        /* Information strings, returned by open_ifport() */
} PORTPROP;

#if 1
#define DEFAULT_DELAY_VALUE	3   //@@@ by t.k
#define DEFAULT_BAUDRATE	115200  //@@@ by t.k
#endif

enum _portclass {               /* Port class */
    TY_LPT, TY_COMM, TY_VCOM, TY_BRIDGE, TY_AVRSP,
    TY_STK200, TY_XILINX, TY_LATTICE, TY_ALTERA
#if 1
        , TY_USBASP, TY_RSCR, TY_HIDASP
#endif
};




/* Prototypes for hardware control functions */

int open_ifport(PORTPROP *);
void close_ifport();
void spi_reset();
void spi_clk();
void spi_xmit(BYTE);
BYTE spi_rcvr(BYTE);
void spi_delayedget(BYTE *, DWORD);
void delay_ms(WORD);
int spi_flush();
FILE *open_cfgfile(char *);
void read_multi(BYTE, DWORD, DWORD, BYTE *);
void write_page(DWORD, const BYTE *);

extern const DEVPROP *Device;

//----------------------------------------------------------------------------
// by t.k
// avrspx.c

// hwctrl.c
void spi_transmit(BYTE cmd1, BYTE cmd2, BYTE cmd3, BYTE cmd4);
int spi_transmit_R(BYTE cmd1, BYTE cmd2, BYTE cmd3, BYTE mode);

// utils.c
extern char progname[];
extern char inifilename[];
#undef INIFILE
#define INIFILE inifilename

unsigned long strtoul_ex(const char *s, char **endptr, int base);
int setup_commands_ex(int argc, char **argv);
DEVPROP *search_device(char *Name);
DEVPROP *get_device(BYTE * signature);

#ifdef USE_COMPORT
int get_outque_count(HANDLE hComm);
void dump_port_list(void);
#endif

void report_update(int bytes);

//----------------------------------------------------------------------------
#endif                          // end of __AVRSPX_H
