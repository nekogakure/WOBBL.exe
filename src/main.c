#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "draw_util.h"

#define MOUSE_JITTER_RANGE 8
#define INVERT_CHANCE 20
#define BEEP_CHANCE 2
#define DISPLAY_JITTER_RANGE 40
#define ICON_BOUNCE_SPEED 8
#define MAX_BOUNCE_ICONS 64

void sleep_seconds(int seconds) {
    Sleep(seconds * 1000);
}

void jitter_mouse(int range) {
    POINT p;
    if (GetCursorPos(&p)) {
        int dx = (rand() % (MOUSE_JITTER_RANGE * range + 1)) - range;
        int dy = (rand() % (MOUSE_JITTER_RANGE * range + 1)) - range;
        SetCursorPos(p.x + dx, p.y + dy);
    }
}

void apply_mosaic_effect(void);
void show_explosion_effect(void);
void shake_screen_effect(void);
void draw_bouncing_icon(void);

DWORD WINAPI text_thread(LPVOID lpParam) {
    while (1) {
        HDC hdc = GetDC(NULL);
        int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        HBRUSH blackBrush = CreateSolidBrush(RGB(0,0,0));
        RECT rect = { left, top, left + w, top + h };
        FillRect(hdc, &rect, blackBrush);
        DeleteObject(blackBrush);
        ReleaseDC(NULL, hdc);

        draw_text_on_screen("WOBBL.EXE");
        Sleep(10);
    }
    return 0;
}

DWORD WINAPI mosaic_thread(LPVOID lpParam) {
    while (1) {
        apply_mosaic_effect();
        Sleep(1000);
    }
    return 0;
}

DWORD WINAPI explosion_thread(LPVOID lpParam) {
    while (1) {
        show_explosion_effect();
        Sleep(5000);
    }
    return 0;
}

DWORD WINAPI shake_thread(LPVOID lpParam) {
    while (1) {
        shake_screen_effect();
        Sleep(30);
    }
    return 0;
}

DWORD WINAPI icon_bounce_thread(LPVOID lpParam) {
    while (1) {
        draw_bouncing_icon();
        Sleep(16);
    }
    return 0;
}

void apply_mosaic_effect(void) {
    HDC hdc = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    for (int i = 0; i < 100; ++i) {
        int x = left + rand() % w;
        int y = top + rand() % h;
        int size = 20 + rand() % 60;
        HBRUSH brush = CreateSolidBrush(RGB(rand()%256, rand()%256, rand()%256));
        RECT rect = { x, y, x + size, y + size };
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }
    ReleaseDC(NULL, hdc);
}

void show_explosion_effect(void) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int gif_w = 256, gif_h = 256;
    int x = rand() % (w - gif_w);
    int y = rand() % (h - gif_h);

    char fullpath[MAX_PATH];
    GetFullPathNameA("assets/bong.gif", MAX_PATH, fullpath, NULL);

    draw_gif_on_screen(fullpath, x, y);
}

void shake_screen_effect(void) {
    HDC hdc = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int top = GetSystemMetrics(SM_YVIRTUALSCREEN);

    int dx = (rand() % 81) - DISPLAY_JITTER_RANGE;
    int dy = (rand() % 81) - DISPLAY_JITTER_RANGE;

    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP membmp = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldbmp = (HBITMAP)SelectObject(memdc, membmp);

    HBRUSH blackBrush = CreateSolidBrush(RGB(0,0,0));
    RECT rect = {0, 0, w, h};
    FillRect(memdc, &rect, blackBrush);
    DeleteObject(blackBrush);

    BitBlt(memdc, dx, dy, w, h, hdc, left, top, SRCCOPY);
    if (rand() % INVERT_CHANCE == 0) {
        BitBlt(memdc, 0, 0, w, h, memdc, 0, 0, NOTSRCCOPY);
    }
    BitBlt(hdc, left, top, w, h, memdc, 0, 0, SRCCOPY);

    if (rand() % BEEP_CHANCE == 0) {
        int freq = 400 + rand() % 1601;    /*  400~2000Hz */
        int dur = 100 + rand() % 201;      /* 100~300ms */
        Beep(freq, dur);
    }

    SelectObject(memdc, oldbmp);
    DeleteObject(membmp);
    DeleteDC(memdc);
    ReleaseDC(NULL, hdc);
}

typedef struct {
    int x, y;
    int vx, vy;
    int w, h;
    HICON hIcon;
    int active;
} BOUNCE_ICON;

void draw_bouncing_icon(void) {
    static BOUNCE_ICON icons[MAX_BOUNCE_ICONS] = {0};
    static int icon_count = 1;
    static int initialized = 0;
    static DWORD last_add = 0;
    HDC hdc = GetDC(NULL);

    if (!initialized) {
        HICON sysicons[] = {
            LoadIcon(NULL, IDI_ERROR),
            LoadIcon(NULL, IDI_WARNING),
            LoadIcon(NULL, IDI_INFORMATION),
            LoadIcon(NULL, IDI_QUESTION)
        };
        int sysicon_count = sizeof(sysicons)/sizeof(sysicons[0]);
        for (int i = 0; i < MAX_BOUNCE_ICONS; ++i) {
            icons[i].active = 0;
        }
        icons[0].hIcon = sysicons[rand() % sysicon_count];
        ICONINFO info;
        BITMAP bmp;
        GetIconInfo(icons[0].hIcon, &info);
        GetObject(info.hbmColor, sizeof(BITMAP), &bmp);
        icons[0].w = bmp.bmWidth;
        icons[0].h = bmp.bmHeight;
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);

        int scrw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int scrh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int top = GetSystemMetrics(SM_YVIRTUALSCREEN);

        icons[0].x = left + rand() % (scrw - 64);
        icons[0].y = top + rand() % (scrh - 64);
        icons[0].vx = (rand() % 2 ? 1 : -1) * ICON_BOUNCE_SPEED;
        icons[0].vy = (rand() % 2 ? 1 : -1) * ICON_BOUNCE_SPEED;
        icons[0].active = 1;
        initialized = 1;
        last_add = GetTickCount();
    }

    DWORD now = GetTickCount();
    if (icon_count < MAX_BOUNCE_ICONS && now - last_add > 1000) {

        HICON sysicons[] = {
            LoadIcon(NULL, IDI_ERROR),
            LoadIcon(NULL, IDI_WARNING),
            LoadIcon(NULL, IDI_INFORMATION),
            LoadIcon(NULL, IDI_QUESTION)
        };
        int sysicon_count = sizeof(sysicons)/sizeof(sysicons[0]);
        int scrw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int scrh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int top = GetSystemMetrics(SM_YVIRTUALSCREEN);

        int idx = icon_count;
        icons[idx].hIcon = sysicons[rand() % sysicon_count];
        ICONINFO info;
        BITMAP bmp;
        GetIconInfo(icons[idx].hIcon, &info);
        GetObject(info.hbmColor, sizeof(BITMAP), &bmp);
        icons[idx].w = bmp.bmWidth;
        icons[idx].h = bmp.bmHeight;
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);

        icons[idx].x = left + rand() % (scrw - 64);
        icons[idx].y = top + rand() % (scrh - 64);
        icons[idx].vx = (rand() % 2 ? 1 : -1) * ICON_BOUNCE_SPEED;
        icons[idx].vy = (rand() % 2 ? 1 : -1) * ICON_BOUNCE_SPEED;
        icons[idx].active = 1;
        icon_count++;
        last_add = now;
    }

    int scrw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int scrh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int top = GetSystemMetrics(SM_YVIRTUALSCREEN);

    for (int i = 0; i < icon_count; ++i) {
        if (!icons[i].active) continue;
        icons[i].x += icons[i].vx;
        icons[i].y += icons[i].vy;

        if (icons[i].x < left) {
            icons[i].x = left;
            icons[i].vx = -icons[i].vx;
            HICON sysicons[] = {
                LoadIcon(NULL, IDI_ERROR),
                LoadIcon(NULL, IDI_WARNING),
                LoadIcon(NULL, IDI_INFORMATION),
                LoadIcon(NULL, IDI_QUESTION)
            };
            int sysicon_count = sizeof(sysicons)/sizeof(sysicons[0]);
            icons[i].hIcon = sysicons[rand() % sysicon_count];
        }
        if (icons[i].x + 64 > left + scrw) {
            icons[i].x = left + scrw - 64;
            icons[i].vx = -icons[i].vx;
            HICON sysicons[] = {
                LoadIcon(NULL, IDI_ERROR),
                LoadIcon(NULL, IDI_WARNING),
                LoadIcon(NULL, IDI_INFORMATION),
                LoadIcon(NULL, IDI_QUESTION)
            };
            int sysicon_count = sizeof(sysicons)/sizeof(sysicons[0]);
            icons[i].hIcon = sysicons[rand() % sysicon_count];
        }
        if (icons[i].y < top) {
            icons[i].y = top;
            icons[i].vy = -icons[i].vy;
            HICON sysicons[] = {
                LoadIcon(NULL, IDI_ERROR),
                LoadIcon(NULL, IDI_WARNING),
                LoadIcon(NULL, IDI_INFORMATION),
                LoadIcon(NULL, IDI_QUESTION)
            };
            int sysicon_count = sizeof(sysicons)/sizeof(sysicons[0]);
            icons[i].hIcon = sysicons[rand() % sysicon_count];
        }
        if (icons[i].y + 64 > top + scrh) {
            icons[i].y = top + scrh - 64;
            icons[i].vy = -icons[i].vy;
            HICON sysicons[] = {
                LoadIcon(NULL, IDI_ERROR),
                LoadIcon(NULL, IDI_WARNING),
                LoadIcon(NULL, IDI_INFORMATION),
                LoadIcon(NULL, IDI_QUESTION)
            };
            int sysicon_count = sizeof(sysicons)/sizeof(sysicons[0]);
            icons[i].hIcon = sysicons[rand() % sysicon_count];
        }

        DrawIconEx(hdc, icons[i].x, icons[i].y, icons[i].hIcon, 64, 64, 0, NULL, DI_NORMAL);
    }

    ReleaseDC(NULL, hdc);
}

int main() {
    SetProcessDPIAware();
    srand((unsigned)time(NULL));
    int counter = 0;

    HANDLE hTextThread = CreateThread(NULL, 0, text_thread, NULL, 0, NULL);
    HANDLE hMosaicThread = CreateThread(NULL, 0, mosaic_thread, NULL, 0, NULL);
    HANDLE hExplosionThread = CreateThread(NULL, 0, explosion_thread, NULL, 0, NULL);
    HANDLE hShakeThread = CreateThread(NULL, 0, shake_thread, NULL, 0, NULL);
    HANDLE hIconBounceThread = CreateThread(NULL, 0, icon_bounce_thread, NULL, 0, NULL);

    while (1) {
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
            (GetAsyncKeyState('Q') & 0x8000) &&
            (GetAsyncKeyState('F') & 0x8000)) {
            break;
        }
        jitter_mouse(8);
        Sleep(100);
    }

    TerminateThread(hTextThread, 0);
    TerminateThread(hMosaicThread, 0);
    TerminateThread(hExplosionThread, 0);
    TerminateThread(hShakeThread, 0);
    TerminateThread(hIconBounceThread, 0);
    CloseHandle(hTextThread);
    CloseHandle(hMosaicThread);
    CloseHandle(hExplosionThread);
    CloseHandle(hShakeThread);
    CloseHandle(hIconBounceThread);

    return 0;
}