#define WIN32_LEAN_AND_MEAN

#include "DisplayProtocol.h"
#include "UDPServer.h"

#include <windows.h>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <cstdint>

const int DISPLAY_WIDTH = 320;
const int DISPLAY_HEIGHT = 240;

HWND windowHandle;
HBITMAP displayBitmap;
HDC memoryDC;

std::mutex displayMutex;

struct Sprite
{
    int width;
    int height;
    std::vector<uint8_t> data;
};

std::map<uint8_t, Sprite> sprites;

COLORREF convertColor(uint16_t color)
{
    int r = ((color >> 11) & 31) * 255 / 31;
    int g = ((color >> 5) & 63) * 255 / 63;
    int b = (color & 31) * 255 / 31;

    return RGB(r, g, b);
}

void clearDisplay(uint16_t color)
{
    HBRUSH brush = CreateSolidBrush(convertColor(color));
    RECT rect = { 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT };

    FillRect(memoryDC, &rect, brush);
    DeleteObject(brush);
}

void drawPixel(int x, int y, uint16_t color)
{
    SetPixel(memoryDC, x, y, convertColor(color));
}

void drawLine(int x0, int y0, int x1, int y1, uint16_t color)
{
    HPEN pen = CreatePen(PS_SOLID, 1, convertColor(color));
    HPEN oldPen = (HPEN)SelectObject(memoryDC, pen);

    MoveToEx(memoryDC, x0, y0, NULL);
    LineTo(memoryDC, x1, y1);

    SelectObject(memoryDC, oldPen);
    DeleteObject(pen);
}

void drawRectangle(
    int x, int y, int w, int h,
    uint16_t color, bool filled)
{
    HBRUSH brush = filled
        ? CreateSolidBrush(convertColor(color))
        : (HBRUSH)GetStockObject(NULL_BRUSH);

    HPEN pen = CreatePen(PS_SOLID, 1, convertColor(color));

    HBRUSH oldBrush = (HBRUSH)SelectObject(memoryDC, brush);
    HPEN oldPen = (HPEN)SelectObject(memoryDC, pen);

    Rectangle(memoryDC, x, y, x + w, y + h);

    SelectObject(memoryDC, oldBrush);
    SelectObject(memoryDC, oldPen);

    if (filled)
        DeleteObject(brush);

    DeleteObject(pen);
}

void drawEllipse(
    int x, int y, int rx, int ry,
    uint16_t color, bool filled)
{
    HBRUSH brush = filled
        ? CreateSolidBrush(convertColor(color))
        : (HBRUSH)GetStockObject(NULL_BRUSH);

    HPEN pen = CreatePen(PS_SOLID, 1, convertColor(color));

    HBRUSH oldBrush = (HBRUSH)SelectObject(memoryDC, brush);
    HPEN oldPen = (HPEN)SelectObject(memoryDC, pen);

    Ellipse(memoryDC, x - rx, y - ry, x + rx, y + ry);

    SelectObject(memoryDC, oldBrush);
    SelectObject(memoryDC, oldPen);

    if (filled)
        DeleteObject(brush);

    DeleteObject(pen);
}

void drawTextCommand(const Command& cmd)
{
    SetTextColor(memoryDC, convertColor(cmd.color));
    SetBkMode(memoryDC, TRANSPARENT);

    int fontSize = 16 + cmd.font * 2;

    HFONT font = CreateFontA(
        fontSize, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        "Arial"
    );

    HFONT oldFont = (HFONT)SelectObject(memoryDC, font);

    TextOutA(
        memoryDC,
        cmd.parameters[0],
        cmd.parameters[1],
        cmd.text.c_str(),
        static_cast<int>(cmd.text.size())
    );

    SelectObject(memoryDC, oldFont);
    DeleteObject(font);
}

void drawImage(
    int x, int y, int w, int h,
    const std::vector<uint8_t>& data)
{
    for (int row = 0; row < h; row++)
    {
        for (int col = 0; col < w; col++)
        {
            size_t index =
                (static_cast<size_t>(row) * w + col) * 2;

            uint16_t color =
                data[index] |
                (static_cast<uint16_t>(data[index + 1]) << 8);

            drawPixel(x + col, y + row, color);
        }
    }
}

void executeCommand(const Command& cmd)
{
    std::lock_guard<std::mutex> lock(displayMutex);

    switch (cmd.type)
    {
    case CLEAR_DISPLAY:
        clearDisplay(cmd.color);
        break;

    case DRAW_PIXEL:
        drawPixel(cmd.parameters[0], cmd.parameters[1], cmd.color);
        break;

    case DRAW_LINE:
        drawLine(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color
        );
        break;

    case DRAW_RECTANGLE:
        drawRectangle(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color, false
        );
        break;

    case FILL_RECTANGLE:
        drawRectangle(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color, true
        );
        break;

    case DRAW_ELLIPSE:
        drawEllipse(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color, false
        );
        break;

    case FILL_ELLIPSE:
        drawEllipse(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color, true
        );
        break;

    case DRAW_CIRCLE:
        drawEllipse(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[2],
            cmd.color, false
        );
        break;

    case FILL_CIRCLE:
        drawEllipse(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[2],
            cmd.color, true
        );
        break;

    case DRAW_ROUNDED_RECTANGLE:
        drawRectangle(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color, false
        );
        break;

    case FILL_ROUNDED_RECTANGLE:
        drawRectangle(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.color, true
        );
        break;

    case DRAW_TEXT:
        drawTextCommand(cmd);
        break;

    case DRAW_IMAGE:
        drawImage(
            cmd.parameters[0], cmd.parameters[1],
            cmd.parameters[2], cmd.parameters[3],
            cmd.imageData
        );
        break;

    case LOAD_SPRITE:
    {
        Sprite sprite;
        sprite.width = cmd.parameters[0];
        sprite.height = cmd.parameters[1];
        sprite.data = cmd.imageData;

        sprites[cmd.spriteIndex] = sprite;
        break;
    }

    case SHOW_SPRITE:
    {
        auto it = sprites.find(cmd.spriteIndex);

        if (it != sprites.end())
        {
            drawImage(
                cmd.parameters[0],
                cmd.parameters[1],
                it->second.width,
                it->second.height,
                it->second.data
            );
        }
        break;
    }

    default:
        break;
    }

    InvalidateRect(windowHandle, NULL, FALSE);
}

LRESULT CALLBACK WindowProcedure(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        {
            std::lock_guard<std::mutex> lock(displayMutex);

            BitBlt(
                hdc, 0, 0,
                DISPLAY_WIDTH, DISPLAY_HEIGHT,
                memoryDC, 0, 0, SRCCOPY
            );
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int main()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = "DisplayEmulator";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    windowHandle = CreateWindowA(
        "DisplayEmulator",
        "Display Module Emulator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        DISPLAY_WIDTH + 16, DISPLAY_HEIGHT + 39,
        NULL, NULL, hInstance, NULL
    );

    HDC hdc = GetDC(windowHandle);
    memoryDC = CreateCompatibleDC(hdc);

    displayBitmap = CreateCompatibleBitmap(
        hdc, DISPLAY_WIDTH, DISPLAY_HEIGHT
    );

    ReleaseDC(windowHandle, hdc);

    SelectObject(memoryDC, displayBitmap);
    clearDisplay(0x0000);

    ShowWindow(windowHandle, SW_SHOW);

    UDPServer server;

    if (!server.start())
    {
        MessageBoxA(
            windowHandle,
            "Failed to start UDP server",
            "Error",
            MB_ICONERROR
        );
        return 1;
    }

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    server.stop();

    DeleteObject(displayBitmap);
    DeleteDC(memoryDC);

    return 0;
}