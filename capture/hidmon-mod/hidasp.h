/* hidasp.h */
#define HIDASP_RST_H_GREEN	0x18
#define HIDASP_RST_H_RED	0x14
#define HIDASP_RST_L_GREEN	0x08
#define HIDASP_RST_L_BOTH	0x00
#define HIDASP_SCK_PULSE 	0x80    // RST-L SLK-pulse ,LED ON  @@kuga

#define HIDASP_ISP_MODE 	1   // Add by senshu
#define HIDASP_USB_IO_MODE 	2
#define HIDASP_FUSION_OK 	4
#define HIDASP_MODE_ERROR 	128

/* hidasp.c */
int hidCommand(int cmd, int arg1, int arg2, int arg3);
int hidPeekMem(int addr);
int hidPokeMem(int addr, int data0, int mask);
int hidReadBuffer(char *buf, int len);
int hidReadPoll(char *buf, int Length, int id);
int hidWriteBuffer(char *buf, int len);
void hidasp_close(void);
int hidasp_cmd(const unsigned char cmd[4], unsigned char res[4]);
int hidasp_init(const char *serial);
int hidasp_list(char *string);
int hidasp_page_read(long addr, unsigned char *wd, int pagesize);
int hidasp_page_write(long addr, const unsigned char *wd, int pagesize,
                      int flashsize);
int hidasp_page_write_fast(long addr, const unsigned char *wd,
                           int pagesize);
int hidasp_program_enable(int delay);
//void memdump (char *msg, char *buf, int len);

/*

 //以下は #include "../firmware/hidcmd.h" にて 定義.

#define HIDASP_TEST           0x01
#define HIDASP_SET_STATUS     0x02
#define HIDASP_CMD_TX         0x10
#define HIDASP_CMD_TX_1       0x11
#define HIDASP_SET_PAGE       0x14
#define HIDASP_PAGE_TX        0x20
#define HIDASP_PAGE_RD        0x21
#define HIDASP_PAGE_TX_START  0x22
#define HIDASP_PAGE_TX_FLUSH  0x24
#define HIDASP_SET_DELAY      0x3c
 */
