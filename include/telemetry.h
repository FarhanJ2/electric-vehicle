#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdbool.h>

#define WIFI_SSID "Verizon_XM9QH9"
#define WIFI_PASSWORD "awe-held6-bay"
#define PC_IP "192.168.1.217"
#define PC_PORT 5000

/**
 * Initialize WiFi chip (must be called before telemetry_init)
 * This initializes the CYW43 chip for both WiFi and LED control
 * @return true if successful, false otherwise
 */
bool telemetry_chip_init();

/**
 * Initialize WiFi connection and UDP telemetry
 * @return true if successful, false otherwise
 */
bool telemetry_init();

bool connect_to();

/**
 * Send telemetry data packet over UDP
 * @param pitch Current pitch angle (degrees)
 * @param roll Current roll angle (degrees)
 * @param yaw Current yaw angle (degrees)
 * @param ax Accelerometer X (g)
 * @param ay Accelerometer Y (g)
 * @param az Accelerometer Z (g)
 * @param gx Gyroscope X (deg/s)
 * @param gy Gyroscope Y (deg/s)
 * @param gz Gyroscope Z (deg/s)
 */
void telemetry_send(float pitch, float roll, float yaw,
                    float ax, float ay, float az,
                    float gx, float gy, float gz);

// Sends formatted string over UDP telemetry
void telemetry_print(const char *fmt, ...);

#ifdef __cplusplus
extern "C" {
#endif

    int _write(int fd, const char *buf, int count);

#ifdef __cplusplus
}
#endif

/**
 * Check if WiFi is connected
 * @return true if connected, false otherwise
 */
bool telemetry_is_connected();

/**
 * Get WiFi connection status as string
 * @return Status string
 */
const char *telemetry_get_status();

#endif // TELEMETRY_H