#pragma once

#include "GraphicsLib.h"

#include <winsock2.h>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

class GraphicsLibUDP : public GraphicsLib
{
public:
    GraphicsLibUDP(uint_least16_t width, uint_least16_t height);
    ~GraphicsLibUDP();

    bool connectTo(const std::string& ip, uint16_t port);
    void disconnect();

    void fillScreen(uint_least16_t color) override;

    void drawPixel(
        int_least16_t x0, int_least16_t y0,
        uint_least16_t color) override;

    void drawLine(
        int_least16_t x0, int_least16_t y0,
        int_least16_t x1, int_least16_t y1,
        uint_least16_t color) override;

    void drawRect(
        int_least16_t x0, int_least16_t y0,
        int_least16_t w, int_least16_t h,
        uint_least16_t color) override;

    void fillRect(
        int_least16_t x0, int_least16_t y0,
        int_least16_t w, int_least16_t h,
        uint_least16_t color) override;

    void drawEllipse(
        int_least16_t x0, int_least16_t y0,
        int_least16_t r_x, int_least16_t r_y,
        uint_least16_t color) override;

    void fillEllipse(
        int_least16_t x0, int_least16_t y0,
        int_least16_t r_x, int_least16_t r_y,
        uint_least16_t color) override;

    int_least16_t drawChar(
        int_least16_t x, int_least16_t y, char c,
        uint_least16_t color, uint_least16_t bg,
        uint_least8_t size) override;

    int_least16_t drawChar(
        int_least16_t x, int_least16_t y, unsigned char c,
        uint_least16_t color, uint_least16_t bg,
        uint_least8_t size) override;

    int_least16_t drawText(
        int_least16_t x, int_least16_t y, const char* s,
        uint_least16_t color, uint_least16_t bg,
        uint_least8_t size) override;

    void loadSprite(
        uint_least8_t index,
        int_least16_t width,
        int_least16_t height,
        char* data) override;

    void showSprite(
        uint_least8_t index,
        uint_least16_t x,
        uint_least16_t y) override;

private:
    SOCKET socketHandle;
    sockaddr_in serverAddress;
    bool connected;
    bool winsockStarted;

    struct Sprite
    {
        int_least16_t width;
        int_least16_t height;
        std::vector<uint8_t> data;
    };

    std::map<uint_least8_t, Sprite> sprites;

    bool sendPacket(const std::vector<uint8_t>& packet);

    void addInt16(
        std::vector<uint8_t>& packet,
        int_least16_t value);

    void addUInt16(
        std::vector<uint8_t>& packet,
        uint_least16_t value);
};