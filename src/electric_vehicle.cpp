#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <include/constants.h>

void periodic();

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    cyw43_arch_enable_ap_mode("elec-vehicle", NULL, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    while (true) {
        periodic();
        sleep_ms(int(constants::dt * 1000));
    }
}

// Called every loop iteration
void periodic() {}
