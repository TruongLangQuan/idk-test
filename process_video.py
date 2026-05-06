import os
import subprocess

video_path = "../../Downloads/YTDown_YouTube_Media_FtutLA63Cp8_003_144p.mp4"
cols = 48
rows = 22
fps = 8 # Smooth but doesn't take up too much flash

# Use ffmpeg to get raw grayscale pixels
cmd = [
    "ffmpeg", "-i", video_path, 
    "-vf", f"fps={fps},scale={cols}:{rows},format=gray", 
    "-f", "rawvideo", "-pix_fmt", "gray", "-"
]

proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
ascii_chars = " .:-=+*#%@"
num_chars = len(ascii_chars)

out = open("frames.h", "w")
out.write("#pragma once\n")
out.write("#include <stdint.h>\n")
out.write(f"const int BADAPPLE_FRAMES = <FRAME_COUNT>;\n")
out.write(f"const uint8_t badapple_data[][{rows * cols}] = {{\n")

frame_count = 0
while True:
    data = proc.stdout.read(cols * rows)
    if len(data) < cols * rows:
        break
    out.write("{")
    for i, val in enumerate(data):
        char_idx = (val * num_chars) // 256
        ascii_char = ascii_chars[char_idx]
        out.write(f"'{ascii_char}',")
    out.write("},\n")
    frame_count += 1

out.write("};\n")
out.close()

# Update frame count
with open("frames.h", "r") as f:
    content = f.read()
content = content.replace("<FRAME_COUNT>", str(frame_count))
with open("frames.h", "w") as f:
    f.write(content)

print(f"Processed {frame_count} frames.")
