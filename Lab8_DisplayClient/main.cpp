#include "GraphicsLibUDP.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

int main()
{
    GraphicsLibUDP display(320, 240);

    if (!display.connectTo("127.0.0.1", 12345))
    {
        std::cout << "Connection error!" << std::endl;
        return 1;
    }

    const uint16_t BLACK = RGB(0, 0, 0);
    const uint16_t WHITE = RGB(255, 255, 255);
    const uint16_t RED = RGB(255, 0, 0);
    const uint16_t GREEN = RGB(0, 255, 0);
    const uint16_t BLUE = RGB(0, 0, 255);
    const uint16_t YELLOW = RGB(255, 255, 0);

    const int width = 16;
    const int height = 16;

    std::vector<uint16_t> image(width * height, BLACK);

    for (int y = 2; y < 14; y++)
    {
        for (int x = 2; x < 14; x++)
        {
            if (x == 2 || x == 13 || y == 2 || y == 13)
                image[y * width + x] = WHITE;
            else if (x >= 5 && x <= 10 && y >= 5 && y <= 10)
                image[y * width + x] = RED;
            else
                image[y * width + x] = GREEN;
        }
    }

    image[7 * width + 7] = BLUE;
    image[7 * width + 8] = BLUE;
    image[8 * width + 7] = BLUE;
    image[8 * width + 8] = BLUE;

    display.fillScreen(BLACK);

    display.drawText(
        20, 20,
        "SPRITE DEMO",
        YELLOW, BLACK, 1
    );

    display.loadSprite(
        1,
        width,
        height,
        reinterpret_cast<char*>(image.data())
    );

    display.showSprite(1, 80, 80);
    display.showSprite(1, 120, 80);
    display.showSprite(1, 160, 80);
    display.showSprite(1, 200, 80);

    display.drawText(
        20, 180,
        "SPRITES LOADED",
        WHITE, BLACK, 1
    );

    std::cout << "Sprite test completed." << std::endl;
    std::cout << "Press Enter to exit." << std::endl;

    std::cin.get();

    display.disconnect();

    return 0;
}