//  ====================================================
//  Ｃ言語:簡易グラフィックライブラリ(Win32版) 実装例
//  ====================================================

#include "gr.h"

//  ====================================================
//  ローカル定義.
//  ====================================================
#define	u_int	unsigned int
#define SWAP(x,y)  {int t; t=(x); (x)=(y); (y)=t; }

//  ====================================================
//  ライブラリコンテキスト.
//  ====================================================
typedef struct {
    u_int width, height;        //描画領域のサイズ.
    char menuname[256];         //ウィンドウタイトル.

    int mouse_x, mouse_y;       //マウス座標.
    int *pixbuf;                //描画バッファ.

    HWND hwMain;                //CreateWindowしたウィンドウハンドル.
    HINSTANCE instance;         //GetModuleHandle(NULL)した自分のインスタンス.
    HBITMAP hBMP;               //CreateDIBSection()したビットマップ.
    HDC hdcMem;                 //メモリーＤＣ(デバイスコンテキスト)
    LPBYTE lpBuf;               //GlobalAlloc()した BitMapInfo 構造体.

    // 状態フラグ.
    int destroy_flag;           // 1なら、DestroyWindow()をキックさせる.
    int esc_flag;               // 1なら、ESCキーが押された.
    int end_flag;               // 1なら、WinMainが終了（あるいは予告）
    int close_flag;             // 1なら、gr_close実行中あるいは実行済みである.

    // スレッドハンドル、イベントハンドル.
    HANDLE thread;              // 描画スレッドのハンドル.
    HANDLE esc_event;
    HANDLE hitanykey_event;
    HANDLE cw_event;
    HANDLE paint_event;

    RECT client_rect;           // クライアント矩形.
    ATOM wndclsatom;            // RegisterClassした値.
} gr_context;

static gr_context gr;

//  ====================================================
//  ウィンドウタイトル文字列を設定する.
//  ====================================================
int gr_settitle(char *name)
{
    strcpy(gr.menuname, name);
    return 0;
}

//  ====================================================
//  再描画.
//  ====================================================
void redraw(HDC hdc, LPRECT prc)
{
    BitBlt(hdc,
           prc->left, prc->top,
           prc->right - prc->left, prc->bottom - prc->top,
           gr.hdcMem, prc->left, prc->top, SRCCOPY);
}

//  ====================================================
//  (x,y)座標からピクセルバッファのポインタ*pを得る.
//  ====================================================
int *gr_point(int x, int y)
{
    if (((u_int) x < gr.width) && ((u_int) y < gr.height)) {
        return &gr.pixbuf[y * gr.width + x];
    }
    return 0;
}

//  ====================================================
//  (x,y)座標に点(color)を打つ.
//  ====================================================
void gr_pset(int x, int y, int color)
{
    if (gr.hBMP == NULL)
        return;

    if (((u_int) x < gr.width) && ((u_int) y < gr.height)) {
        gr.pixbuf[y * gr.width + x] = color;
    }
}

#define GR_PSET(x,y,color)	gr.pixbuf[(y)*gr.width+(x)]=color

//  ====================================================
//  線分を描画する.
//  ====================================================
void gr_line(int x1, int y1, int x2, int y2, int c)
{
    int px, py;                 /* 打つべき点 */
    int r;                      /* 誤差レジスタ */
    int dx, dy, dir, count;

    if (x1 > x2) {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    px = x1;
    py = y1;                    /* 開始点 */
    dx = x2 - x1;               /* デルタ */
    dy = y2 - y1;
    dir = 1;
    if (dy < 0) {
        dy = -dy;
        dir = -1;
    }
    /* 負の傾き */
    if (dx < dy) {              /* デルタｙの方が大きい場合 */
        count = dy + 1;
        r = dy / 2;
        do {
            gr_pset(px, py, c);
            py += dir;
            r += dx;
            if (r >= dy) {
                r -= dy;
                px++;
            }
        } while (--count);
    } else {                    /* デルタｘの方が大きい場合 */
        count = dx + 1;
        r = dx / 2;
        do {
            gr_pset(px, py, c);
            px++;
            r += dy;
            if (r >= dx) {
                r -= dx;
                py += dir;
            }
        } while (--count);
    }
}

//  ====================================================
//  円(cx,cy),半径r,色c を描く
//  ====================================================
void gr_circle(int cx, int cy, int r, int c)
{
    int x, y;
    int xr, yr;

    if (r == 0)
        return;
    x = r * r;
    y = 0;
    do {
        xr = x / r;
        yr = y / r;
        gr_pset(cx + xr, cy + yr, c);
        gr_pset(cx - xr, cy + yr, c);
        gr_pset(cx - xr, cy - yr, c);
        gr_pset(cx + xr, cy - yr, c);

        gr_pset(cx + yr, cy + xr, c);
        gr_pset(cx - yr, cy + xr, c);
        gr_pset(cx - yr, cy - xr, c);
        gr_pset(cx + yr, cy - xr, c);

        x += yr;
        y -= xr;
    } while (x >= (-y));
}

//  ====================================================
//  水平ライン(x1,y1)を引く為のサブルーチン
//  ====================================================
static void hline_sub(int x1, int y1, int xlen, int c)
{
    int *p = gr_point(x1, y1);
    while (xlen) {
        *p++ = c;
        xlen--;
    }
}

//  ====================================================
//  垂直ライン(x1,y1)を引く為のサブルーチン
//  ====================================================
static void vline_sub(int x1, int y1, int ylen, int stride, int c)
{
    int *p = gr_point(x1, y1);
    while (ylen) {
        *p = c;
        p += stride;
        ylen--;
    }
}

//  ====================================================
//  水平ライン(x1,y1) - (x2,y2) を引く　（y2は無視）
//  ====================================================
void gr_hline(int x1, int y1, int x2, int y2, int c)
{
    unsigned int xlen;

    if (x2 < x1)
        SWAP(x1, x2);
    if (((u_int) x1 < gr.width) && ((u_int) y1 < gr.height)) {
        xlen = x2 - x1 + 1;     /* 線の長さ */
        hline_sub(x1, y1, xlen, c);
    }
}

//  ====================================================
//  水平ライン(x1,y1) - (x2,y2) を引く　（y2は無視）
//  ====================================================
void gr_vline(int x1, int y1, int x2, int y2, int c)
{
    unsigned int ylen;
    if (gr.hBMP == NULL)
        return;

    if (y2 < y1)
        SWAP(y1, y2);
    if (((u_int) y1 < gr.height) && ((u_int) x1 < gr.width)) {
        ylen = y2 - y1 + 1;     /* 線の長さ */
        vline_sub(x1, y1, ylen, gr.width, c);
    }
}

//  ====================================================
//  ＢＯＸＦＩＬＬ(x1,y1) - (x2,y2),c
//  ====================================================
void gr_boxfill(int x1, int y1, int x2, int y2, int c)
{
    int xlen, ylen;
    if (y2 < y1)
        SWAP(y1, y2);
    if (x2 < x1)
        SWAP(x1, x2);

    ylen = y2 - y1 + 1;
    xlen = x2 - x1 + 1;         /* 線の長さ */

    while (ylen) {
        hline_sub(x1, y1, xlen, c);
        y1++;
        ylen--;
    }
}


//  ====================================================
//  ＢＯＸ(x1,y1) - (x2,y2),c
//  ====================================================
void gr_box(int x1, int y1, int x2, int y2, int c)
{
    gr_hline(x1, y1, x2, y1, c);
    gr_hline(x1, y2, x2, y2, c);
    gr_vline(x1, y1, x1, y2, c);
    gr_vline(x2, y1, x2, y2, c);
}

//  ====================================================
//  描画領域を色(color)で全クリア.
//  ====================================================
void gr_cls(int color)
{
    int i;
    int size = gr.width * gr.height;
    int *p = gr.pixbuf;
    for (i = 0; i < size; i++)
        *p++ = color;
}

//  ====================================================
//  (x,y)座標に色(color)で文字列(*s)を描画する.
//  ====================================================
void gr_puts(int x, int y, char *s, int color)
{
    RECT r;
    r.left = x;                 /* ビットマップの領域指定 */
    r.top = y;
    r.right = gr.width;
    r.bottom = gr.height;

    /* GDI で文字を描画 */
    SetTextColor(gr.hdcMem, color);
    SetBkMode(gr.hdcMem, TRANSPARENT);
    DrawText(gr.hdcMem, s, lstrlen(s), &r, DT_SINGLELINE);
}

//  ====================================================
//  
//  ====================================================
void gr_textout(int x, int y, char *s)
{
    RECT r;
    r.left = x;                 /* ビットマップの領域指定 */
    r.top = y;
    r.right = gr.width;
    r.bottom = gr.height;

    /* GDI で文字を描画 */
    SetTextColor(gr.hdcMem, RGB(240, 240, 240));
    SetBkMode(gr.hdcMem, TRANSPARENT);
//  DrawText(gr.hdcMem,s,lstrlen(s),&r,DT_SINGLELINE);
    TextOut(gr.hdcMem, x, y, s, lstrlen(s));
}

//  ====================================================
//  ウィンドウプロシージャー
//  ====================================================
LRESULT CALLBACK mainwnd_proc(HWND hwnd, UINT msgid, WPARAM wparam,
                              LPARAM lparam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    LPRECT prc;
    RECT r;
    switch (msgid) {

    case WM_MOUSEMOVE:         /* マウスカーソル移動 */
        gr.mouse_x = LOWORD(lparam);    /* マウスカーソルの位置取得 */
        gr.mouse_y = HIWORD(lparam);
        return 0;

    case WM_CREATE:
        return 0;
    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);     // 再描画
//      SetWindowPos(hwMain, NULL, 0, 0,
//               gr.client_rect.right - gr.client_rect.left,
//               gr.client_rect.bottom - gr.client_rect.top,
//               SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
        return 0;
    case WM_SIZING:
        prc = (LPRECT) lparam;
        switch (wparam) {
        case WMSZ_TOP:
        case WMSZ_TOPLEFT:
        case WMSZ_TOPRIGHT:
        case WMSZ_BOTTOMLEFT:
        case WMSZ_LEFT:
            prc->left = prc->right - (gr.client_rect.right
                                      - gr.client_rect.left);
            prc->top = prc->bottom - (gr.client_rect.bottom
                                      - gr.client_rect.top);
            break;
        case WMSZ_BOTTOM:
        case WMSZ_BOTTOMRIGHT:
        case WMSZ_RIGHT:
            prc->right = prc->left + (gr.client_rect.right
                                      - gr.client_rect.left);
            prc->bottom = prc->top + (gr.client_rect.bottom
                                      - gr.client_rect.top);
            break;
        }
        return 0;
    case WM_PAINT:
        {
            int x, y;
            GetClientRect(hwnd, &r);    /* クライアント領域取得 */
            x = (r.right - gr.width) / 2;   /* ビットマップの表示位置計算 */
            y = (r.bottom - gr.height) / 2;

            hdc = BeginPaint(gr.hwMain, &ps);

            BitBlt(hdc, x, y, gr.width, gr.height, gr.hdcMem, 0, 0,
                   SRCCOPY);

            EndPaint(gr.hwMain, &ps);
        }
        SetEvent(gr.paint_event);
        return 0;
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE) {
            gr.esc_flag = 1;
            gr.end_flag = 1;
            SetEvent(gr.esc_event);
        }
        if (lparam & (1 << 30))
            return 0;
        if (wparam == VK_F5) {
            hdc = GetDC(gr.hwMain);
            redraw(hdc, &gr.client_rect);
            ReleaseDC(gr.hwMain, hdc);
            return 0;
        }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        SetEvent(gr.hitanykey_event);
        return 0;
    case WM_DESTROY:
        if (gr.lpBuf) {
            GlobalFree(gr.lpBuf);   /* メモリを解放 */
            gr.lpBuf = NULL;
        }
        DeleteDC(gr.hdcMem);    /* デバイスコンテキスト開放 */
        DeleteObject(gr.hBMP);  /* ビットマップ開放 */
        PostQuitMessage(0);
        gr.hdcMem = NULL;
        gr.hBMP = NULL;
        gr.end_flag = 1;
        return 0;
    }
    return DefWindowProc(hwnd, msgid, wparam, lparam);
}

//  ====================================================
//  ウィンドウ生成
//  ====================================================
void init_window(void)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = mainwnd_proc;
    wcex.cbClsExtra = wcex.cbWndExtra = 0;
    wcex.hInstance = gr.instance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "graphics";
    wcex.hIconSm = wcex.hIcon;
    gr.wndclsatom = RegisterClassEx(&wcex);
    if (gr.wndclsatom == 0) {
        fprintf(stderr, "Can't register class.\n");
        exit(1);
    }

    AdjustWindowRectEx(&gr.client_rect,
                       WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

    gr.hwMain = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                               "graphics",
                               gr.menuname,
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               gr.client_rect.right - gr.client_rect.left,
                               gr.client_rect.bottom - gr.client_rect.top,
                               NULL, NULL, gr.instance, NULL);

    if (gr.hwMain == NULL) {
        fprintf(stderr, "Can't create window.\n");
        UnregisterClass("graphics", gr.instance);
        exit(1);
    }


    ShowWindow(gr.hwMain, SW_NORMAL);
    UpdateWindow(gr.hwMain);

//  SetActiveWindow(gr.hwMain);

}

//  ====================================================
//  ＤＩＢビットマップ生成
//  ====================================================
void init_bitmap(int bpp)
{
    LPBITMAPINFO lpDIB;
    LPBYTE lpBMP;
    HDC hdcWin;
    gr.lpBuf = GlobalAlloc(GPTR, sizeof(BITMAPINFO));
    lpDIB = (LPBITMAPINFO) gr.lpBuf;
    lpDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpDIB->bmiHeader.biWidth = gr.width;
    lpDIB->bmiHeader.biHeight = -gr.height; //上がy=0のbitmap
    lpDIB->bmiHeader.biPlanes = 1;
    lpDIB->bmiHeader.biBitCount = bpp;  //16;
    lpDIB->bmiHeader.biCompression = BI_RGB;
    lpDIB->bmiHeader.biSizeImage = 0;
    lpDIB->bmiHeader.biXPelsPerMeter = 0;
    lpDIB->bmiHeader.biYPelsPerMeter = 0;
    lpDIB->bmiHeader.biClrUsed = 0;
    lpDIB->bmiHeader.biClrImportant = 0;

    hdcWin = GetDC(gr.hwMain);  /* ウインドウのDC を取得 */

    /* DIB とウインドウのDC からDIBSection を作成 */
    gr.hBMP =
        CreateDIBSection(hdcWin, lpDIB, DIB_RGB_COLORS, (void *) &lpBMP,
                         NULL, 0);

    gr.hdcMem = CreateCompatibleDC(hdcWin); /* メモリDC を作成 */

    SelectObject(gr.hdcMem, gr.hBMP);   /* メモリDC にビットマップを選択 */

    ReleaseDC(gr.hwMain, hdcWin);

    gr.pixbuf = (int *) lpBMP;
}

//  ====================================================
//  ウィンドウメイン処理用のスレッド.
//  ====================================================
DWORD WINAPI xmain(LPVOID param)
{
    MSG msg;

    init_window();

    SetEvent(gr.cw_event);

    gr.destroy_flag = 0;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (gr.destroy_flag) {
            DestroyWindow(gr.hwMain);
            gr.hwMain = NULL;
        }
    }

    gr.end_flag = 1;
    gr.esc_flag = 0;
//  DeleteObject(bitmap);

    return msg.wParam;
}

//  ====================================================
//  画面フリップ.
//  ====================================================
int gr_flip(int flag)
{
    ResetEvent(gr.paint_event);
    InvalidateRect(gr.hwMain, NULL, FALSE);
    WaitForSingleObject(gr.paint_event, INFINITE);

    return gr.end_flag;
}

//  ====================================================
//  再描画を促すと同時に、終了チェックの値を返す.
//  ====================================================
int gr_break(void)
{
    if (gr.hwMain != NULL) {
        InvalidateRect(gr.hwMain, NULL, FALSE);
    }
    return gr.end_flag;
}

//  ====================================================
//  
//  ====================================================
int hitanykey(void)
{
    char fnbuf[MAX_PATH];

    gr_flip(0);

    GetWindowsDirectory(fnbuf, MAX_PATH);
    ResetEvent(gr.hitanykey_event);
    WaitForSingleObject(gr.hitanykey_event, INFINITE);

    return gr.esc_flag;
}


//  ====================================================
//  グラフィック終了処理.
//  ====================================================
void gr_close(void)
{
    DWORD exitcode;

    if (gr.close_flag)
        return;

    GetExitCodeThread(gr.thread, &exitcode);
    if (exitcode == STILL_ACTIVE) {
        gr.destroy_flag = 1;
        while (exitcode == STILL_ACTIVE) {
            GetExitCodeThread(gr.thread, &exitcode);
        }
    }
    if (gr.wndclsatom != 0) {
        UnregisterClass("graphics", gr.instance);
    }
    CloseHandle(gr.esc_event);
    CloseHandle(gr.hitanykey_event);
    CloseHandle(gr.cw_event);
    CloseHandle(gr.paint_event);

    if (gr.lpBuf) {
        GlobalFree(gr.lpBuf);
        gr.lpBuf = NULL;
    }
    gr.close_flag = 1;
}


//  ====================================================
//  グラフィック初期化.
//  ====================================================
void gr_init(int width, int height, int bpp, int color)
{
    //static 
    int first = 1;

    width = (width + 3) & (-4); //４の倍数にする.

    memset(&gr, 0, sizeof(gr));
    gr.width = width;
    gr.height = height;
    gr_settitle("HIDmon");

    gr.client_rect.left = 32;
    gr.client_rect.top = 32;

    gr.client_rect.right = gr.client_rect.left + width;
    gr.client_rect.bottom = gr.client_rect.top + height;

    gr.end_flag = 0;

    if (first) {
        DWORD thid;

        atexit(gr_close);
        first = 0;
        gr.close_flag = 0;

        gr.esc_event = CreateEvent(NULL, FALSE, TRUE, NULL);
        gr.hitanykey_event = CreateEvent(NULL, FALSE, TRUE, NULL);
        gr.cw_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        gr.paint_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        gr.thread = CreateThread(NULL, 0, xmain, NULL, 0, &thid);
        if (gr.thread == NULL) {
            fprintf(stderr, "Can't create thread.\n");
            exit(1);
        }
        WaitForSingleObject(gr.cw_event, INFINITE);
    }
    init_bitmap(bpp);
}
