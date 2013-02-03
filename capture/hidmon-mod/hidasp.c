/* hidasp.c
 * original: binzume.net
 * modify: senshu , iruka
 * 2008-09-22 : for HIDaspx.
 * 2008-10-13 : hidspx-1012b (binzume)
 * 2009-02-01 : hidmon-2009-0201 (senshu)
 * 2009-02-03 : SerialNumberチェックを追加 (senshu)
 */

#define DEBUG 		   	0       // for DEBUG

#define DEBUG_PKTDUMP  	0       // HID Reportパケットをダンプする.
#define DUMP_PRODUCT   	0       // 製造者,製品名を表示.

#define CHECK_COUNT		2       // 4: Connect時の Ping test の回数.

#ifndef WIN32
#define LIBUSB                  // USE libusb  http://libusb.sourceforge.net/
#endif


#include <stdio.h>
#include <string.h>
#include "avrspx.h"
#include "hidasp.h"
#include "hidcmd.h"


#ifdef LIBUSB
#	include <usb.h>
#else
#	include <windows.h>
#	include "usbhid.h"
#	ifndef __GNUC__
#		pragma comment(lib, "setupapi.lib")
#	endif
#endif


//  obdev
#define MY_VID		0x16c0      /* 5824 in dec, stands for VOTI */
#define MY_PID_libusb	0x05dc  /* libusb:0x05dc, obdev's free PID */
#define MY_PID 		0x05df      /* HID: 0x05df, obdev's free PID */

#define	MY_Manufacturer	"YCIT"
#define	MY_Product		"HIDaspx"
//  MY_Manufacturer,MY_Product のdefine を外すと、VID,PIDのみの照合になる.
//  どちらかをはずすと、その照合を省略するようになる.

static int dev_id = 0;          // ターゲットID: 0x55 もしくは 0x5a だけを許容.
static int have_ledcmd = 0;     // LED制御の有無.

#if	DEBUG_PKTDUMP
static void memdump(char *msg, char *buf, int len);
#endif

//----------------------------------------------------------------------------
//--------------------------    Tables    ------------------------------------
//----------------------------------------------------------------------------
//  HID Report のパケットはサイズ毎に ３種類用意されている.
#define	REPORT_ID1              1   // 8  REPORT_COUNT(6)
#define	REPORT_ID2             	2   // 32 REPORT_COUNT(30)
#define	REPORT_ID3             	3   // 40 REPORT_COUNT(38)

#define	REPORT_LENGTH1          7   // 8  REPORT_COUNT(6)
#define	REPORT_LENGTH2          31  // 32 REPORT_COUNT(30)
#define	REPORT_LENGTH3          39  // 40 REPORT_COUNT(38)

#define	PAGE_WRITE_LENGTH       32  // Page Writeでは32byte単位の転送を心掛ける.
                                // Length5より7バイト少ない値である必要がある.

//  最大の長さをもつ HID ReportID,Length
#define	REPORT_IDMAX		REPORT_ID3
#define	REPORT_LENGTHMAX	REPORT_LENGTH3

#if _LINUX_
static int hidRead(usb_dev_handle * h, unsigned char *buf, int Length, int id);
static int hidWrite(usb_dev_handle * h, unsigned char *buf, int Length, int id);
static int hidOpen(int my_pid, const char *serial);
#else

//----------------------------------------------------------------------------
/*
 * 	unicode を ASCIIに変換.
 */
static char *uni_to_string(char *t, unsigned short *u)
{
    char *buf = t;
    int c;
    // short を char 配列に入れなおす.
    while (1) {
        c = *u++;
        if (c == 0)
            break;
        if (c & 0xff00)
            c = '?';
        *t++ = c;
    }

    *t = 0;
    return buf;
}
#endif



#ifdef LIBUSB                   ///////////////////////////////////////////////////////////////

typedef struct usbDevice usbDevice_t;
#define USBRQ_HID_GET_REPORT    0x01
#define USBRQ_HID_SET_REPORT    0x09
#define USB_HID_REPORT_TYPE_FEATURE 3
//static int  usesReportIDs;

usb_dev_handle *hHID = NULL;    // USB-IO dev handle
struct usb_device *gdev;        // @@@ add by tanioka


static int hidInit()
{
    usb_init();
    return 1;
}

static void hidFree()
{
}


static int usbhidGetStringAscii(usb_dev_handle * dev, int index, char *buf, int buflen)
{
    unsigned char buffer[256];
    int rval, i;

    if ((rval = usb_get_string_simple(dev, index, buf, buflen)) >= 0)   /* use libusb version if it works */
        return rval;

    rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
                           (USB_DT_STRING << 8) + index, 0x0409,
                           (char *) buffer, sizeof(buffer), 5000);

    if (rval < 0)
        return rval;
    if (buffer[1] != USB_DT_STRING) {
        *buf = 0;
        return 0;
    }
    if ((unsigned char) buffer[0] < rval)
        rval = (unsigned char) buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1: */
    for (i = 1; i < rval; i++) {
        if (i > buflen)         /* destination buffer overflow */
            break;
        buf[i - 1] = buffer[2 * i];
        if (buffer[2 * i + 1] != 0) /* outside of ISO Latin1 range */
            buf[i - 1] = '?';
    }
    buf[i - 1] = 0;
    return i - 1;
}

static int hidOpen(int my_pid, const char * serial)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_dev_handle *handle = NULL;
    int errorCode = 0;

    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            gdev = dev;         /* @@@ add by tanioka */
#if DEBUG
            printf("dev=%04x:%04x\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
#endif
            if (dev->descriptor.idVendor == MY_VID && dev->descriptor.idProduct == my_pid) {
                char string[256];
                int len;
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if (!handle) {
                    errorCode = -1;
                    fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }
                if (usb_set_configuration
                    (handle, dev->config->bConfigurationValue) < 0) {
                    fprintf(stderr, "!!usb_set_configuration Error.\n");
                }
#if DEBUG
                printf("!!usb_set_configuration.\n");
#endif

                if (usb_claim_interface
                    (handle,
                     dev->config->interface->
                     altsetting->bInterfaceNumber) < 0) {
                    fprintf(stderr, "!!usb_claim_interface Error.\n");
                }
#if DEBUG
                printf("usb_claim_interface.\n");
#endif

                /* now check whether the names match: */
                len = usbhidGetStringAscii(handle, dev->descriptor.iManufacturer, string, sizeof(string));
                errorCode = -1;
                if (len < 0) {
                    fprintf(stderr, "Warning: cannot query manufacturer for device: %s\n", usb_strerror());
                } else {
#if DEBUG
                    printf("seen device from vendor [%s]\n", string);
#endif
                    if (strcmp(string, MY_Manufacturer) == 0) {
                        len = usbhidGetStringAscii(handle, dev->descriptor.iProduct, string, sizeof(string));
                        if (len < 0) {
                            fprintf(stderr, "Warning: cannot query product for device: %s\n", usb_strerror());
                        } else {
#if DEBUG
                            fprintf(stderr, "seen product [%s]\n", string);
#endif
                            if (strcmp(string, MY_Product) == 0) {
                               len = usbhidGetStringAscii(handle, dev->descriptor.iSerialNumber, string, sizeof(string));
                               if (len < 0) {
                                  fprintf(stderr, "Warning: cannot query serialnumbert for device: %s\n", usb_strerror());
                               } else {
                                 if (strcmp(string, serial) == 0) {
#if DEBUG
                                     fprintf(stderr, "seen SerialNumber [%s]\n", string);
#endif
                                     break;
                                  }
                               }
                            }
                        }
                    }
                }
                usb_close(handle);
                handle = NULL;
            }
        }
        if (hHID) {
            break;
        }
    }
    if (handle != NULL) {
        errorCode = 1;
        hHID = handle;
        //usesReportIDs = 3;
    } else {
        errorCode = 0;
    }
#if _LINUX_
    setuid(getuid());           // @@@ by iruka
#endif
    return errorCode;
}

static void hidClose()
{
    if (hHID) {
        usb_release_interface((void *) hHID, gdev->config->interface->altsetting->bInterfaceNumber);
//      usb_reset((void*)hHID);
        usb_close((void *) hHID);
    }
    hHID = NULL;
#if DEBUG
    printf("usb_close.\n");
#endif
}

static int hidRead(usb_dev_handle * h, unsigned char *buf, int Length,
                   int id)
{
    int rc;

    buf[0] = id;
/*
int usb_control_msg(usb_dev_handle *dev, int requesttype,
	int request, int value, int index, char *bytes, int size, int timeout);
 */
    rc = usb_control_msg(hHID,
                         USB_TYPE_CLASS | USB_RECIP_INTERFACE |
                         USB_ENDPOINT_IN, USBRQ_HID_GET_REPORT,
                         USB_HID_REPORT_TYPE_FEATURE << 8 | id, 0,
                         (char *) buf, Length, 5000);

    if (rc < Length) {
        fprintf(stderr, "Error hidRead(): %s (rc=%d), %d\n",
                usb_strerror(), rc, Length);
        return 0;
    }
    return rc + 1;
}

static int hidWrite(usb_dev_handle * h, unsigned char *buf, int Length,
                    int id)
{
    int rc;

    buf[0] = id;

    if (h == NULL) {
        fprintf(stderr, "hidWrite(%p %p)\n", h, buf);
        return 0;
    }
    rc = usb_control_msg(hHID,
                         USB_TYPE_CLASS | USB_RECIP_INTERFACE |
                         USB_ENDPOINT_OUT, USBRQ_HID_SET_REPORT,
                         USB_HID_REPORT_TYPE_FEATURE << 8 | (id & 0xff), 0,
                         (char *) buf, Length, 5000);

    if (rc < Length) {
        fprintf(stderr, "Error hidWrite(): %s (rc=%d), %d \n",
                usb_strerror(), rc, Length);
        return 0;
    }
    return rc;
}


#else                           // For Windows //////////////////////////////////////////////////////////////

// HID API (from w2k DDK)
_HidD_GetAttributes HidD_GetAttributes;
_HidD_GetHidGuid HidD_GetHidGuid;
_HidD_GetPreparsedData HidD_GetPreparsedData;
_HidD_FreePreparsedData HidD_FreePreparsedData;
_HidP_GetCaps HidP_GetCaps;
_HidP_GetValueCaps HidP_GetValueCaps;
_HidD_GetFeature HidD_GetFeature;
_HidD_SetFeature HidD_SetFeature;
_HidD_GetManufacturerString HidD_GetManufacturerString;
_HidD_GetProductString HidD_GetProductString;

HINSTANCE hHID_DLL = NULL;      // hid.dll handle
HANDLE hHID = NULL;             // USB-IO dev handle
HIDP_CAPS Caps;
static int check_product_string(HANDLE handle);


/*
 * wrapper for HidD_GetFeature / HidD_SetFeature.
 */
//----------------------------------------------------------------------------
/*
 *	HIDデバイスから HID Report を取得する.
 *	受け取ったバッファは先頭の１バイトに必ずReportIDが入っている.
 *
 *	id と Length の組はデバイス側で定義されたものでなければならない.
 *
 *	戻り値はHidD_GetFeatureの戻り値( 0 = 失敗 )
 *
 */
static int hidRead(HANDLE h, char *buf, int Length, int id)
{
    int rc;
    buf[0] = id;
    rc = HidD_GetFeature(h, buf, Length);
#if	DEBUG_PKTDUMP
    memdump("RD", buf, Length);
#endif
    return rc;
}

/*
 *	HIDデバイスに HID Report を送信するする.
 *	送信バッファの先頭の１バイトにReportID を入れる処理は
 *	この関数内で行うので、先頭１バイトを予約しておくこと.
 *
 *	id と Length の組はデバイス側で定義されたものでなければならない.
 *
 *	戻り値はHidD_SetFeatureの戻り値( 0 = 失敗 )
 *
 */
static int hidWrite(HANDLE h, char *buf, int Length, int id)
{
    int rc;
    buf[0] = id;
    rc = HidD_SetFeature(h, buf, Length);
#if	DEBUG_PKTDUMP
    memdump("WR", buf, Length);
#endif
    return rc;
}

////////////////////////////////////////////////////////////////////////
//             hid.dll をロード
static int hidInit()
{
    hHID_DLL = LoadLibrary("hid.dll");
    if (!hHID_DLL) {
        fprintf(stderr, "Error at Load 'hid.dll'\n");
        return 0;
    }
    HidD_GetAttributes = (_HidD_GetAttributes) GetProcAddress(hHID_DLL, "HidD_GetAttributes");
    if (!HidD_GetAttributes) {
        fprintf(stderr, "Error at HidD_GetAttributes\n");
        return 0;
    }
    HidD_GetHidGuid = (_HidD_GetHidGuid) GetProcAddress(hHID_DLL, "HidD_GetHidGuid");
    if (!HidD_GetHidGuid) {
        fprintf(stderr, "Error at HidD_GetHidGuid\n");
        return 0;
    }
    HidD_GetPreparsedData = (_HidD_GetPreparsedData) GetProcAddress(hHID_DLL, "HidD_GetPreparsedData");
    HidD_FreePreparsedData = (_HidD_FreePreparsedData) GetProcAddress(hHID_DLL, "HidD_FreePreparsedData");
    HidP_GetCaps =  (_HidP_GetCaps) GetProcAddress(hHID_DLL, "HidP_GetCaps");
    HidP_GetValueCaps = (_HidP_GetValueCaps) GetProcAddress(hHID_DLL, "HidP_GetValueCaps");

//
    HidD_GetFeature = (_HidD_GetFeature) GetProcAddress(hHID_DLL, "HidD_GetFeature");
    HidD_SetFeature = (_HidD_SetFeature) GetProcAddress(hHID_DLL, "HidD_SetFeature");
    HidD_GetManufacturerString = (_HidD_GetManufacturerString) GetProcAddress(hHID_DLL, "HidD_GetManufacturerString");
    HidD_GetProductString = (_HidD_GetProductString) GetProcAddress(hHID_DLL, "HidD_GetProductString");

#if	DEBUG
    printf("_HidD_GetFeature= %x\n", (int) HidD_GetFeature);
    printf("_HidD_SetFeature= %x\n", (int) HidD_SetFeature);
#endif
    return 1;
}

void hidFree()
{
    if (hHID_DLL)
        FreeLibrary(hHID_DLL);
    hHID_DLL = NULL;
}

////////////////////////////////////////////////////////////////////////
// HIDディバイス一覧からUSB-IOを検索 (for Windows)
static int hidOpen(int my_pid, char *serial)
{
    int f = 0;
    int i = 0;
    ULONG Needed, l;
    GUID HidGuid;
    HDEVINFO DeviceInfoSet;
    HIDD_ATTRIBUTES DeviceAttributes;
    SP_DEVICE_INTERFACE_DATA DevData;
    PSP_INTERFACE_DEVICE_DETAIL_DATA DevDetail;
    //SP_DEVICE_INTERFACE_DETAIL_DATA *MyDeviceInterfaceDetailData;

    DeviceAttributes.Size = sizeof(HIDD_ATTRIBUTES);
    DevData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    HidD_GetHidGuid(&HidGuid);

    DeviceInfoSet = SetupDiGetClassDevs(&HidGuid, NULL, NULL,
                            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    while (SetupDiEnumDeviceInterfaces(DeviceInfoSet, 0, &HidGuid, i++, &DevData)) {
        SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DevData, NULL, 0, &Needed, 0);
        l = Needed;
        DevDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA *) GlobalAlloc(GPTR, l + 4);
        DevDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
        SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DevData, DevDetail, l, &Needed, 0);

        hHID = CreateFile(DevDetail->DevicePath,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL, OPEN_EXISTING,
//                        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
                          0, NULL);

        GlobalFree(DevDetail);

        if (hHID == INVALID_HANDLE_VALUE)   // Can't open a device
            continue;
        HidD_GetAttributes(hHID, &DeviceAttributes);

        // HIDaspかどうか調べる.
        if (DeviceAttributes.VendorID == MY_VID  && DeviceAttributes.ProductID == my_pid) {
            int rc; dev->descriptor.iSerialNumber
            rc = check_product_string(hHID, serial);
            if (rc == 1) {
              f = 1;              // 発見された.
              break;
            }
        } else {
            // 違ったら閉じる
            CloseHandle(hHID);
            hHID = NULL;
        }
    }
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    return f;
}

void hidClose()
{
    if (hHID)
        CloseHandle(hHID);
    hHID = NULL;
}

////////////////////////////////////////////////////////////////////////
// ディバイスの情報を取得
static void GetDevCaps()
{
    PHIDP_PREPARSED_DATA PreparsedData;
    HIDP_VALUE_CAPS *VCaps;
    char buf[1024];

    VCaps = (HIDP_VALUE_CAPS *) (&buf);

    HidD_GetPreparsedData(hHID, &PreparsedData);
    HidP_GetCaps(PreparsedData, &Caps);
    HidP_GetValueCaps(HidP_Input, VCaps, &Caps.NumberInputValueCaps,
                      PreparsedData);
    HidD_FreePreparsedData(PreparsedData);

#if DEBUG
    fprintf(stderr, " Caps.OutputReportByteLength = %d\n",
            Caps.OutputReportByteLength);
    fprintf(stderr, " Caps.InputReportByteLength = %d\n",
            Caps.InputReportByteLength);
#endif


#if	REPORT_LENGTH_OVERRIDE
    //複数のREPORT_COUNTが存在するときは下記Capsが 0 のままなので、上書きする.
    Caps.OutputReportByteLength = REPORT_LENGTH_OVERRIDE;
    Caps.InputReportByteLength = REPORT_LENGTH_OVERRIDE;
#endif
}

//----------------------------------------------------------------------------
/*  Manufacturer & Product name check.
 *  名前チェック : 成功=1  失敗=0 読み取り不能=(-1)
 */
static int check_product_string(HANDLE handle)
{
    unsigned short unicode[512];
    char string1[256];
    char string2[256];

    if (!HidD_GetManufacturerString(handle, unicode, sizeof(unicode))) {
        return -1;
    }
    uni_to_string(string1, unicode);

    if (!HidD_GetProductString(handle, unicode, sizeof(unicode))) {
        return -1;
    }
    uni_to_string(string2, unicode);

#if	DUMP_PRODUCT
    fprintf(stderr, "iManufacturer:%s\n", string1);
    fprintf(stderr, "iProduct:%s\n", string2);
#endif

#ifdef	MY_Manufacturer
    if (strcmp(string1, MY_Manufacturer) != 0)
        return 0;
#endif

#ifdef	MY_Product
    if (strcmp(string2, MY_Product) != 0)
        return 0;
#endif

    return 1;                   //合致した.
}

#endif                          // LIBUSB

/*
 *	hidWrite()を使用して、デバイス側に buf[] データを len バイト送る.
 *	長さを自動判別して、ReportIDも自動選択する.
 *
 *	戻り値はHidD_SetFeatureの戻り値( 0 = 失敗 )
 *
 */
int hidWriteBuffer(char *buf, int len)
{
    int report_id = 0;
    int length = 0;

    if (len <= REPORT_LENGTH1) {
        length = REPORT_LENGTH1;
        report_id = REPORT_ID1;
    } else if (len <= REPORT_LENGTH2) {
        length = REPORT_LENGTH2;
        report_id = REPORT_ID2;
    } else if (len <= REPORT_LENGTH3) {
        length = REPORT_LENGTH3;
        report_id = REPORT_ID3;
    }

    if (report_id == 0) {
        // 適切な長さが選択できなかった.
        fprintf(stderr, "Error at hidWriteBuffer. len=%d\n", len);
        exit(1);
        return 0;
    }

    return hidWrite(hHID, (unsigned char *) buf, length, report_id);
}

/*
 *	hidRead()を使用して、デバイス側から buf[] データを len バイト取得する.
 *	長さを自動判別して、ReportIDも自動選択する.
 *
 *	戻り値はHidD_GetFeatureの戻り値( 0 = 失敗 )
 *
 */
int hidReadBuffer(char *buf, int len)
{
    int report_id = 0;
    int length = 0;

    if (len <= REPORT_LENGTH1) {
        length = REPORT_LENGTH1;
        report_id = REPORT_ID1;
    } else if (len <= REPORT_LENGTH2) {
        length = REPORT_LENGTH2;
        report_id = REPORT_ID2;
    } else if (len <= REPORT_LENGTH3) {
        length = REPORT_LENGTH3;
        report_id = REPORT_ID3;
    }

    if (report_id == 0) {
        // 適切な長さが選択できなかった.
        fprintf(stderr, "Error at hidWriteBuffer. len=%d\n", len);
        exit(1);
        return 0;
    }

    return hidRead(hHID, (unsigned char *) buf, length, report_id);
}

/*
 *	hidWrite()を使用して、デバイス側に4バイトの情報を送る.
 *	4バイトの内訳は cmd , arg1 , arg2 , arg 3 である.
 *  ReportIDはID1を使用する.
 *
 *	戻り値はHidD_SetFeatureの戻り値( 0 = 失敗 )
 *
 */
int hidCommand(int cmd, int arg1, int arg2, int arg3)
{
    unsigned char buf[128];

    memset(buf, 0, sizeof(buf));

    buf[1] = cmd;
    buf[2] = arg1;
    buf[3] = arg2;
    buf[4] = arg3;

    return hidWrite(hHID, buf, REPORT_LENGTH1, REPORT_ID1);
}

//
//  mask が   0 の場合は、 addr に data0 を1バイト書き込み.
//  mask が 非0 の場合は、 addr に data0 と mask の論理積を書き込む.
//      但し、その場合は mask bitが 0 になっている部分に影響を与えないようにする.
//
//  例:    PORTB の 8bit に dataを書き込む.
//      hidPokeMem( PORTB , data , 0 );
//  例:    PORTB の bit2 だけを on
//      hidPokeMem( PORTB , 1<<2 , 1<<2 );
//  例:    PORTB の bit2 だけを off
//      hidPokeMem( PORTB ,    0 , 1<<2 );
//
int hidPokeMem(int addr, int data0, int mask)
{
    unsigned char buf[128];
    memset(buf, 0, sizeof(buf));

    buf[1] = HIDASP_POKE;
    buf[2] = 0;
    buf[3] = addr;
    buf[4] = (addr >> 8);
    if (mask) {
        buf[5] = data0 & mask;
        buf[6] = ~mask;
    } else {
        buf[5] = data0;
        buf[6] = 0;
    }
    return hidWrite(hHID, buf, REPORT_LENGTH1, REPORT_ID1);
}

int hidPeekMem(int addr)
{
    unsigned char buf[128];
    memset(buf, 0, sizeof(buf));

    buf[1] = HIDASP_PEEK;
    buf[2] = 1;
    buf[3] = addr;
    buf[4] = (addr >> 8);

    hidWrite(hHID, buf, REPORT_LENGTH1, REPORT_ID1);
    hidReadBuffer((char *) buf, REPORT_LENGTH1);
    return buf[1];
}

#define	USICR			0x2d    //
#define	DDRB			0x37    // PB4=RST PB3=LED
#define	DDRB_WR_MASK	0xf0    // 制御可能bit = 1111_0000
#define	PORTB			0x38    // PB4=RST PB3=LED
#define	PORTB_WR_MASK	0       // 0 の場合はMASK演算は省略され、直書き.

#define HIDASP_RST		0x10    // RST bit

/*
 *	LEDの制御.
#define HIDASP_RST_H_GREEN	0x18	// RST解除,LED OFF
#define HIDASP_RST_L_BOTH	0x00	// RST実行,LED ON
#define HIDASP_SCK_PULSE 	0x80	// RST-L SLK-pulse ,LED ON	@@kuga
 */


static void hidSetStatus(int ledstat)
{
    int ddrb;
    if (have_ledcmd) {
        hidCommand(HIDASP_SET_STATUS, 0, ledstat, 0);   // cmd,portd(&0000_0011),portb(&0001_1111),0
    } else {
        if (ledstat & HIDASP_RST) { // RST解除.
            ddrb = 0x10;        // PORTB 全入力.
            hidPokeMem(USICR, 0, 0);
            hidPokeMem(PORTB, ledstat, PORTB_WR_MASK);
            hidPokeMem(DDRB, ddrb, DDRB_WR_MASK);
        } else {
            // RSTをLにする.
            ddrb = 0xd0;        // DDRB 1101_1100 : 1が出力ピン ただしbit3-0は影響しない.
            hidPokeMem(USICR, 0, 0);
            hidPokeMem(DDRB, ddrb, DDRB_WR_MASK);
            hidPokeMem(PORTB, ledstat, PORTB_WR_MASK);
            hidPokeMem(PORTB, ledstat | HIDASP_RST, PORTB_WR_MASK); // RSTはまだHi
            hidPokeMem(PORTB, ledstat, PORTB_WR_MASK);  // RSTはあとでLに.
            hidPokeMem(USICR, 0x1a, 0);
        }
    }
}



#if	DEBUG_PKTDUMP
//----------------------------------------------------------------------------
//  メモリーダンプ.
void memdump(char *msg, char *buf, int len)
{
    int j;
    fprintf(stderr, "%s", msg);
    for (j = 0; j < len; j++) {
        fprintf(stderr, " %02x", buf[j] & 0xff);
        if ((j & 0x1f) == 31)
            fprintf(stderr, "\n +");
    }
    fprintf(stderr, "\n");
}
#endif


//----------------------------------------------------------------------------
//  初期化.
//----------------------------------------------------------------------------
int hidasp_init(const char *serial)
{
    extern bool f_hid_libusb;
    unsigned char rd_data[128];
    int i, r, pid;

    hidInit();
    if (f_hid_libusb) {
        pid = MY_PID_libusb;
    } else {
        pid = MY_PID;
    }
    if (hidOpen(pid, serial) == 0) {
#if DEBUG
        fprintf(stderr, "ERROR: fail to hidOpen()\n");
#endif
        return HIDASP_MODE_ERROR;
    }
//  GetDevCaps();
    Sleep(100);

#if DEBUG
    fprintf(stderr, "HIDaspx Connection check!\n");
#endif

    for (i = 0; i < CHECK_COUNT; i++) {
        hidCommand(HIDASP_TEST, i, 0, 0);   // Connection test
        r = hidRead(hHID, rd_data, REPORT_LENGTH1, REPORT_ID1);
#if DEBUG
        fprintf(stderr, "HIDasp Ping(%d) = HIDASP_TEST(%02x) %02x %02x\n",
                i, rd_data[0], rd_data[1], rd_data[2]);
#endif
        if (r == 0) {
            fprintf(stderr, "ERR(rc=0). fail to Read().\n");
            return  HIDASP_MODE_ERROR;
        }
        dev_id = rd_data[1];
        if ((dev_id != DEV_ID_FUSION) && (dev_id != DEV_ID_STD)) {
            fprintf(stderr, "ERR. fail to ping test. (id = %x)\n", dev_id);
            return  HIDASP_MODE_ERROR;
        }
        if (rd_data[2] != i) {
            fprintf(stderr, "ERR. fail to ping test. %d != %d\n",
                    rd_data[2], i);
            return  HIDASP_MODE_ERROR;
        }
    }
    hidCommand(HIDASP_SET_STATUS, 0, HIDASP_RST_H_GREEN, 0);    // RESET HIGH
    r = hidRead(hHID, rd_data, REPORT_LENGTH1, REPORT_ID1);
    if (rd_data[1] == 0xaa) {   // LEDコマンド(isp_enable)が正常動作した.
        have_ledcmd = 1;        // LED制御OK.
#if DEBUG
        fprintf(stderr, "LED OK.\n");
#endif
    } else {
#if DEBUG
        fprintf(stderr, "Don't have LED COMMAND.\n");
#endif
    }

#if DEBUG
    fprintf(stderr, "OK.\n");
#endif
    return 0;
}


#define HIDASP_NOP 			  0 //これは ISPのコマンドと思われる?

//----------------------------------------------------------------------------
//  終了.
//----------------------------------------------------------------------------
void hidasp_close()
{
    extern int hidmon_mode;

    if (hHID) {
        unsigned char buf[128];

        if (!hidmon_mode) {
          buf[0] = 0x00;
          buf[1] = HIDASP_NOP;
          buf[2] = 0x00;
          buf[3] = 0x00;
          hidasp_cmd(buf, NULL);  // AVOID BUG!
          hidSetStatus(HIDASP_RST_H_GREEN);   // RESET HIGH
        }
        hidClose();
    }
    hidFree();
}




//----------------------------------------------------------------------------
//  ターゲットマイコンをプログラムモードに移行する.
//----------------------------------------------------------------------------
int hidasp_program_enable(int delay)
{
    unsigned char buf[128];
    unsigned char res[4];
    int i, rc;
#if 1                           //AVRSP type protocole
    int tried;

    rc = 1;
    hidSetStatus(HIDASP_RST_H_GREEN);   // RESET HIGH
    hidSetStatus(HIDASP_RST_L_BOTH);    // RESET LOW (H PULSE)
    hidCommand(HIDASP_SET_DELAY, delay, 0, 0);  // SET_DELAY
    Sleep(30);                  // 30

    for (tried = 1; tried <= 3; tried++) {
        for (i = 1; i <= 32; i++) {

            buf[0] = 0xAC;
            buf[1] = 0x53;
            buf[2] = 0x00;
            buf[3] = 0x00;
            hidasp_cmd(buf, res);

            if (res[2] == 0x53) {
                rc = 0;
                goto hidasp_program_enable_exit;
            }
            if (tried <= 2) {
                break;
            }
            hidSetStatus(HIDASP_SCK_PULSE); // RESET LOW SCK H PULSE shift scan point
        }
    }
#else
    // ISPモード移行が失敗したら再試行するように修正 by senshu(2008-9-16)
    rc = 1;
    for (i = 0; i < 3; i++) {
        hidSetStatus(HIDASP_RST_H_GREEN);   // RESET HIGH
        Sleep(2);               // 10 => 100

        hidSetStatus(HIDASP_RST_L_BOTH);    // RESET LOW
        hidCommand(HIDASP_SET_DELAY, delay, 0, 0);  // SET_DELAY
        Sleep(30);              // 30

        buf[0] = 0xAC;
        buf[1] = 0x53;
        buf[2] = 0x00;
        buf[3] = 0x00;
        hidasp_cmd(buf, res);

        if (res[2] == 0x53) {
            /* AVRマイコンからの同期(ISPモード)が確認できた */
            rc = 0;
            break;
        } else {
            /* AVRマイコンからの同期が確認できないので再試行 */
            Sleep(50);
        }
    }
#endif
  hidasp_program_enable_exit:
#if DEBUG
    if (rc == 0) {
        fprintf(stderr, "hidasp_program_enable() == OK\n");
    } else {
        fprintf(stderr, "hidasp_program_enable() == NG\n");
    }
#endif
    return rc;
}



//----------------------------------------------------------------------------
//  ISPコマンド発行.
//----------------------------------------------------------------------------
int hidasp_cmd(const unsigned char cmd[4], unsigned char res[4])
{
    unsigned char buf[128];
    int r;

    memset(buf, 0, sizeof(buf));
    if (res != NULL) {
        buf[1] = HIDASP_CMD_TX_1;
    } else {
        buf[1] = HIDASP_CMD_TX;
    }
    memcpy(buf + 2, cmd, 4);

    r = hidWrite(hHID, buf, REPORT_LENGTH1, REPORT_ID1);
#if DEBUG
    fprintf(stderr, "hidasp_cmd %02X, cmd: %02X %02X %02X %02X ", buf[1],
            cmd[0], cmd[1], cmd[2], cmd[3]);
#endif

    if (res != NULL) {
        r = hidRead(hHID, buf, REPORT_LENGTH1, REPORT_ID1);
        memcpy(res, buf + 1, 4);
#if DEBUG
        fprintf(stderr, " --> res: %02X %02X %02X %02X\n", res[0], res[1],
                res[2], res[3]);
#endif
    }

    return 1;
}


static void hid_transmit(BYTE cmd1, BYTE cmd2, BYTE cmd3, BYTE cmd4)
{
    unsigned char cmd[4];

    cmd[0] = cmd1;
    cmd[1] = cmd2;
    cmd[2] = cmd3;
    cmd[3] = cmd4;
    hidasp_cmd(cmd, NULL);
}

//----------------------------------------------------------------------------
//  フュージョンサポートのページライト.
//----------------------------------------------------------------------------
int hidasp_page_write_fast(long addr, const unsigned char *wd,
                           int pagesize)
{
    int n, l;
    char buf[128];
    int cmd = HIDASP_PAGE_TX_START;

    memset(buf, 0, sizeof(buf));

    // Load the page data into page buffer
    n = 0;                      // n はPageBufferへの書き込み基点offset.
    while (n < pagesize) {
        l = PAGE_WRITE_LENGTH;  // 32バイトが最大データ長.
        if (pagesize - n < l) { // 残量が32バイト未満のときは len を残量に置き換える.
            l = pagesize - n;
        }
        buf[1] = cmd;           // HIDASP_PAGE_TX_* コマンド.
        buf[2] = l;             // l       書き込みデータ列の長さ.
        memcpy(buf + 3, wd + n, l); // data[l] 書き込みデータ列.

        if ((pagesize - n) == l) {
            buf[1] |= HIDASP_PAGE_TX_FLUSH; //最終page_writeではisp_commandを付加する.
            // ISP コマンド列.
            buf[3 + l + 0] = C_WR_PAGE;
            buf[3 + l + 1] = (BYTE) (addr >> 9);
            buf[3 + l + 2] = (BYTE) (addr >> 1);
            buf[3 + l + 3] = 0;
        }
        hidWriteBuffer(buf, REPORT_LENGTHMAX);

        n += l;
        cmd = HIDASP_PAGE_TX;   // cmd をノーマルのpage_writeに戻す.
    }


    return 0;
}

//----------------------------------------------------------------------------
//  ページライト.
//----------------------------------------------------------------------------
int hidasp_page_write(long addr, const unsigned char *wd, int pagesize,
                      int flashsize)
{
    int n, l;
    char buf[128];

    if ((dev_id == DEV_ID_FUSION) && (flashsize <= (128 * 1024))) {
//      ((addr & 0xFF) == 0     ) ) {
        // addres_set , page_write , isp_command が融合された高速版を実行.
        return hidasp_page_write_fast(addr, wd, pagesize);
    }
#if	0
    fprintf(stderr, "dev_id=%x flashsize=%x addr=%x\n",
            dev_id, flashsize, (int) addr);
#endif
    // set page
    hidCommand(HIDASP_SET_PAGE, 0x40, 0, (addr & 0xFF));    // Set Page mode , FlashWrite

    // Load the page data into page buffer
    n = 0;                      // n はPageBufferへの書き込み基点offset.
    while (n < pagesize) {
        l = PAGE_WRITE_LENGTH;  // MAX
        if (pagesize - n < l) {
            l = pagesize - n;
        }
        buf[0] = 0x00;
        buf[1] = HIDASP_PAGE_TX;    // PageBuf
        buf[2] = l;             // Len
        memcpy(buf + 3, wd + n, l);
        hidWriteBuffer(buf, 3 + l);

#if DEBUG
        fprintf(stderr, "  p: %02x %02x %02x %02x\n",
                buf[1] & 0xff, buf[2] & 0xff, buf[3] & 0xff,
                buf[4] & 0xff);
#endif
        n += l;
    }

    /* Load extended address if needed */
    if (flashsize > (128 * 1024)) {
        hid_transmit(C_LD_ADRX, 0, (BYTE) (addr >> 17), 0);
    }

    /* Start page programming */
    hid_transmit(C_WR_PAGE, (BYTE) (addr >> 9), (BYTE) (addr >> 1), 0);

    return 0;
}

//----------------------------------------------------------------------------
//  ページリード.
//----------------------------------------------------------------------------
int hidasp_page_read(long addr, unsigned char *wd, int pagesize)
{
    int n, l;
    unsigned char buf[128];

    // set page
    if (addr >= 0) {
        hidCommand(HIDASP_SET_PAGE, 0x20, 0, 0);    // Set Page mode , FlashRead
    }
    // Load the page data into page buffer
    n = 0;                      // n は読み込み基点offset.
    while (n < pagesize) {
        l = REPORT_LENGTHMAX - 1;   // MAX
        if (pagesize - n < l) {
            l = pagesize - n;
        }
        hidCommand(HIDASP_PAGE_RD, l, 0, 0);    // PageRead , length

        memset(buf + 3, 0, l);
        hidRead(hHID, buf, REPORT_LENGTHMAX, REPORT_IDMAX);
        memcpy(wd + n, buf + 1, l); // Copy result.

#if 1
        report_update(l);
#else
        if (n % 128 == 0) {
            report_update(128);
        }
#endif

#if DEBUG
        fprintf(stderr, "  p: %02x %02x %02x %02x\n",
                buf[1] & 0xff, buf[2] & 0xff, buf[3] & 0xff,
                buf[4] & 0xff);
#endif
        n += l;
    }

    return 0;
}

//----------------------------------------------------------------------------

//  以下、Linux版：未実装.

int hidReadPoll(char *buf, int Length, int id)
{
    return 0;
}
