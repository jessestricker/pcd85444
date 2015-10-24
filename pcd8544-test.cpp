#include <iostream>

#include <signal.h>
#include <unistd.h>

#include <pcd8544.h>

volatile bool running = true;

void my_handler(int s)
{
    std::cout << "Caught signal " << s << "\n";
    running = false;
}

void setup_handler()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

int main()
{
    setup_handler();

    // dc, sce, rst
    auto lcd = pcd8544(22, 17, 27);
    lcd.clear_display();

    uint8_t contrast = 50;
    bool plus = true;

    while (running) {
        std::cout << "Contrast: " << +contrast << "\n";
        lcd.set_contrast(contrast);

        if (contrast >= 90)
            plus = false;
        else if (contrast <= 50)
            plus = true;
        if (plus)
            contrast++;
        else
            contrast--;

        lcd.draw_pixel(0, 0, pcd8544::color::black);
        lcd.draw_pixel(1, 0, pcd8544::color::black);
        lcd.draw_pixel(2, 0, pcd8544::color::black);
        lcd.display();

        delay(500);

        lcd.draw_pixel(0, 0, pcd8544::color::white);
        lcd.draw_pixel(1, 0, pcd8544::color::white);
        lcd.draw_pixel(2, 0, pcd8544::color::white);
        lcd.display();

        delay(500);
    }

    return 0;
}