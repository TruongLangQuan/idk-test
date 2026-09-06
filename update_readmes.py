import os
import glob

repo_root = '.'
device_dirs = [d for d in os.listdir(repo_root) if os.path.isdir(d) and d.startswith('Device_')]

# Generate Root README
root_readme_content = """# idk-test - Universal Firmware Repository

This repository contains various firmware projects for different ESP32-based devices. The codebase has been organized into specific device folders to manage hardware variations and dependencies cleanly.

## 🗂️ Directory Structure

- **`Core_System/`** - Shared libraries, build tools, and common core code.
- **`Docs/`** - Project documentation, tasks, workflows, and hardware maps.
- **`Scripts/`** - Python scripts for patching, testing, and processing video/assets.
- **`Outputs/`** - Compiled binaries, output videos, and logs.

### 📱 Device Families
"""

for d in sorted(device_dirs):
    root_readme_content += f"\n### [{d}](./{d})\n"
    projects = [p for p in os.listdir(d) if os.path.isdir(os.path.join(d, p))]
    if not projects:
        root_readme_content += "*(Empty)*\n"
    for p in sorted(projects):
        root_readme_content += f"- `{p}`\n"

root_readme_content += """
## 🛠️ Building & Flashing

Each firmware is an independent PlatformIO project. To build and flash:

```bash
cd Device_Name/Project_Name
pio run -e firmware -t upload
pio device monitor -b 115200
```

*Note: All `platformio.ini` files have been automatically configured to resolve dependencies from the `Core_System/shared` and `Core_System/tools` folders.*
"""

with open('README.md', 'w', encoding='utf-8') as f:
    f.write(root_readme_content)

# Generate READMEs for each Device directory
for d in device_dirs:
    device_readme = f"# {d} Firmware Collection\n\nThis folder contains all firmware projects specifically configured for the **{d.replace('Device_', '')}** hardware.\n\n## Projects\n\n"
    
    projects = [p for p in os.listdir(d) if os.path.isdir(os.path.join(d, p))]
    for p in sorted(projects):
        device_readme += f"- **`{p}`**: [Open Project](./{p})\n"
        
    device_readme += "\n## Build Instructions\n```bash\ncd <project-name>\npio run -t upload\n```\n"
    
    with open(os.path.join(d, 'README.md'), 'w', encoding='utf-8') as f:
        f.write(device_readme)

if os.path.exists('Docs/README.md'):
    os.remove('Docs/README.md')

if os.path.exists('Docs/README copy.md'):
    os.remove('Docs/README copy.md')

print("Successfully updated root README.md and all Device README.md files!")
