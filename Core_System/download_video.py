#!/usr/bin/env python3
import os
import sys
import subprocess
import argparse

def check_dependencies():
    for cmd in ['yt-dlp', 'ffmpeg']:
        if subprocess.call(f"type {cmd}", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE) != 0:
            print(f"Error: '{cmd}' is not installed. Please install it first.")
            if cmd == 'yt-dlp':
                print("Install with: pip3 install yt-dlp")
            elif cmd == 'ffmpeg':
                print("Install with: sudo apt install ffmpeg")
            sys.exit(1)

def download_and_encode(url, output_name, portrait=False, folder="idk_videos"):
    # Ensure the output directory exists
    if not os.path.exists(folder):
        os.makedirs(folder)
        print(f"[*] Created folder: {folder}/")

    width, height = (172, 320) if portrait else (320, 172)
    
    # Define file paths inside the folder
    original_file = os.path.join(folder, f"{output_name}_original.mp4")
    encoded_file = os.path.join(folder, f"{output_name}.mjpeg")
    
    print(f"\n[*] Step 1: Downloading video from {url}...")
    
    # Download best video (ignoring audio)
    yt_cmd = [
        "yt-dlp",
        "-f", "bestvideo[ext=mp4]/best",
        "-o", original_file,
        url
    ]
    
    result = subprocess.run(yt_cmd)
    if result.returncode != 0:
        print("[!] Error downloading video.")
        sys.exit(1)

    print(f"\n[*] Step 2: Encoding video to MJPEG ({width}x{height}) at 30 FPS...")
    
    # FFmpeg command to scale/crop to exact dimensions, set 30fps, strip audio, and output as MJPEG
    ffmpeg_cmd = [
        "ffmpeg",
        "-y",                 # Overwrite output
        "-i", original_file,  # Input file
        "-an",                # No audio
        "-r", "30",           # 30 FPS
        "-c:v", "mjpeg",      # MJPEG codec
        "-q:v", "5",          # JPEG quality (2-31, lower is better. 5 is a good balance for ESP32)
        "-pix_fmt", "yuvj420p", # Standard pixel format for ESP32 decoders
        "-vf", f"scale={width}:{height}:force_original_aspect_ratio=decrease,pad={width}:{height}:(ow-iw)/2:(oh-ih)/2",
        encoded_file
    ]
    
    result = subprocess.run(ffmpeg_cmd)
    
    if result.returncode == 0:
        print(f"\n[+] Success! Both videos are saved in the '{folder}/' folder:")
        print(f"    - Original: {original_file}")
        print(f"    - Encoded for Device:  {encoded_file}")
        print(f"\n[+] You can now copy '{encoded_file}' to the root of your SD card.")
    else:
        print("\n[!] Error during ffmpeg encoding.")

if __name__ == "__main__":
    check_dependencies()
    
    parser = argparse.ArgumentParser(description="Download and encode videos for ESP32-S3 Waveshare 1.47")
    parser.add_argument("url", help="URL of the video (YouTube, TikTok, etc.)")
    parser.add_argument("output", help="Output filename base (e.g., 'my_video')")
    parser.add_argument("--portrait", action="store_true", help="Format for vertical viewing (172x320) instead of landscape (320x172)")
    parser.add_argument("--folder", default="idk_videos", help="Folder to save the videos in (default: 'idk_videos')")
    
    args = parser.parse_args()
    download_and_encode(args.url, args.output, args.portrait, args.folder)
