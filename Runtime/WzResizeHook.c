#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "WzResizeHook.h"

#define RESIZE_TIMER_ID 1

static void    (*s_frame_cb)(void) = NULL;
static WNDPROC   s_orig_wndproc   = NULL;
static BOOL      s_in_frame       = FALSE;

static LRESULT CALLBACK resize_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ENTERSIZEMOVE:
        SetTimer(hwnd, RESIZE_TIMER_ID, 1, NULL);
        break;
    case WM_EXITSIZEMOVE:
        KillTimer(hwnd, RESIZE_TIMER_ID);
        break;
    case WM_TIMER:
        if (wp == RESIZE_TIMER_ID && s_frame_cb && !s_in_frame) {
            s_in_frame = TRUE;
            s_frame_cb();
            s_in_frame = FALSE;
            return 0;
        }
        break;
    }
    return CallWindowProcW(s_orig_wndproc, hwnd, msg, wp, lp);
}

void wz_install_resize_hook(void* hwnd, void (*on_frame)(void))
{
    s_frame_cb     = on_frame;
    s_orig_wndproc = (WNDPROC)SetWindowLongPtrW((HWND)hwnd, GWLP_WNDPROC, (LONG_PTR)resize_wndproc);
}
