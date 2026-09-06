import os
import subprocess

video_path = "/home/truonglangquan/Downloads/YTDown_YouTube_Media_FtutLA63Cp8_001_360p.mp4"
cols = 120
rows = 68
fps = 30

if not os.path.exists(video_path):
    print(f"Error: Video file not found at {video_path}")
    exit(1)

# Use ffmpeg to get raw grayscale pixels
cmd = [
    "ffmpeg", "-i", video_path, 
    "-vf", f"fps={fps},scale={cols}:{rows},format=gray", 
    "-f", "rawvideo", "-pix_fmt", "gray", "-"
]

print("Running ffmpeg to extract frames...")
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

rle_bytes = bytearray()
frame_offsets = []
frame_count = 0

total_pixels = cols * rows

while True:
    data = proc.stdout.read(total_pixels)
    if len(data) < total_pixels:
        break
    
    # Store current RLE byte index as this frame's offset
    frame_offsets.append(len(rle_bytes))
    
    # Binarize: pixel < 127 is Black (0), pixel >= 127 is White (1)
    bin_pixels = [1 if val >= 127 else 0 for val in data]
    
    # Perform RLE (runs alternate Black, White, Black, White...)
    # First run starts with Black (color=0)
    current_color = 0
    current_run = 0
    
    for val in bin_pixels:
        if val == current_color:
            current_run += 1
            if current_run == 255:
                rle_bytes.append(255)
                rle_bytes.append(0) # alternate color run of length 0
                current_run = 0
        else:
            rle_bytes.append(current_run)
            current_color = 1 - current_color
            current_run = 1
            
    rle_bytes.append(current_run)
    frame_count += 1
    
    if frame_count % 500 == 0:
        print(f"Compressed {frame_count} frames...")

# Wait for process to exit
proc.wait()
stdout, stderr = proc.communicate()
if proc.returncode != 0:
    print(f"ffmpeg failed with return code {proc.returncode}")
    print(stderr.decode())
    exit(1)

print(f"Total compressed frames: {frame_count}")
print(f"Raw frame data size: {frame_count * total_pixels} bytes ({frame_count * total_pixels / (1024*1024):.2f} MB)")
print(f"Compressed RLE data size: {len(rle_bytes)} bytes ({len(rle_bytes) / (1024*1024):.2f} MB)")
compression_ratio = (frame_count * total_pixels) / len(rle_bytes)
print(f"Compression ratio: {compression_ratio:.2f}x")

# Now generate badapple_video.h
os.makedirs("src", exist_ok=True)
header_path = "src/badapple_video.h"

print(f"Writing to C++ header {header_path}...")
with open(header_path, "w") as out:
    out.write("#pragma once\n")
    out.write("#include <stdint.h>\n\n")
    out.write(f"static constexpr int BADAPPLE_FRAMES = {frame_count};\n")
    out.write(f"static constexpr int BADAPPLE_WIDTH = {cols};\n")
    out.write(f"static constexpr int BADAPPLE_HEIGHT = {rows};\n")
    out.write(f"static constexpr int BADAPPLE_FPS = {fps};\n\n")
    
    # Write offsets
    out.write(f"const uint32_t badapple_frame_offsets[{frame_count}] PROGMEM = {{\n    ")
    for i, offset in enumerate(frame_offsets):
        out.write(f"{offset},")
        if (i + 1) % 12 == 0:
            out.write("\n    ")
    out.write("\n};\n\n")
    
    # Write compressed bytes as PROGMEM to keep RAM usage near zero!
    out.write(f"const uint8_t badapple_rle[{len(rle_bytes)}] PROGMEM = {{\n    ")
    for i, b in enumerate(rle_bytes):
        out.write(f"{b},")
        if (i + 1) % 16 == 0:
            out.write("\n    ")
    out.write("\n};\n")

print("Header generation complete!")
