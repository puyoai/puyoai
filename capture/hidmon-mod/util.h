/* util.c */
void memdump_print(void *ptr, int len, int off);
int QueryAVR(cmdBuf * cmd, uchar * buf, int reply_len);
int dumpmem(int adr, int arena, int size, unsigned char *buf);
int pokemem(int adr, int arena, int data0, int data1);
int hid_ping(int i);
void UsbBench(int cnt, int psize);
void UsbDump(int adr, int arena, int cnt);
int UsbRead(int adr, int arena, uchar * buf, int size);
int UsbPeek(int adr, int arena);
int UsbSetPoll_slow(int adr, int arena);
int UsbPoll_slow(void);
int UsbSetPoll(int adr, int arena);
int UsbPoll(void);
int UsbPoke(int adr, int arena, int data, int mask);
void UsbPoke_b(int adr, int arena, int bit, int mask);
void UsbCheckPollCmd(void);
int UsbInit(int verbose, int enable_bulk, char *serial);
int UsbExit(void);
