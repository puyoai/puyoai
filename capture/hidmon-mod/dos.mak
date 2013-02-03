# ==========================================================
#       MS-DOS (Windowsを含む) 環境かどうかチェック.
#    環境変数 comspecが定義されていたらDOSだと判定する.
# ==========================================================
#       もっと良いやりかたがあったら教えてください.
#
 ifdef ComSpec
DOS = 1
 else

 ifdef COMSPEC
DOS = 1
 else
 endif

 endif

#
#  === OS環境依存の定義 ====================================
#
 ifdef DOS
#===============================
#  Windows
#===============================
EXE_SUFFIX = .exe
RC = windres
SYS_LIB= -lkernel32 -luser32 -lgdi32 -lsetupapi
DEFINES=-D_DOS_
#===============================
 else
#===============================
#  Linux
#===============================
EXE_SUFFIX =
RC = echo RC.EXE
SYS_LIB= -lusb
DEFINES=-D_LINUX_
#===============================
 endif
#
