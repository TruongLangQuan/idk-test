import os
import shutil
import configparser

PROJECTS = [
    "idk-ascii-doom", "idk-ascii-idk", "idk-ascii-screensaver", "idk-screensaver",
    "idk-geometry-dash", "idk-sanctuaryrpg", "idk-motion-ascii", "idk-chess",
    "idk-dice", "idk-maze", "idk-draw", "idk-block-builder-3d", "idk-bitnet",
    "idk-english-wordform", "idk-miner", "idk-translator", "idk-ai", "idk-audio",
    "idk-worldgen", "idk-worldgen-2d", "idk-geogen-2d", "idk-geogen-multidim",
    "idk-gamerom", "idk-graph-2d3d",
    
    # M5StickC Original Projects
    "idk-gif", "idk-img", "idk-txt", "idk-video", "idk-ir", "idk-clock", 
    "idk-atom", "idk-blackhole", "idk-linux", "idk-portal", "idk-badapple"
]

ROOT_DIR = "/home/truonglangquan/idk-code/idk-test"
TEMPLATE_DIR = os.path.join(ROOT_DIR, "tools/templates")

def get_original_path(project):
    if project == "idk-graph-2d3d":
        return os.path.join(ROOT_DIR, "idk-math", "idk-graph-2d3d")
    return os.path.join(ROOT_DIR, project)

def parse_original_ini(original_ini_path):
    config = configparser.ConfigParser(strict=False, inline_comment_prefixes=';')
    try:
        config.read(original_ini_path)
    except Exception as e:
        print(f"Error reading {original_ini_path}: {e}")
        return None
    return config

def generate_sticks3_ini(project, orig_config):
    # Determine source path
    orig_path = "idk-math/idk-graph-2d3d" if project == "idk-graph-2d3d" else project
    
    # Base setup
    sticks3_ini = f"""[platformio]
src_dir = ../../{orig_path}/src

[env:sticks3]
platform = espressif32@6.9.0
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.arduino.partitions = default_8MB.csv
board_build.arduino.memory_type = qio_opi
lib_extra_dirs = ../../shared
"""

    # Extract lib_deps
    lib_deps = ["m5stack/M5Unified @ ^0.2.13"]
    build_flags = [
        "-DESP32S3",
        "-DSTICKS3",
        "-DBOARD_HAS_PSRAM",
        "-mfix-esp32-psram-cache-issue",
        "-DCORE_DEBUG_LEVEL=0",
        "-DARDUINO_USB_CDC_ON_BOOT=1",
        "-DARDUINO_USB_MODE=1",
        f"-I../../{orig_path}/include"
    ]
    
    for section in orig_config.sections():
        if section.startswith("env:"):
            # Check for extra lib_deps
            if orig_config.has_option(section, "lib_deps"):
                deps = orig_config.get(section, "lib_deps").strip().split("\n")
                for dep in deps:
                    dep = dep.strip()
                    if dep and "M5Unified" not in dep:
                        lib_deps.append(dep)
            # Check for custom build_flags
            if orig_config.has_option(section, "build_flags"):
                flags = orig_config.get(section, "build_flags").strip().split("\n")
                for flag in flags:
                    flag = flag.strip()
                    if flag and "-DCORE_DEBUG_LEVEL" not in flag:
                        if flag.startswith("-I") and not flag.startswith("-I/") and not flag.startswith("-I.."):
                            path = flag[2:]
                            build_flags.append(f"-I../../{orig_path}/{path}")
                        else:
                            build_flags.append(flag)

    # Format config strings
    lib_deps_str = "\n  ".join(lib_deps)
    build_flags_str = "\n  ".join(build_flags)
    
    sticks3_ini += f"build_flags =\n  {build_flags_str}\n"
    sticks3_ini += f"lib_deps =\n  {lib_deps_str}\n"
    return sticks3_ini

def generate_pcbfun_ini(project, orig_config):
    # Determine source path
    orig_path = "idk-math/idk-graph-2d3d" if project == "idk-graph-2d3d" else project
    
    # Base setup
    pcbfun_ini = f"""[platformio]
src_dir = ../../{orig_path}/src

[env:pcbfun_s3]
platform = espressif32@6.9.0
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.arduino.partitions = default_16MB.csv
board_build.arduino.memory_type = qio_opi
board_build.flash_size = 16MB
board_upload.flash_size = 16MB
lib_extra_dirs = ../../shared
"""

    # Extract lib_deps and custom flags
    lib_deps = ["lovyan03/LovyanGFX @ ^1.1.16"]
    build_flags = [
        "-DESP32S3",
        "-DPCBFUN",
        "-DBOARD_HAS_PSRAM",
        "-mfix-esp32-psram-cache-issue",
        "-DCORE_DEBUG_LEVEL=0",
        "-DARDUINO_USB_CDC_ON_BOOT=1",
        "-DARDUINO_USB_MODE=1",
        "-Iinclude",
        f"-I../../{orig_path}/include"
    ]
    
    for section in orig_config.sections():
        if section.startswith("env:"):
            # Check for extra lib_deps
            if orig_config.has_option(section, "lib_deps"):
                deps = orig_config.get(section, "lib_deps").strip().split("\n")
                for dep in deps:
                    dep = dep.strip()
                    if dep and "M5Unified" not in dep:
                        lib_deps.append(dep)
            # Check for custom build_flags
            if orig_config.has_option(section, "build_flags"):
                flags = orig_config.get(section, "build_flags").strip().split("\n")
                for flag in flags:
                    flag = flag.strip()
                    if flag and "-DCORE_DEBUG_LEVEL" not in flag:
                        if flag.startswith("-I") and not flag.startswith("-I/") and not flag.startswith("-I.."):
                            path = flag[2:]
                            build_flags.append(f"-I../../{orig_path}/{path}")
                        else:
                            build_flags.append(flag)

    # Format config strings
    lib_deps_str = "\n  ".join(lib_deps)
    build_flags_str = "\n  ".join(build_flags)
    
    pcbfun_ini += f"build_flags =\n  {build_flags_str}\n"
    pcbfun_ini += f"lib_deps =\n  {lib_deps_str}\n"
    return pcbfun_ini

def copy_hal_templates(dest_include_dir, dest_src_dir):
    os.makedirs(dest_include_dir, exist_ok=True)
    os.makedirs(dest_src_dir, exist_ok=True)
    
    # Copy M5Unified.h, M5GFX.h, idk_hal_pcbfun.h to include/
    shutil.copy2(os.path.join(TEMPLATE_DIR, "M5Unified.h"), dest_include_dir)
    shutil.copy2(os.path.join(TEMPLATE_DIR, "M5GFX.h"), dest_include_dir)
    shutil.copy2(os.path.join(TEMPLATE_DIR, "idk_hal_pcbfun.h"), dest_include_dir)
    
    # Copy idk_hal_pcbfun.cpp to src/
    shutil.copy2(os.path.join(TEMPLATE_DIR, "idk_hal_pcbfun.cpp"), dest_src_dir)

def main():
    print("=== Generating S3 Ports ===")
    
    sticks3_parent = os.path.join(ROOT_DIR, "firmware-sticks3")
    pcbfun_parent = os.path.join(ROOT_DIR, "firmware-pcbfun")
    
    os.makedirs(sticks3_parent, exist_ok=True)
    os.makedirs(pcbfun_parent, exist_ok=True)
    
    for project in PROJECTS:
        orig_path = get_original_path(project)
        orig_ini = os.path.join(orig_path, "platformio.ini")
        
        if not os.path.exists(orig_ini):
            print(f"Skipping {project}: no platformio.ini found.")
            continue
            
        orig_config = parse_original_ini(orig_ini)
        if not orig_config:
            continue
            
        # 1. Create M5StickS3 Port
        sticks3_dir = os.path.join(sticks3_parent, project)
        os.makedirs(sticks3_dir, exist_ok=True)
        sticks3_ini_content = generate_sticks3_ini(project, orig_config)
        with open(os.path.join(sticks3_dir, "platformio.ini"), "w") as f:
            f.write(sticks3_ini_content)
        print(f"Generated Sticks3 port for {project}")
            
        # 2. Create PCBFUN Port
        if project != "idk-audio":
            pcbfun_dir = os.path.join(pcbfun_parent, project)
            os.makedirs(pcbfun_dir, exist_ok=True)
            pcbfun_ini_content = generate_pcbfun_ini(project, orig_config)
            with open(os.path.join(pcbfun_dir, "platformio.ini"), "w") as f:
                f.write(pcbfun_ini_content)
                
            # Copy compatibility HAL files
            copy_hal_templates(
                os.path.join(pcbfun_dir, "include"),
                os.path.join(pcbfun_dir, "src")
            )
            print(f"Generated PCBFUN port for {project}")

    print("\nGeneration Complete!")

if __name__ == "__main__":
    main()
