#include "telemetry.h"
#include "constant.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>

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
        wifi_fault = true;
        return true;
    }
    wifi_fault = false;
    telemetry_print("[Telemetry] WiFi connected!\n");
    
    // Create UDP protocol control block
    telemetry_pcb = udp_new();
    if (!telemetry_pcb) {
        printf("[Telemetry] Failed to create UDP PCB\n");
        return true;
    }
    
    // Parse PC IP address
    if (!ipaddr_aton(PC_IP, &pc_addr)) {
        printf("[Telemetry] Invalid PC IP address: %s\n", PC_IP);
        udp_remove(telemetry_pcb);
        telemetry_pcb = nullptr;
        wifi_fault = true;
        return true;
    }
    return false;
}

void telemetry_send(float pitch, float roll, float yaw,
                   float ax, float ay, float az,
                   float gx, float gy, float gz) {
    // Check if telemetry is initialized
    if (wifi_fault || !telemetry_pcb) {
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

void telemetry_print(const char* fmt, ...) {
    if (!run_telemetry) return;

    if (wifi_fault || !telemetry_pcb) return;

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, strlen(buf), PBUF_RAM);
    if (!p) return;

    memcpy(p->payload, buf, strlen(buf));
    udp_sendto(telemetry_pcb, p, &pc_addr, PC_PORT);
    pbuf_free(p);
}

extern "C" int _write(int fd, const char* buf, int count) {
    if (!run_telemetry) {
        return count; // skip sending
    }

    if (!wifi_fault && telemetry_pcb) {
        int send_len = count;
        if (send_len > 256) send_len = 256; // limit packet size
        struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, send_len, PBUF_RAM);
        if (p) {
            memcpy(p->payload, buf, send_len);
            udp_sendto(telemetry_pcb, p, &pc_addr, PC_PORT);
            pbuf_free(p);
        }
    }
    return count;
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