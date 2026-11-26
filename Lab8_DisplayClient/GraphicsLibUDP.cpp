#include "GraphicsLibUDP.h"

#include <ws2tcpip.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")

GraphicsLibUDP::GraphicsLibUDP(
    uint_least16_t width,
    uint_least16_t height)
    : GraphicsLib(width, height)
{
    socketHandle = INVALID_SOCKET;
    connected = false;
    winsockStarted = false;
    serverAddress = {};
}

GraphicsLibUDP::~GraphicsLibUDP()
{
    disconnect();
}

bool GraphicsLibUDP::connectTo(
    const std::string& ip,
    uint16_t port)
{
    if (connected)
        disconnect();

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    winsockStarted = true;

    socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socketHandle == INVALID_SOCKET)
    {
        disconnect();
        return false;
    }

    serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    if (InetPtonA(AF_INET, ip.c_str(), &serverAddress.sin_addr) != 1)
    {
        disconnect();
        return false;
    }

    connected = true;
    return true;
}

void GraphicsLibUDP::disconnect()
{
    connected = false;

    if (socketHandle != INVALID_SOCKET)
    {
        closesocket(socketHandle);
        socketHandle = INVALID_SOCKET;
    }

    if (winsockStarted)
    {
        WSACleanup();
        winsockStarted = false;
    }
}

void GraphicsLibUDP::addInt16(
    std::vector<uint8_t>& packet,
    int_least16_t value)
{
    uint16_t v = static_cast<uint16_t>(value);

    packet.push_back(static_cast<uint8_t>(v & 0xFF));
    packet.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}

void GraphicsLibUDP::addUInt16(
    std::vector<uint8_t>& packet,
    uint_least16_t value)
{
    packet.push_back(static_cast<uint8_t>(value & 0xFF));
    packet.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

bool GraphicsLibUDP::sendPacket(
    const std::vector<uint8_t>& packet)
{
    if (!connected || packet.empty())
        return false;

    int result = sendto(
        socketHandle,
        reinterpret_cast<const char*>(packet.data()),
        static_cast<int>(packet.size()),
        0,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR)
    {
        std::cout << "UDP send error: "
            << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

void GraphicsLibUDP::fillScreen(uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x01 };
    addUInt16(packet, color);
    sendPacket(packet);
}

void GraphicsLibUDP::drawPixel(
    int_least16_t x0, int_least16_t y0,
    uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x02 };
    addInt16(packet, x0);
    addInt16(packet, y0);
    addUInt16(packet, color);
    sendPacket(packet);
}

void GraphicsLibUDP::drawLine(
    int_least16_t x0, int_least16_t y0,
    int_least16_t x1, int_least16_t y1,
    uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x03 };
    addInt16(packet, x0);
    addInt16(packet, y0);
    addInt16(packet, x1);
    addInt16(packet, y1);
    addUInt16(packet, color);
    sendPacket(packet);
}

void GraphicsLibUDP::drawRect(
    int_least16_t x0, int_least16_t y0,
    int_least16_t w, int_least16_t h,
    uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x04 };
    addInt16(packet, x0);
    addInt16(packet, y0);
    addInt16(packet, w);
    addInt16(packet, h);
    addUInt16(packet, color);
    sendPacket(packet);
}

void GraphicsLibUDP::fillRect(
    int_least16_t x0, int_least16_t y0,
    int_least16_t w, int_least16_t h,
    uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x05 };
    addInt16(packet, x0);
    addInt16(packet, y0);
    addInt16(packet, w);
    addInt16(packet, h);
    addUInt16(packet, color);
    sendPacket(packet);
}

void GraphicsLibUDP::drawEllipse(
    int_least16_t x0, int_least16_t y0,
    int_least16_t rx, int_least16_t ry,
    uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x06 };
    addInt16(packet, x0);
    addInt16(packet, y0);
    addInt16(packet, rx);
    addInt16(packet, ry);
    addUInt16(packet, color);
    sendPacket(packet);
}

void GraphicsLibUDP::fillEllipse(
    int_least16_t x0, int_least16_t y0,
    int_least16_t rx, int_least16_t ry,
    uint_least16_t color)
{
    std::vector<uint8_t> packet = { 0x07 };
    addInt16(packet, x0);
    addInt16(packet, y0);
    addInt16(packet, rx);
    addInt16(packet, ry);
    addUInt16(packet, color);
    sendPacket(packet);
}

int_least16_t GraphicsLibUDP::drawChar(
    int_least16_t x, int_least16_t y, char c,
    uint_least16_t color, uint_least16_t bg,
    uint_least8_t size)
{
    if (c == '\0')
        return x;

    char text[2] = { c, '\0' };
    return drawText(x, y, text, color, bg, size);
}

int_least16_t GraphicsLibUDP::drawChar(
    int_least16_t x, int_least16_t y, unsigned char c,
    uint_least16_t color, uint_least16_t bg,
    uint_least8_t size)
{
    char text[2] = { static_cast<char>(c), '\0' };
    return drawText(x, y, text, color, bg, size);
}

int_least16_t GraphicsLibUDP::drawText(
    int_least16_t x, int_least16_t y, const char* s,
    uint_least16_t color, uint_least16_t bg,
    uint_least8_t size)
{
    (void)bg;

    if (s == nullptr)
        return x;

    size_t length = std::strlen(s);

    if (length > 65535)
        return x;

    std::vector<uint8_t> packet = { 0x0C };

    addInt16(packet, x);
    addInt16(packet, y);
    addUInt16(packet, color);
    packet.push_back(static_cast<uint8_t>(size));
    addUInt16(packet, static_cast<uint16_t>(length));

    for (size_t i = 0; i < length; i++)
        packet.push_back(static_cast<uint8_t>(s[i]));

    sendPacket(packet);

    long long nextX =
        static_cast<long long>(x) +
        static_cast<long long>(length) * 8 *
        (size == 0 ? 1 : size);

    if (nextX > 32767)
        nextX = 32767;

    if (nextX < -32768)
        nextX = -32768;

    return static_cast<int_least16_t>(nextX);
}

void GraphicsLibUDP::loadSprite(
    uint_least8_t index,
    int_least16_t width,
    int_least16_t height,
    char* data)
{
    if (!data || width <= 0 || height <= 0)
        return;

    size_t dataSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) * 2;

    Sprite sprite;
    sprite.width = width;
    sprite.height = height;

    const uint8_t* bytes =
        reinterpret_cast<const uint8_t*>(data);

    sprite.data.assign(bytes, bytes + dataSize);

    sprites[index] = sprite;

    std::vector<uint8_t> packet = { 0x11, index };

    addUInt16(packet, static_cast<uint16_t>(width));
    addUInt16(packet, static_cast<uint16_t>(height));

    packet.insert(
        packet.end(),
        sprite.data.begin(),
        sprite.data.end()
    );

    sendPacket(packet);
}

void GraphicsLibUDP::showSprite(
    uint_least8_t index,
    uint_least16_t x,
    uint_least16_t y)
{
    if (sprites.find(index) == sprites.end())
        return;

    std::vector<uint8_t> packet = { 0x12, index };

    addUInt16(packet, x);
    addUInt16(packet, y);

    sendPacket(packet);
}