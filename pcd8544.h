#ifndef PCD8544_H
#define PCD8544_H

#include <array>
#include <cinttypes>

#include <gpio.h>

class pcd8544 {
public:
    enum class color : uint8_t { white = 0, black = 1 };

    static const uint8_t width = 84, height = 48, pixel_per_byte = 0;

    pcd8544(uint8_t dc_pin, uint8_t sce_pin, uint8_t rst_pin);
    ~pcd8544();

    void command(uint8_t c);
    void data(uint8_t c);

    void set_contrast(uint8_t val);
    void clear_display();
    void display();

    pcd8544::color get_pixel(uint8_t x, uint8_t y);

    void draw_pixel(uint8_t x, uint8_t y, pcd8544::color color);
    void draw_pixel(uint8_t x, uint8_t y, bool black);

private:
    std::array<uint8_t, pcd8544::width * pcd8544::height
            / pcd8544::pixel_per_byte> m_pixel_buffer;
    gpio::output_pin m_dc_pin, m_sce_pin, m_rst_pin;

    void begin(uint8_t contrast = 40, uint8_t bias = 0x04);
    void spi_write(uint8_t c);
};

#endif
