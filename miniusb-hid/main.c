#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// 1. Monochrome LED pins for diagnostic blinking (compatible with standard Pico and other boards)
const uint LED_PINS[] = {25, 2, 29};
const size_t NUM_LED_PINS = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

void led_init(void) {
    for (size_t i = 0; i < NUM_LED_PINS; i++) {
        gpio_init(LED_PINS[i]);
        gpio_set_dir(LED_PINS[i], GPIO_OUT);
        gpio_put(LED_PINS[i], 0);
    }
}

void set_monochrome_leds(bool state) {
    for (size_t i = 0; i < NUM_LED_PINS; i++) {
        gpio_put(LED_PINS[i], state ? 1 : 0);
    }
}

// 2. WS2812 PIO program instructions (compiled from ws2812.pio)
static const uint16_t ws2812_program_instructions[] = {
    0x6321, //  0: out    x, 1            side 0 [3]
    0x1223, //  1: jmp    !x, 3           side 1 [2]
    0x1200, //  2: jmp    0               side 1 [2]
    0xa242, //  3: nop                    side 0 [2]
};

static const struct pio_program ws2812_program = {
    .instructions = ws2812_program_instructions,
    .length = 4,
    .origin = -1,
};

static inline pio_sm_config ws2812_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + 0, offset + 3);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

#define WS2812_PIO pio0
#define WS2812_FREQ 800000

// Use two State Machines to drive GP16 and GP22 simultaneously
const uint sm_gp16 = 0;
const uint sm_gp22 = 1;

void ws2812_init_all(void) {
    // Add the program to PIO instruction memory
    uint offset = pio_add_program(WS2812_PIO, &ws2812_program);

    // Config State Machine for GP16
    pio_gpio_init(WS2812_PIO, 16);
    pio_sm_set_consecutive_pindirs(WS2812_PIO, sm_gp16, 16, 1, true);
    pio_sm_config c1 = ws2812_program_get_default_config(offset);
    sm_config_set_out_shift(&c1, false, true, 24);
    sm_config_set_fifo_join(&c1, PIO_FIFO_JOIN_TX);
    float div1 = clock_get_hz(clk_sys) / (WS2812_FREQ * 10);
    sm_config_set_clkdiv(&c1, div1);
    pio_sm_init(WS2812_PIO, sm_gp16, offset, &c1);
    pio_sm_set_enabled(WS2812_PIO, sm_gp16, true);

    // Config State Machine for GP22
    pio_gpio_init(WS2812_PIO, 22);
    pio_sm_set_consecutive_pindirs(WS2812_PIO, sm_gp22, 22, 1, true);
    pio_sm_config c2 = ws2812_program_get_default_config(offset);
    sm_config_set_out_shift(&c2, false, true, 24);
    sm_config_set_fifo_join(&c2, PIO_FIFO_JOIN_TX);
    float div2 = clock_get_hz(clk_sys) / (WS2812_FREQ * 10);
    sm_config_set_clkdiv(&c2, div2);
    pio_sm_init(WS2812_PIO, sm_gp22, offset, &c2);
    pio_sm_set_enabled(WS2812_PIO, sm_gp22, true);
}

void ws2812_set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    // Format: Green, Red, Blue (24-bit)
    uint32_t data = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
    // Shift left 8 bits to match FIFO input requirements
    pio_sm_put_blocking(WS2812_PIO, sm_gp16, data << 8u);
    pio_sm_put_blocking(WS2812_PIO, sm_gp22, data << 8u);
}

int main(void) {
    board_init();
    tusb_init();
    led_init();
    ws2812_init_all();

    // Turn off all LEDs initially
    set_monochrome_leds(false);
    ws2812_set_color_all(0, 0, 0);

    uint32_t last_jiggle_ms = 0;
    const uint32_t kJiggleIntervalMs = 100; // Continuous jiggle frequency of 10Hz
    bool jiggle_direction = true;           // Alternate direction back and forth
    uint32_t led_timer_ms = 0;
    bool led_on = false;

    while (1) {
        tud_task(); // Maintain USB HID task

        uint32_t now = board_millis();

        if (tud_hid_ready()) {
            if (now - last_jiggle_ms >= kJiggleIntervalMs) {
                last_jiggle_ms = now;

                // 1. Mouse Jiggler logic: move mouse slightly back and forth
                if (jiggle_direction) {
                    tud_hid_mouse_report(0, 0, 1, 0, 0, 0);
                } else {
                    tud_hid_mouse_report(0, 0, -1, 0, 0, 0);
                }
                jiggle_direction = !jiggle_direction;

                // 2. Trigger active diagnostic blinking (every 500ms, blink for 80ms)
                static uint32_t last_blink_ms = 0;
                if (now - last_blink_ms >= 500) {
                    last_blink_ms = now;
                    
                    // Turn on monochromatic pins
                    set_monochrome_leds(true);
                    
                    // Light up WS2812 RGB NeoPixel on both GP16 and GP22 in vibrant Green (brightness: 40)
                    ws2812_set_color_all(0, 40, 0); 
                    
                    led_on = true;
                    led_timer_ms = now;
                }
            }

            // Turn off diagnostic LEDs after 80ms of being ON
            if (led_on && (now - led_timer_ms >= 80)) {
                set_monochrome_leds(false);
                ws2812_set_color_all(0, 0, 0); // Turn off WS2812 on both GP16 and GP22
                led_on = false;
            }
        }

        sleep_ms(1); // Low power mode sleep
    }

    return 0;
}

// Mandatory TinyUSB Callback Functions
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, uint8_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) itf; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, uint8_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) itf; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}
