#include "telemetry.h"

#include <cstdio>
#include <cstring>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

// Internal state
static struct udp_pcb* telemetry_pcb = nullptr;
static ip_addr_t pc_addr;
static bool wifi_fault = false;
static bool chip_initialized = false;

bool telemetry_chip_init() {
    if (chip_initialized) {
        return false;
    }
    
    printf("[Telemetry] Initializing CYW43 chip...\n");
    fflush(stdout);
    
    if (cyw43_arch_init()) {
        printf("[Telemetry] CYW43 chip init failed\n");
        return true;
    }
    
    chip_initialized = true;
    printf("[Telemetry] CYW43 chip initialized\n");
    fflush(stdout);
    
    return false;
}

bool telemetry_init() {
    if (!chip_initialized) {
        printf("[Telemetry] Error: Chip not initialized. Call telemetry_init_chip() first.\n");
        return true;
    }
    
    // Enable WiFi station mode
    cyw43_arch_deinit();
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    bool failed = connect_to();
    if (!failed) {
        printf("[Telemetry] Connecting to WiFi '%s'...\n", WIFI_SSID);
        fflush(stdout);
        
        printf("[Telemetry] Ready - sending to %s:%d\n", PC_IP, PC_PORT);
        fflush(stdout);
    } else {
        return true;
    }
    return false;
}

bool connect_to() {
    // Connect to WiFi with 30 second timeout
    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, 
        WIFI_PASSWORD, 
        CYW43_AUTH_WPA2_AES_PSK, 
        30000
    );
    
    if (result != 0) {
        printf("[Telemetry] Failed to connect to WiFi (error %d)\n", result);
        wifi_fault = false;
        return true;
    }
    
    printf("[Telemetry] WiFi connected!\n");
    wifi_fault = true;
    
    // Create UDP protocol control block
    telemetry_pcb = udp_new();
    if (!telemetry_pcb) {
        printf("[Telemetry] Failed to create UDP PCB\n");
        wifi_fault = false;
        return true;
    }
    
    // Parse PC IP address
    if (!ipaddr_aton(PC_IP, &pc_addr)) {
        printf("[Telemetry] Invalid PC IP address: %s\n", PC_IP);
        udp_remove(telemetry_pcb);
        telemetry_pcb = nullptr;
        wifi_fault = false;
        return true;
    }
    return false;
}

void telemetry_send(float pitch, float roll, float yaw,
                   float ax, float ay, float az,
                   float gx, float gy, float gz) {
    // Check if telemetry is initialized
    if (!wifi_fault || !telemetry_pcb) {
        return;
    }
    
    // Create JSON packet
    char json_buf[256];
    int len = snprintf(json_buf, sizeof(json_buf),
        "{\"pitch\":%.2f,\"roll\":%.2f,\"yaw\":%.2f,"
        "\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,"
        "\"gx\":%.2f,\"gy\":%.2f,\"gz\":%.2f}",
        pitch, roll, yaw, ax, ay, az, gx, gy, gz);
    
    // Allocate packet buffer
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (!p) {
        return;
    }
    
    // Copy data to packet buffer
    memcpy(p->payload, json_buf, len);
    
    // Send UDP packet
    err_t err = udp_sendto(telemetry_pcb, p, &pc_addr, PC_PORT);
    
    // Free packet buffer
    pbuf_free(p);
    
    // Optional: log errors (comment out for performance)
    // if (err != ERR_OK) {
    //     printf("[Telemetry] Send error: %d\n", err);
    // }
}

void telemetry_led(bool on) {
    if (chip_initialized) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on ? 1 : 0);
    }
}

bool telemetry_is_connected() {
    return wifi_fault && telemetry_pcb != nullptr;
}

const char* telemetry_get_status() {
    if (!wifi_fault) {
        return "WiFi Disconnected";
    }
    if (!telemetry_pcb) {
        return "UDP Not Ready";
    }
    return "Connected";
}