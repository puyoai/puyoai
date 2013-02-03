#include<stdio.h>

#include"hidasp.h"
main(){
  if (UsbInit(verbose_mode, 0, usb_serial) == 0) {
    exit(1);
  }
  hidInit();
}
