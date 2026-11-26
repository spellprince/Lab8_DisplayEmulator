#include "DisplayProtocol.h"

bool DisplayProtocol::parse(
    const std::vector<uint8_t>& data,
    Command& cmd)
{
    if (data.empty())
        return false;

    cmd = {};
    cmd.type = static_cast<CommandType>(data[0]);

    auto read16 = [&](size_t pos) -> uint16_t
        {
            return static_cast<uint16_t>(
                data[pos] |
                (static_cast<uint16_t>(data[pos + 1]) << 8)
                );
        };

    switch (cmd.type)
    {
    case CLEAR_DISPLAY:
        if (data.size() != 3)
            return false;

        cmd.color = read16(1);
        return true;

    case DRAW_PIXEL:
        if (data.size() != 7)
            return false;

        cmd.parameters = {
            static_cast<int16_t>(read16(1)),
            static_cast<int16_t>(read16(3))
        };
        cmd.color = read16(5);
        return true;

    case DRAW_LINE:
        if (data.size() != 11)
            return false;

        for (int i = 0; i < 4; i++)
            cmd.parameters.push_back(
                static_cast<int16_t>(read16(1 + i * 2))
            );

        cmd.color = read16(9);
        return true;

    case DRAW_RECTANGLE:
    case FILL_RECTANGLE:
    case DRAW_ELLIPSE:
    case FILL_ELLIPSE:
        if (data.size() != 11)
            return false;

        for (int i = 0; i < 4; i++)
            cmd.parameters.push_back(
                static_cast<int16_t>(read16(1 + i * 2))
            );

        cmd.color = read16(9);
        return true;

    case DRAW_CIRCLE:
    case FILL_CIRCLE:
        if (data.size() != 9)
            return false;

        for (int i = 0; i < 3; i++)
            cmd.parameters.push_back(
                static_cast<int16_t>(read16(1 + i * 2))
            );

        cmd.color = read16(7);
        return true;

    case DRAW_ROUNDED_RECTANGLE:
    case FILL_ROUNDED_RECTANGLE:
        if (data.size() != 13)
            return false;

        for (int i = 0; i < 5; i++)
            cmd.parameters.push_back(
                static_cast<int16_t>(read16(1 + i * 2))
            );

        cmd.color = read16(11);
        return true;

    case DRAW_TEXT:
    {
        if (data.size() < 10)
            return false;

        int16_t x = static_cast<int16_t>(read16(1));
        int16_t y = static_cast<int16_t>(read16(3));

        cmd.parameters = { x, y };
        cmd.color = read16(5);
        cmd.font = data[7];

        uint16_t length = read16(8);

        if (data.size() != 10 + length)
            return false;

        cmd.text.assign(
            data.begin() + 10,
            data.end()
        );

        return true;
    }

    case DRAW_IMAGE:
    {
        if (data.size() < 9)
            return false;

        int16_t x = static_cast<int16_t>(read16(1));
        int16_t y = static_cast<int16_t>(read16(3));
        int16_t w = static_cast<int16_t>(read16(5));
        int16_t h = static_cast<int16_t>(read16(7));

        if (w <= 0 || h <= 0)
            return false;

        size_t expected =
            9 + static_cast<size_t>(w) *
            static_cast<size_t>(h) * 2;

        if (data.size() != expected)
            return false;

        cmd.parameters = { x, y, w, h };
        cmd.imageData.assign(data.begin() + 9, data.end());

        return true;
    }

    case SET_ORIENTATION:
        if (data.size() != 2 || data[1] > 3)
            return false;

        cmd.orientation = data[1];
        return true;

    case GET_WIDTH:
    case GET_HEIGHT:
        return data.size() == 1;

    case LOAD_SPRITE:
    {
        if (data.size() < 8)
            return false;

        cmd.spriteIndex = data[1];

        int16_t w = static_cast<int16_t>(read16(2));
        int16_t h = static_cast<int16_t>(read16(4));

        if (w <= 0 || h <= 0)
            return false;

        size_t expected =
            6 + static_cast<size_t>(w) *
            static_cast<size_t>(h) * 2;

        if (data.size() != expected)
            return false;

        cmd.parameters = { w, h };
        cmd.imageData.assign(data.begin() + 6, data.end());

        return true;
    }

    case SHOW_SPRITE:
        if (data.size() != 6)
            return false;

        cmd.spriteIndex = data[1];
        cmd.parameters = {
            static_cast<int16_t>(read16(2)),
            static_cast<int16_t>(read16(4))
        };
        return true;

    default:
        return false;
    }
}