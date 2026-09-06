import os
import shutil
import glob

# Create target directories
dirs = {
    "Device_Waveshare_1.47": [],
    "Device_Tenstar": [],
    "Device_Seeed_XIAO": [],
    "Device_CYD": [],
    "Device_M5Stick": [],
    "Device_ESP32S3_Generic": [],
    "Core_System": [],
    "Scripts": [],
    "Scripts/Patch_Scripts": [],
    "Scripts/Test_Scripts": [],
    "Scripts/Video_Processing": [],
    "Outputs": [],
    "Docs": []
}

for d in dirs:
    os.makedirs(d, exist_ok=True)

# Categorize
for item in os.listdir('.'):
    if item in dirs or item in ['.git', '.vscode', '.venv', '.venv-pio313', 'organize.py', '.gitignore', '.codegraph']:
        continue
    
    target = None
    
    if os.path.isfile(item):
        if item.startswith('patch_') and item.endswith('.py'):
            target = "Scripts/Patch_Scripts"
        elif item.startswith('test_') and item.endswith('.py'):
            target = "Scripts/Test_Scripts"
        elif item == 'process_video.py':
            target = "Scripts/Video_Processing"
        elif item.endswith('.md') or item.endswith('.txt'):
            target = "Docs"
        elif item.endswith('.json'):
            target = "Core_System"
        elif item == 'test_calc' or item == 'test_calc.cpp':
            target = "Outputs"
        else:
            target = "Core_System"
            
    elif os.path.isdir(item):
        name = item.lower()
        if 'ws-s3' in name or 'ws-1.47' in name or 'ws-e32s3-1.47' in name or 'launcher-ws-1.47' in name:
            target = "Device_Waveshare_1.47"
        elif 'ts-' in name or 'tenstar' in name:
            target = "Device_Tenstar"
        elif 'seeed' in name or 's3-lcd-147' in name or 's3-sd-5way' in name or 's3-147-hid' in name:
            target = "Device_Seeed_XIAO"
        elif 'cyd' in name:
            target = "Device_CYD"
        elif 's3-test' in name or 's3-ssh' in name or 's3-coop' in name or 'gif-s3' in name:
            target = "Device_ESP32S3_Generic"
        elif item in ['shared', 'tools', 'idk-corey']:
            target = "Core_System"
        elif item in ['idk-bin', 'idk_videos', 'mnt']:
            target = "Outputs"
        else:
            # Default to M5Stick (all the generic idk-math, idk-draw, etc.)
            target = "Device_M5Stick"
            
    if target:
        print(f"Moving {item} -> {target}/")
        shutil.move(item, os.path.join(target, item))

# Fix relative paths in all platformio.ini files
print("\nFixing relative paths in platformio.ini files...")
for root, _, files in os.walk('.'):
    if '.git' in root or '.venv' in root: continue
    for f in files:
        if f == 'platformio.ini':
            filepath = os.path.join(root, f)
            with open(filepath, 'r') as file:
                content = file.read()
                
            # Calculate how many levels deep we are to reach Core_System
            # e.g., ./Device_M5Stick/idk-math/platformio.ini
            # root is ./Device_M5Stick/idk-math (depth = 2)
            # We need to go up 2 levels, then into Core_System
            depth = len([p for p in root.split(os.sep) if p and p != '.'])
            up_path = '../' * depth
            core_path = up_path + 'Core_System/'
            
            # Replace '../shared' with proper path
            # We assume originally they were exactly 1 level down, so '../shared' was the root shared folder.
            # Now the root shared folder is at core_path + 'shared'
            if '../shared' in content or '../tools' in content:
                content = content.replace('../shared', core_path + 'shared')
                content = content.replace('../tools', core_path + 'tools')
                
                with open(filepath, 'w') as file:
                    file.write(content)
                print(f"Fixed paths in {filepath}")

print("Done!")
