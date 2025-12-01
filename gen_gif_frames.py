# pip install pillow
from PIL import Image, ImageSequence

GIF_PATH = "anim/nebula_runner_zoom_stars_fast.gif"
OUT_PATH = "include/oled_frames.h"
MAX_FRAMES = 32 # how many frames to keep prob shouldnt go past 48

OLED_WIDTH = 128
OLED_HEIGHT = 64
PAGES = OLED_HEIGHT // 8
FRAME_BUFFER_SIZE = OLED_WIDTH * PAGES

def frame_to_buffer(img):
    """
    Convert a 128x64 1-bit image into SSD1306 page-buffer layout:
    buffer[x + width*(y>>3)] bit (y & 7)
    """
    img = img.convert("1")      # 1-bit, pixels 0 or 255
    pixels = img.load()

    buf = [0] * FRAME_BUFFER_SIZE
    for y in range(OLED_HEIGHT):
        page = y // 8
        bit = y % 8
        for x in range(OLED_WIDTH):
            idx = x + OLED_WIDTH * page
            if pixels[x, y] == 0:   # black = 0, white = 255 (invert if you want)
                # if you want the planet to glow white on black bg, flip this condition
                continue
            buf[idx] |= (1 << bit)
    return buf

def main():
    im = Image.open(GIF_PATH)

    frames = []
    for idx, frame in enumerate(ImageSequence.Iterator(im)):
        if idx >= MAX_FRAMES:
            break
        f = frame.convert("L").resize((OLED_WIDTH, OLED_HEIGHT))
        buf = frame_to_buffer(f)
        frames.append(buf)

    with open(OUT_PATH, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define OLED_WIDTH {OLED_WIDTH}\n")
        f.write(f"#define OLED_HEIGHT {OLED_HEIGHT}\n")
        f.write(f"#define FRAME_COUNT {len(frames)}\n")
        f.write(f"#define FRAME_BUFFER_SIZE {FRAME_BUFFER_SIZE}\n\n")
        f.write("static const uint8_t FRAMES[FRAME_COUNT][FRAME_BUFFER_SIZE] = {\n")

        for i, buf in enumerate(frames):
            f.write(f"    /* frame {i} */\n    {{")

            for j, b in enumerate(buf):
                if j % 16 == 0:
                    f.write("\n        ")
                f.write(f"0x{b:02X}")
                if j != len(buf) - 1:
                    f.write(", ")

            f.write("\n    },\n")
        f.write("};\n")


    print(f"Generated {OUT_PATH} with {len(frames)} frames.")

if __name__ == "__main__":
    main()
