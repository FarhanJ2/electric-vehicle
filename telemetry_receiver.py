import socket
import json
import threading
import time
from collections import deque
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

class TelemetryReceiver:
    def __init__(self, host='0.0.0.0', port=5000, max_points=100):
        self.host = host
        self.port = port
        self.max_points = max_points
        
        # Data buffers
        self.timestamps = deque(maxlen=max_points)
        self.pitch = deque(maxlen=max_points)
        self.roll = deque(maxlen=max_points)
        self.yaw = deque(maxlen=max_points)
        self.ax = deque(maxlen=max_points)
        self.ay = deque(maxlen=max_points)
        self.az = deque(maxlen=max_points)
        self.gx = deque(maxlen=max_points)
        self.gy = deque(maxlen=max_points)
        self.gz = deque(maxlen=max_points)
        
        self.running = False
        self.start_time = time.time()
        self.lock = threading.Lock()
        
    def start_server(self):
        """Start UDP server to receive telemetry"""
        self.running = True
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        self.sock.settimeout(1.0)
        
        thread = threading.Thread(target=self._receive_data, daemon=True)
        thread.start()
        print(f"Telemetry server started on {self.host}:{self.port}")
        
    def _receive_data(self):
        """Receive and parse telemetry data"""
        while self.running:
            try:
                data, addr = self.sock.recvfrom(1024)
                telemetry = json.loads(data.decode())
                
                with self.lock:
                    self.timestamps.append(time.time() - self.start_time)
                    self.pitch.append(telemetry.get('pitch', 0))
                    self.roll.append(telemetry.get('roll', 0))
                    self.yaw.append(telemetry.get('yaw', 0))
                    self.ax.append(telemetry.get('ax', 0))
                    self.ay.append(telemetry.get('ay', 0))
                    self.az.append(telemetry.get('az', 0))
                    self.gx.append(telemetry.get('gx', 0))
                    self.gy.append(telemetry.get('gy', 0))
                    self.gz.append(telemetry.get('gz', 0))
                    
            except socket.timeout:
                continue
            except Exception as e:
                print(f"Error receiving data: {e}")
                
    def stop(self):
        """Stop the server"""
        self.running = False
        self.sock.close()
        
    def visualize(self):
        """Create real-time visualization"""
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 8))
        fig.suptitle('Nebula Runner - Live Telemetry', fontsize=16)
        
        lines_angles = []
        lines_accel = []
        lines_gyro = []
        
        # Setup subplots
        ax1.set_title('IMU Angles (degrees)')
        ax1.set_xlabel('Time (s)')
        ax1.set_ylabel('Angle')
        line_pitch, = ax1.plot([], [], 'r-', label='Pitch', linewidth=2)
        line_roll, = ax1.plot([], [], 'g-', label='Roll', linewidth=2)
        line_yaw, = ax1.plot([], [], 'b-', label='Yaw', linewidth=2)
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        lines_angles = [line_pitch, line_roll, line_yaw]
        
        ax2.set_title('Accelerometer (g)')
        ax2.set_xlabel('Time (s)')
        ax2.set_ylabel('Acceleration')
        line_ax, = ax2.plot([], [], 'r-', label='X', linewidth=2)
        line_ay, = ax2.plot([], [], 'g-', label='Y', linewidth=2)
        line_az, = ax2.plot([], [], 'b-', label='Z', linewidth=2)
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        lines_accel = [line_ax, line_ay, line_az]
        
        ax3.set_title('Gyroscope (deg/s)')
        ax3.set_xlabel('Time (s)')
        ax3.set_ylabel('Angular Velocity')
        line_gx, = ax3.plot([], [], 'r-', label='X', linewidth=2)
        line_gy, = ax3.plot([], [], 'g-', label='Y', linewidth=2)
        line_gz, = ax3.plot([], [], 'b-', label='Z', linewidth=2)
        ax3.legend()
        ax3.grid(True, alpha=0.3)
        lines_gyro = [line_gx, line_gy, line_gz]
        
        # Status display
        ax4.axis('off')
        status_text = ax4.text(0.1, 0.5, '', fontsize=12, family='monospace')
        
        def update(frame):
            with self.lock:
                if len(self.timestamps) == 0:
                    return lines_angles + lines_accel + lines_gyro
                
                t = list(self.timestamps)
                
                # Update angles
                lines_angles[0].set_data(t, list(self.pitch))
                lines_angles[1].set_data(t, list(self.roll))
                lines_angles[2].set_data(t, list(self.yaw))
                
                # Update accelerometer
                lines_accel[0].set_data(t, list(self.ax))
                lines_accel[1].set_data(t, list(self.ay))
                lines_accel[2].set_data(t, list(self.az))
                
                # Update gyroscope
                lines_gyro[0].set_data(t, list(self.gx))
                lines_gyro[1].set_data(t, list(self.gy))
                lines_gyro[2].set_data(t, list(self.gz))
                
                # Auto-scale axes
                for ax in [ax1, ax2, ax3]:
                    ax.relim()
                    ax.autoscale_view()
                
                # Update status
                status = f"Connected\n"
                status += f"Time: {t[-1]:.1f}s\n"
                status += f"Samples: {len(t)}\n\n"
                status += f"Current Values:\n"
                status += f"Pitch: {self.pitch[-1]:.1f}°\n"
                status += f"Roll:  {self.roll[-1]:.1f}°\n"
                status += f"Yaw:   {self.yaw[-1]:.1f}°"
                status_text.set_text(status)
            
            return lines_angles + lines_accel + lines_gyro
        
        ani = FuncAnimation(fig, update, interval=50, blit=True, cache_frame_data=False)
        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    receiver = TelemetryReceiver(host='0.0.0.0', port=5000, max_points=200)
    receiver.start_server()
    
    try:
        receiver.visualize()
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        receiver.stop()