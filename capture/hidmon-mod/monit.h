#ifndef	_include_monit_h_
#define	_include_monit_h_

typedef unsigned char uchar;

typedef struct {
    uchar report_id;
    uchar cmd;
    uchar size;                 // bit7:6 をarenaに使用する予定.
    uchar adr[2];
    uchar data[48];
} cmdBuf;

//
//  cmd.
//
enum {
    CMD_PING = 1,
    CMD_SETPAGE = 0x14,
    CMD_POKE = 62,
    CMD_PEEK = 63,
};

//
//  area.
//
enum {
    AREA_RAM = 0,
    AREA_EEPROM = 0x20,
    AREA_PGMEM = 0x40,
//  reserved      0x60
    AREA_MASK = 0x60,
};


typedef struct {
    char *name;
    void (*func) ();
} CmdList;


/*
 *	オプション文字列チェック
 */
char *opt[128];                 /* オプション文字が指定されていたら、
                                   その文字に続く文字列を格納、
                                   指定がなければNULLポインタを格納   */

/*
 *	オプション文字列チェック
 *		optstring に含まれるオプション文字は、
 *				  後続パラメータ必須とみなす。
 */
#define Getopt(argc,argv,optstring)           		\
 {int i;int c;for(i=0;i<128;i++) opt[i]=NULL; 		\
   while( ( argc>1 )&&( *argv[1]=='-') ) {    		\
	 c = argv[1][1] & 0x7f;   			\
       opt[c] = &argv[1][2] ; 				\
       if(( *opt[c] ==0 )&&(strchr(optstring,c))) {	\
         opt[c] = argv[2] ;argc--;argv++;          	\
       }                        			\
     argc--;argv++;           				\
 } }

#if	0
#define Getopt(argc,argv)  \
 {int i;for(i=0;i<128;i++) opt[i]=NULL; \
   while( ( argc>1 )&&( *argv[1]=='-') ) \
    { opt[ argv[1][1] & 0x7f ] = &argv[1][2] ; argc--;argv++; } \
 }
#endif

#define IsOpt(c) ((opt[ c & 0x7f ])!=NULL)
#define   Opt(c)   opt[ c & 0x7f ]

#define Ropen(name) {ifp=fopen(name,"rb");if(ifp==NULL) \
{ printf("Fatal: can't open file:%s\n",name);exit(1);}  \
}

#define Wopen(name) {ofp=fopen(name,"wb");if(ofp==NULL) \
{ printf("Fatal: can't create file:%s\n",name);exit(1);}  \
}

#define Read(buf,siz)   fread (buf,1,siz,ifp)
#define Write(buf,siz)  fwrite(buf,1,siz,ofp)
#define Rclose()  fclose(ifp)
#define Wclose()  fclose(ofp)


#define	ZZ	printf("%s:%d: ZZ\n",__FILE__,__LINE__);

/* monit.c */
char *sp_skip(char *buf);
void Term_Log(int c);
int calc_ypos(int i);

void chop_crlf(char *p);
void cmdBench(char *buf);
void cmdDump(char *buf);
void cmdEdit(char *buf);
void cmdFill(char *buf);
void cmdGraph(char *buf);
void cmdHelp(char *buf);
void cmdMonit(char *buf);
void cmdPoll(char *buf);
void cmdPort(char *buf);
void cmdPortPrintAllCr(int count, unsigned char *pinbuf);
void cmdPortPrintOne(char *name, int adrs, int val);
void cmdPortPrintOneCr(int count, char *name, int adrs, int val);
void cmdPortPrintOne_b(char *name, int adrs, int val, int mask);
void cmdQuestion(char *buf);
void cmdQuit(char *buf);
void cmdSleep(char *buf);
void draw_PortName(int adr);
void draw_sample(int x, int y, int val);

int get_arg(char *buf);
int is_space(int c);
void plot_signal(int x, int y, int v, int diff);
int radix2scanf(char *s, int *p);
void scan_args(int arg_cnt);
int str_comp(char *buf, char *cmd);
void usage(void);

#endif                          //_include_monit_h_
