#include "hardware/oled_hw.h"
#include "ssd1306.h"
#include "font.h"
#include "oled_frames.h"
#include <string.h>

static ssd1306_t disp;

int oled_hw_init() {
    // init I2C @ 400kHz
    i2c_init(I2C_PORT, 400000);

    gpio_set_function(OLED_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(OLED_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_PIN_SDA);
    gpio_pull_up(OLED_PIN_SCL);

    // init SSD1306
    bool success = ssd1306_init(&disp, 128, 64, ADDR, I2C_PORT);
    ssd1306_clear(&disp);
    ssd1306_show(&disp);
    return !success;
}

void oled_hw_clear() {
    ssd1306_clear(&disp);
}

void oled_hw_print(int x, int y, const char* text) {
    ssd1306_draw_string(&disp, x, y, 1, text);
}

void oled_hw_update() {
    ssd1306_show(&disp);
}

void play_animation(uint32_t delay_ms) {
    for (int i = 0; i < FRAME_COUNT; ++i) {
        // copy frame buffer into display buffer
        memcpy(disp.buffer, FRAMES[i], FRAME_BUFFER_SIZE);
        oled_hw_update();
        // send buffer to display
        ssd1306_show(&disp);
        sleep_ms(delay_ms);
    }
}
