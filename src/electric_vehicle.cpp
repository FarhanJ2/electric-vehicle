#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include <include/constants.h>

// === CONFIG ===
const bool SEND_TELEMETRY = true; // set to false for comp
const char *WIFI_SSID = "WiFi";
const char *WIFI_PASS = "Pass";
const char *SERVER_IP = "192.168.1.223"; // computer's ip
const uint16_t SERVER_PORT = 2601;

// === GLOBAL SOCKET ===
int sock = -1;

// === FUNCTION DECLARATIONS ===
void wifi_connect();
void connect_to_server();
void periodic();

// === MAIN ===
int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    if (SEND_TELEMETRY) {
        cyw43_arch_enable_sta_mode();

        // Connect to Wi-Fi
        wifi_connect();
        sleep_ms(500); // wait a bit
        // Connect to the PC
        connect_to_server();
    }

    // Main loop
    while (true) {
        periodic();
        sleep_ms(int(constants::dt * 1000));
    }
}

// === CONNECT TO WIFI ===
void wifi_connect() {
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect.\n");
        while (true) tight_loop_contents();
    }
    printf("Connected! Pico IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
}

// === CONNECT TO SERVER ===
void connect_to_server() {
    sock = lwip_socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = PP_HTONS(SERVER_PORT);
    addr.sin_addr.s_addr = ipaddr_addr(SERVER_IP);

    printf("Connecting to server %s:%d...\n", SERVER_IP, SERVER_PORT);
    if (lwip_connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Connection failed.\n");
        while (true) tight_loop_contents();
    }
    printf("Connected to server!\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

// === PERIODIC LOOP ===
void periodic() {
    if (sock < 0) return;

    float test_value = 42.0f;
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Sensor: %.2f\n", test_value);

    lwip_send(sock, buffer, strlen(buffer), MSG_NOSIGNAL);
}
