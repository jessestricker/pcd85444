#include "pcd8544.h"

#include <stdexcept>

#include <bcm2835.h>

const uint8_t PCD8544_POWERDOWN = 0x04, PCD8544_ENTRYMODE = 0x02,
              PCD8544_EXTENDEDINSTRUCTION = 0x01;

const uint8_t PCD8544_DISPLAYBLANK = 0x0, PCD8544_DISPLAYNORMAL = 0x4,
              PCD8544_DISPLAYALLON = 0x1, PCD8544_DISPLAYINVERTED = 0x5;

const uint8_t PCD8544_FUNCTIONSET = 0x20, PCD8544_DISPLAYCONTROL = 0x08,
              PCD8544_SETYADDR = 0x40, PCD8544_SETXADDR = 0x80;

const uint8_t PCD8544_SETTEMP = 0x04, PCD8544_SETBIAS = 0x10,
              PCD8544_SETVOP = 0x80;

inline uint8_t enable_bit(uint8_t bit) { return 1 << bit; }

pcd8544::pcd8544(uint8_t dc_pin, uint8_t sce_pin, uint8_t rst_pin)
    : m_dc_pin(dc_pin)
    , m_sce_pin(sce_pin)
    , m_rst_pin(rst_pin)
{
    begin();
}

pcd8544::~pcd8544() { bcm2835_spi_end(); }

// the most basic function, set a single pixel
void pcd8544::draw_pixel(uint8_t x, uint8_t y, pcd8544::color color)
{
    if ((x >= width) || (y >= height))
        return;

    // x is which column
    if (color != pcd8544::color::white)
        m_pixel_buffer[x + (y / 8) * Width] |= enable_bit(y % 8);
    else
        m_pixel_buffer[x + (y / 8) * Width] &= ~enable_bit(y % 8);
}

// the most basic function, set a single pixel
// wrapper function for easier access
void pcd8544::draw_pixel(uint8_t x, uint8_t y, bool black)
{
    draw_pixel(x, y, black ? color::black : color::white);
}

// the most basic function, get a single pixel
uint8_t pcd8544::get_pixel(uint8_t x, uint8_t y)
{
    if ((x >= width) || (y >= height))
        return 0;

    return (m_pixel_buffer[x + (y / 8) * width] >> (y % 8)) & 0x1;
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
    command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);

    // LCD bias select (4 is optimal?)
    command(PCD8544_SETBIAS | bias);

    set_contrast(contrast);

    // Set display to Normal
    command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

    // Push out m_pixel_buffer to the Display (will show the AFI logo)
    display();
}

void pcd8544::command(uint8_t c)
{
    m_dc_pin.set_state(false);
    m_sce_pin.set_state(false);
    bcm2835_spi_transfer(c);
    m_sce_pin.set_state(true);
}

void pcd8544::data(uint8_t c)
{
    m_dc_pin.set_state(true);
    m_sce_pin.set_state(false);
    bcm2835_spi_transfer(c);
    m_sce_pin.set_state(true);
}

void pcd8544::set_contrast(uint8_t val)
{
    if (val > 0x7f) {
        val = 0x7f;
    }
    command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
    command(PCD8544_SETVOP | val);
    command(PCD8544_FUNCTIONSET);
}

void pcd8544::display()
{
    uint8_t col, maxcol, p;

    for (p = 0; p < 6; p++) {
        command(PCD8544_SETYADDR | p);

        col = 0;
        maxcol = width - 1;

        command(PCD8544_SETXADDR | col);

        m_dc_pin.set_state(true);
        m_sce_pin.set_state(false);
        for (; col <= maxcol; col++) {
            bcm2835_spi_transfer(m_pixel_buffer[(width * p) + col]);
        }
        m_sce_pin.set_state(true);
    }

    // no idea why this is necessary but it is to
    // finish the last byte
    command(PCD8544_SETYADDR);
}

// clear everything
void pcd8544::clear_display() { m_pixel_buffer.fill(0); }