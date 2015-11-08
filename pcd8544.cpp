#include "pcd8544.h"

#include <stdexcept>

#include <bcm2835.h>

inline uint8_t enable_bit(uint8_t bit) { return 1 << bit; }
inline bool get_bit(uint8_t byte, uint8_t n) { return (byte >> n) & 0x1; }

pcd8544::pcd8544(uint8_t dc_pin, uint8_t sce_pin, uint8_t rst_pin)
    : m_dc_pin(dc_pin)
    , m_sce_pin(sce_pin)
    , m_rst_pin(rst_pin)
{
    begin();
}

pcd8544::~pcd8544() { bcm2835_spi_end(); }

// the most basic function, set a single pixel
void pcd8544::draw_pixel(uint8_t x, uint8_t y, color color)
{
    if (x >= width)
        throw std::out_of_range("x");
    if (y >= height)
        throw std::out_of_range("y");

    // x is which column
    if (color != pcd8544::color::white) {
        m_pixel_buffer[x + (y / pixel_per_byte) * width]
            |= enable_bit(y % pixel_per_byte);
    }
    else {
        m_pixel_buffer[x + (y / pixel_per_byte) * width]
            &= ~enable_bit(y % pixel_per_byte);
    }
}

// the most basic function, set a single pixel
// wrapper function for easier access
void pcd8544::draw_pixel(uint8_t x, uint8_t y, bool black)
{
    draw_pixel(x, y, black ? color::black : color::white);
}

// the most basic function, get a single pixel
pcd8544::color pcd8544::get_pixel(uint8_t x, uint8_t y)
{
    if (x >= width)
        throw std::out_of_range("x");
    if (y >= height)
        throw std::out_of_range("y");

    auto byteContainingPixelXY
        = m_pixel_buffer[x + (y / pixel_per_byte) * width];
    bool isBlack = get_bit(byteContainingPixelXY, y % pixel_per_byte);

    return isBlack ? color::black : color::white;
}

// initializes the SPI and resets the display
void pcd8544::begin(uint8_t contrast, uint8_t bias)
{
    // Setup hardware SPI.
    bcm2835_spi_begin();
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); // 4 MHz CLK
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

    // toggle RST low to reset
    m_rst_pin.set_state(false);
    delay(500);
    m_rst_pin.set_state(true);

    // get into the EXTENDED mode!
    command(command_t::set_function_set_extended);

    // LCD bias select (4 is optimal?)
    command(command_t::set_bias, bias);

    set_contrast(contrast);

    // Set display to Normal
    command(command_t::set_display_control_normal);

    // Push out m_pixel_buffer to the Display (will show the AFI logo)
    display();
}

void pcd8544::command(command_t c, uint8_t v)
{
    m_dc_pin.set_state(false);
    m_sce_pin.set_state(false);
    bcm2835_spi_transfer(c | v);
    m_sce_pin.set_state(true);
}

void pcd8544::set_contrast(uint8_t val)
{
    // clamp contrast to its maximum value of 0x7f (
    if (val > 0x7f) {
        val = 0x7f;
    }
    command(command_t::set_function_set_extended);
    command(command_t::set_vop, val);
    command(command_t::set_function_set_basic);
}

void pcd8544::display()
{
    uint8_t col, maxcol, p;

    for (p = 0; p < 6; p++) {
        command(command_t::set_y_addr, p);

        col = 0;
        maxcol = width - 1;

        command(command_t::set_x_addr, col);

        m_dc_pin.set_state(true);
        m_sce_pin.set_state(false);
        for (; col <= maxcol; col++) {
            bcm2835_spi_transfer(m_pixel_buffer[(width * p) + col]);
        }
        m_sce_pin.set_state(true);
    }

    // no idea why this is necessary but it is to
    // finish the last byte
    command(command_t::set_y_addr);
}

// clear everything
void pcd8544::clear_display() { m_pixel_buffer.fill(0); }