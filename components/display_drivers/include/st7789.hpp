#pragma once

#include "display_drivers.hpp"

namespace espp {
  /**
   * @brief Display driver for the ST7789 display controller.
   *
   *   This code is modified from
   *   https://github.com/lvgl/lvgl_esp32_drivers/blob/master/lvgl_tft/st7789.c
   *   and
   *   https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ST7789_Defines.h
   *
   *   See also:
   *   https://github.com/espressif/esp-who/blob/master/components/screen/controller_driver/st7789/st7789.c
   *   or
   *   https://github.com/espressif/tflite-micro-esp-examples/blob/master/components/screen/controller_driver/st7789/st7789.c
   *   or https://esphome.io/api/st7789v_8h_source.html or
   *   https://github.com/mireq/esp32-st7789-demo/blob/master/components/st7789/include/st7789.h
   *   or
   *   https://github.com/mireq/esp32-st7789-demo/blob/master/components/st7789/st7789.c
   *
   * \section st7789_ex1 st7789 Example
   * \snippet display_drivers_example.cpp st7789 example
   */
  class St7789 {
  public:
    enum class Command : uint8_t {
      nop = 0x00,        // no operation
      swreset = 0x01,    // software reset
      rddid = 0x04,      // read display id
      rddst = 0x09,      // read display status

      rddpm = 0x0a,      // read display power mode
      rdd_madctl = 0x0b, // read display madctl
      rdd_colmod = 0x0c, // read display pixel format
      rddim = 0x0d,      // read display image mode
      rddsm = 0x0e,      // read display signal mode
      rddsr = 0x0f,      // read display self-diagnostic result (st7789v)

      slpin = 0x10,      // sleep in
      slpout = 0x11,     // sleep out
      ptlon = 0x12,      // partial mode on
      noron = 0x13,      // normal display mode on

      invoff = 0x20, // display inversion off
      invon = 0x21,  // display inversion on
      gamset = 0x26, // gamma set
      dispoff = 0x28,// display off
      dispon = 0x29, // display on
      caset = 0x2a,  // column address set
      raset = 0x2b,  // row address set
      ramwr = 0x2c,  // ram write
      rgbset = 0x2d, // color setting for 4096, 64k and 262k colors
      ramrd = 0x2e,  // ram read

      ptlar = 0x30,
      vscrdef = 0x33, // vertical scrolling definition (st7789v)
      teoff = 0x34,   // tearing effect line off
      teon = 0x35,    // tearing effect line on
      madctl = 0x36,  // memory data access control
      idmoff = 0x38,  // idle mode off
      idmon = 0x39,   // idle mode on
      ramwrc = 0x3c,  // memory write continue (st7789v)
      ramrdc = 0x3e,  // memory read continue (st7789v)
      colmod = 0x3a,  // color mode - pixel format

      ramctrl = 0xb0,   // ram control
      rgbctrl = 0xb1,   // rgb control
      porctrl = 0xb2,   // porch control
      frctrl1 = 0xb3,   // frame rate control
      parctrl = 0xb5,   // partial mode control
      gctrl = 0xb7,     // gate control
      gtadj = 0xb8,     // gate on timing adjustment
      dgmen = 0xba,     // digital gamma enable
      vcoms = 0xbb,     // vcoms setting
      lcmctrl = 0xc0,   // lcm control
      idset = 0xc1,     // id setting
      vdvvrhen = 0xc2,  // vdv and vrh command enable
      vrhs = 0xc3,      // vrh set
      vdvset = 0xc4,    // vdv setting
      vcmofset = 0xc5,  // vcoms offset set
      frctr2 = 0xc6,    // fr control 2
      cabcctrl = 0xc7,  // cabc control
      regsel1 = 0xc8,   // register value section 1
      regsel2 = 0xca,   // register value section 2
      pwmfrsel = 0xcc,  // pwm frequency selection
      pwctrl1 = 0xd0,   // power control 1
      vapvanen = 0xd2,  // enable vap/van signal output
      cmd2en = 0xdf,    // command 2 enable
      pvgamctrl = 0xe0, // positive voltage gamma control
      nvgamctrl = 0xe1, // negative voltage gamma control
      dgmlutr = 0xe2,   // digital gamma look-up table for red
      dgmlutb = 0xe3,   // digital gamma look-up table for blue
      gatectrl = 0xe4,  // gate control
      spi2en = 0xe7,    // spi2 enable
      pwctrl2 = 0xe8,   // power control 2
      eqctrl = 0xe9,    // equalize time control
      promctrl = 0xec,  // program control
      promen = 0xfa,    // program mode enable
      nvmset = 0xfc,    // nvm setting
      promact = 0xfe,   // program action
    };

    /**
     * @brief Store the config data and send the initialization commands to the
     *        display controller.
     * @param config display_drivers::Config structure
     */
    static void initialize(const display_drivers::Config& config) {
      // update the static members
      lcd_write_ = config.lcd_write;
      reset_pin_ = config.reset_pin;
      dc_pin_ = config.data_command_pin;
      backlight_pin_ = config.backlight_pin;
      offset_x_ = config.offset_x;
      offset_y_ = config.offset_y;

      // Initialize display pins
      display_drivers::init_pins(reset_pin_, dc_pin_, backlight_pin_, config.backlight_on_value);

      // set up the init commands
      display_drivers::LcdInitCmd st_init_cmds[] = {
        {0xCF, {0x00, 0x83, 0X30}, 3},
        {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
        {(uint8_t)Command::pwctrl2, {0x85, 0x01, 0x79}, 3},
        {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
        {0xF7, {0x20}, 1},
        {0xEA, {0x00, 0x00}, 2},
        {(uint8_t)Command::lcmctrl, {0x26}, 1},
        {(uint8_t)Command::idset, {0x11}, 1},
        {(uint8_t)Command::vcmofset, {0x35, 0x3E}, 2},
        {(uint8_t)Command::cabcctrl, {0xBE}, 1},
        {(uint8_t)Command::madctl, {0x00}, 1},
        {(uint8_t)Command::colmod, {0x55}, 1},
        {(uint8_t)Command::invon, {0}, 0},
        {(uint8_t)Command::rgbctrl, {0x00, 0x1B}, 2},
        {0xF2, {0x08}, 1},
        {(uint8_t)Command::gamset, {0x01}, 1},
        {(uint8_t)Command::pvgamctrl, {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28,
        0x32, 0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17}, 14},
        {(uint8_t)Command::nvgamctrl, {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28,
        0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E}, 14},
        {(uint8_t)Command::caset, {0x00, 0x00, 0x00, 0xEF}, 4},
        {(uint8_t)Command::raset, {0x00, 0x00, 0x01, 0x3f}, 4},
        {(uint8_t)Command::ramwr, {0}, 0},
        {(uint8_t)Command::gctrl, {0x07}, 1},
        {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
        {(uint8_t)Command::slpout, {0}, 0x80},
        {(uint8_t)Command::dispon, {0}, 0x80},
        {0, {0}, 0xff},
      };
      // NOTE: these configurations operates on the MADCTL command / register
      if (config.mirror_x) {
        st_init_cmds[10].data[0] |= LCD_CMD_MX_BIT;
      }
      if (config.mirror_y) {
        st_init_cmds[10].data[0] |= LCD_CMD_MY_BIT;
      }
      if (config.swap_xy) {
        st_init_cmds[10].data[0] |= LCD_CMD_MV_BIT;
      }

      // NOTE: ST7789 setting the reverse color is the normal color so we inver
      // the logic here.
      if (config.invert_colors) {
        st_init_cmds[12].command = (uint8_t)Command::invoff;
      } else {
        st_init_cmds[12].command = (uint8_t)Command::invon;
      }

      // send the init commands
      send_commands(st_init_cmds);
    }

    /**
     * @brief Flush the pixel data for the provided area to the display.
     * @param *drv Pointer to the LVGL display driver.
     * @param *area Pointer to the structure describing the pixel area.
     * @param *color_map Pointer to array of colors to flush to the display.
     */
    static void flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
      fill(drv, area, color_map, (uint32_t)Display::Signal::FLUSH);
    }

    /**
     * @brief Flush the pixel data for the provided area to the display.
     * @param *drv Pointer to the LVGL display driver.
     * @param *area Pointer to the structure describing the pixel area.
     * @param *color_map Pointer to array of colors to flush to the display.
     * @param flags uint32_t user data / flags to pass to the lcd_write transfer function.
     */
    static void fill(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map, uint32_t flags=(uint32_t)Display::Signal::NONE) {
      uint8_t data[4] = {0};

      uint16_t start_x = area->x1 + offset_x_;
      uint16_t end_x = area->x2 + offset_x_;
      uint16_t start_y = area->y1 + offset_y_;
      uint16_t end_y = area->y2 + offset_y_;

      // Set the column (x) start / end addresses
      send_command((uint8_t)Command::caset);
      data[0] = (start_x >> 8) & 0xFF;
      data[1] = start_x & 0xFF;
      data[2] = (end_x >> 8) & 0xFF;
      data[3] = end_x & 0xFF;
      send_data(data, 4);

      // Set the row (y) start / end addresses
      send_command((uint8_t)Command::raset);
      data[0] = (start_y >> 8) & 0xFF;
      data[1] = start_y & 0xFF;
      data[2] = (end_y >> 8) & 0xFF;
      data[3] = end_y & 0xFF;
      send_data(data, 4);

      // Write the color data to controller RAM
      send_command((uint8_t)Command::ramwr);
      uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
      send_data((uint8_t *)color_map, size * 2, flags);
    }

    /**
     * @brief Clear the display area, filling it with the provided color.
     * @param x X coordinate of the upper left corner of the display area.
     * @param y Y coordinate of the upper left corner of the display area.
     * @param width Width of the display area to clear.
     * @param height Height of the display area to clear.
     * @param color 16 bit color (default 0x0000) to fill with.
     */
    static void clear(size_t x, size_t y, size_t width, size_t height, uint16_t color=0x0000) {
      uint8_t data[4] = {0};

      uint16_t start_x = x + offset_x_;
      uint16_t end_x = (x+width) + offset_x_;
      uint16_t start_y = y + offset_y_;
      uint16_t end_y = (y+width) + offset_y_;

      // Set the column (x) start / end addresses
      send_command((uint8_t)Command::caset);
      data[0] = (start_x >> 8) & 0xFF;
      data[1] = start_x & 0xFF;
      data[2] = (end_x >> 8) & 0xFF;
      data[3] = end_x & 0xFF;
      send_data(data, 4);

      // Set the row (y) start / end addresses
      send_command((uint8_t)Command::raset);
      data[0] = (start_y >> 8) & 0xFF;
      data[1] = start_y & 0xFF;
      data[2] = (end_y >> 8) & 0xFF;
      data[3] = end_y & 0xFF;
      send_data(data, 4);

      // Write the color data to controller RAM
      send_command((uint8_t)Command::ramwr);
      uint32_t size = width * height;
      static constexpr int max_bytes_to_send = 1024 * 2;
      uint16_t color_data[max_bytes_to_send];
      memset(color_data, color, max_bytes_to_send * sizeof(uint16_t));
      for (int i=0; i<size; i+=max_bytes_to_send) {
        int num_bytes = std::min((int)(size-i), (int)(max_bytes_to_send));
        send_data((uint8_t *)color_data, num_bytes * 2);
      }
    }

    /**
     * @brief Sets the DC pin to command and sends the command code.
     * @param command Command code to send
     */
    static void send_command(uint8_t command) {
      gpio_set_level(dc_pin_, (uint8_t)display_drivers::Mode::COMMAND);
      lcd_write_(&command, 1, (uint32_t)Display::Signal::NONE);
    }

    /**
     * @brief Sets the DC pin to data and sends the data, with optional flags.
     * @param data Pointer to array of bytes to be sent
     * @param length Number of bytes of data to send.
     * @param flags Optional (default = Display::Signal::NONE) flags associated with transfer.
     */
    static void send_data(uint8_t* data, size_t length, uint32_t flags=(uint32_t)Display::Signal::NONE) {
      gpio_set_level(dc_pin_, (uint8_t)display_drivers::Mode::DATA);
      lcd_write_(data, length, flags);
    }

    static void send_commands(display_drivers::LcdInitCmd *commands) {
      using namespace std::chrono_literals;
      //Send all the commands
      uint16_t cmd = 0;
      while (commands[cmd].length!=0xff) {
        send_command(commands[cmd].command);
        send_data(commands[cmd].data, commands[cmd].length&0x1F);
        if (commands[cmd].length & 0x80) {
          std::this_thread::sleep_for(100ms);
        }
        cmd++;
      }
    }

    static void set_offset(int x, int y) {
      offset_x_ = x;
      offset_y_ = y;
    }

    static void get_offset(int &x, int &y) {
      x = offset_x_;
      y = offset_y_;
    }

  protected:
    static Display::write_fn lcd_write_;
    static gpio_num_t reset_pin_;
    static gpio_num_t dc_pin_;
    static gpio_num_t backlight_pin_;
    static int offset_x_;
    static int offset_y_;
  };
}
