#include "draw_util.h"
#include <windows.h>
#include <string.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

void draw_text_on_screen(const char* text) {
    HDC hdc = GetDC(NULL);
    if (!hdc) return;

    HFONT hFont = CreateFontA(
        -100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Meiryo"
    );
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    SIZE sz;
    GetTextExtentPoint32A(hdc, text, (int)strlen(text), &sz);

    int x = (sw - sz.cx) / 2;
    int y = (sh - sz.cy) / 2;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255,0,0));
    TextOutA(hdc, x, y, text, (int)strlen(text));

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(NULL, hdc);
}

void draw_gif_on_screen(const char* gif_path, int x, int y) {
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != 0) {
        return;
    }

    HDC hdc = GetDC(NULL);
    if (hdc) {
        wchar_t wpath[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, gif_path, -1, wpath, MAX_PATH);

        GpImage* image = NULL;
        GdipLoadImageFromFile(wpath, &image);
        if (image) {
            UINT width, height;
            GdipGetImageWidth(image, &width);
            GdipGetImageHeight(image, &height);

            GpGraphics* graphics = NULL;
            GdipCreateFromHDC(hdc, &graphics);
            if (graphics) {
                GdipDrawImageRectI(graphics, image, x, y, width, height);
                GdipDeleteGraphics(graphics);
            }
            GdipDisposeImage(image);
        }
        ReleaseDC(NULL, hdc);
    }

    GdiplusShutdown(gdiplusToken);
}