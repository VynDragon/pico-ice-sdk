/*
 * MIT License
 * 
 * Copyright (c) 2023 tinyVision.ai
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "boards/pico_ice.h"
#include "ice_fpga.h"

void ice_fpga_init(uint8_t freq_mhz) {
    // High impedance mode: do not reset the FPGA: let the user control it
    gpio_set_dir(ICE_FPGA_CRESET_B_PIN, GPIO_IN);
    gpio_init(ICE_FPGA_CRESET_B_PIN);

    // Input pin for sensing configuration status.
    gpio_init(ICE_FPGA_CDONE_PIN);
    gpio_set_dir(ICE_FPGA_CDONE_PIN, GPIO_IN);

    // Output a clock at chosen frequency
    clock_gpio_init(ICE_FPGA_CLOCK_PIN, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_USB, 48 / freq_mhz);
}

void ice_fpga_stop(void) {
    gpio_put(ICE_FPGA_CRESET_B_PIN, false);
    gpio_set_dir(ICE_FPGA_CRESET_B_PIN, GPIO_OUT);
}

// Datasheet iCE40 Programming Configuration - 3.1. Mode Selection
bool ice_fpga_start(void) {
    gpio_put(ICE_FPGA_CRESET_B_PIN, true);
    gpio_set_dir(ICE_FPGA_CRESET_B_PIN, GPIO_OUT);

    // Wait that the configuration is finished before interferring.
    // This makes sure the SPI bus is not driven by both the FPGA
    // (reading from flash) and the RP2040 (configuring the flash).
    // Note that if the flash is corrupted, this function will timeout!
    for (uint8_t timeout = 100; !gpio_get(ICE_FPGA_CDONE_PIN); timeout--) {
        if (timeout == 0)
            return false;
        sleep_ms(1);
    }
    return true;
}
