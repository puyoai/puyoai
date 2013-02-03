#
#	Windows/Linux兼用です.
#
#	コンパイラは minGw gccを使用します。
#	試してはいませんがcygwinでも大丈夫なはずです。
#	DEBUGの時には、-gをつけ、-sを外します。

include ./dos.mak
#EXE_SUFFIX = .exe

CC = gcc
USBFLAGS = -DHIDMON=1
#CFLAGS	= $(DEFINES) $(USBFLAGS) -O2 -Wall
CFLAGS	= $(DEFINES) $(USBFLAGS) -g -O2 -Wall
LIBS	= $(USBLIBS) -lreadline

TARG = hidmon

 ifdef DOS
OBJS = monit.o util.o hidasp-dos.o gr.o portlist.o icon.o
 else
OBJS = monit.o util.o hidasp.o gr-dmy.o portlist.o
OBJS2 =  my_monit.o util.o hidasp.o gr-dmy.o portlist.o wiiController.o
 endif

PROGRAM = $(TARG)$(EXE_SUFFIX)
PROGRAM2 = puyo$(EXE_SUFFIX)

all: $(PROGRAM) $(PROGRAM2)

.c.o:
	$(CC) $(CFLAGS) -c $<

icon.o:	icon.rc
	$(RC) -i icon.rc -o icon.o

$(PROGRAM): $(OBJS)
#	$(CC) -g -o $(PROGRAM) $(OBJS) $(SYS_LIB) $(LIBS)
	$(CC) -s -o $(PROGRAM) $(OBJS) $(SYS_LIB) $(LIBS)

$(PROGRAM2): $(OBJS2)
#	$(CC) -g -o $(PROGRAM2) $(OBJS2) $(SYS_LIB) $(LIBS)
	$(CC) -s -o $(PROGRAM2) $(OBJS2) $(SYS_LIB) $(LIBS)


strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f *~ *.o *.BAK $(PROGRAM) $(PROGRAM2)
 ifndef DOS
	chmod 644 *.c *.h Makefile *.mak
 endif

install:
 ifdef DOS
	install -c $(PROGRAM) ../bin
 else
	install -c $(PROGRAM) /usr/local/bin
 endif
#
