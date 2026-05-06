import re

print("Parsing frames.h...")
with open("idk-badapple/src/frames.h", "r") as f:
    content = f.read()

# Extract the array content
match = re.search(r'const uint8_t badapple_data\[\]\[1056\] = \{(.*?)\};', content, re.DOTALL)
if not match:
    print("Could not find array data")
    exit(1)

data_str = match.group(1)

print("Writing to binary file...")
import os
os.makedirs("idk-badapple/data", exist_ok=True)

with open("idk-badapple/data/badapple.bin", "wb") as f_out:
    # Split by }, { or similar
    frames = data_str.split('}')
    for frame_str in frames:
        # Extract characters enclosed in single quotes
        chars = re.findall(r"'(.*?)'", frame_str)
        if len(chars) == 1056:
            # write them
            b = bytearray()
            for c in chars:
                if c == '\\\\': b.append(ord('\\'))
                elif c == "\\'": b.append(ord("'"))
                else: b.append(ord(c[0]))
            f_out.write(b)
        elif len(chars) > 0:
            print(f"Warning: Frame had {len(chars)} chars instead of 1056")

print("Done! badapple.bin generated in idk-badapple/data/")
