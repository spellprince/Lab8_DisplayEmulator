#pragma once

#include <cstdint>
#include <vector>
#include <string>

enum CommandType
{
    CLEAR_DISPLAY = 0x01,
    DRAW_PIXEL = 0x02,
    DRAW_LINE = 0x03,
    DRAW_RECTANGLE = 0x04,
    FILL_RECTANGLE = 0x05,
    DRAW_ELLIPSE = 0x06,
    FILL_ELLIPSE = 0x07,
    DRAW_CIRCLE = 0x08,
    FILL_CIRCLE = 0x09,
    DRAW_ROUNDED_RECTANGLE = 0x0A,
    FILL_ROUNDED_RECTANGLE = 0x0B,
    DRAW_TEXT = 0x0C,
    DRAW_IMAGE = 0x0D,
    SET_ORIENTATION = 0x0E,
    GET_WIDTH = 0x0F,
    GET_HEIGHT = 0x10,
    LOAD_SPRITE = 0x11,
    SHOW_SPRITE = 0x12
};

struct Command
{
    CommandType type;
    std::vector<int16_t> parameters;
    uint16_t color = 0;
    uint8_t font = 0;
    uint8_t orientation = 0;
    std::string text;
    std::vector<uint8_t> imageData;
    uint8_t spriteIndex = 0;
};

class DisplayProtocol
{
public:
    static bool parse(
        const std::vector<uint8_t>& data,
        Command& command
    );
};