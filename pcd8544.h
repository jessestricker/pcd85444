#ifndef PCD8544_H
#define PCD8544_H

#include <array>
#include <cinttypes>

#include <gpio.h>

class pcd8544 {
public:
    enum class color : uint8_t { white = 0, black = 1 };

    enum command_t : uint8_t {
        set_emp = 0x04,
        set_display_control_normal = 0x08 | 0x04,
        set_bias = 0x10,
        set_function_set_basic = 0x20,
        set_function_set_extended = set_function_set_basic | 0x01,
        set_y_addr = 0x40,
        set_x_addr = 0x80,
        set_vop = 0x80
    };

    static const uint8_t width = 84, height = 48, pixel_per_byte = 8;

    pcd8544(uint8_t dc_pin, uint8_t sce_pin, uint8_t rst_pin);
    ~pcd8544();

    void command(command_t c, uint8_t v = 0);

    void set_contrast(uint8_t val);
    void clear_display();
    void display();

    color get_pixel(uint8_t x, uint8_t y);

    void draw_pixel(uint8_t x, uint8_t y, color color);
    void draw_pixel(uint8_t x, uint8_t y, bool black);

private:
    std::array<uint8_t, width * height / pixel_per_byte> m_pixel_buffer;
    gpio::output_pin m_dc_pin, m_sce_pin, m_rst_pin;

    void begin(uint8_t contrast = 40, uint8_t bias = 0x04);
    void spi_write(uint8_t c);
};

#endif
