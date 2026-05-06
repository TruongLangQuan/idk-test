from PIL import Image, ImageOps, ImageFilter
import os
import sys

def convert_images(src_dir, dst_dir, target_w=135, target_h=240):
    if not os.path.exists(dst_dir):
        os.makedirs(dst_dir)
        
    files = sorted([f for f in os.listdir(src_dir) if f.lower().endswith(('.jpg', '.jpeg', '.png'))])
    
    print(f"Converting {len(files)} images to {target_w}x{target_h}...")
    
    for i, filename in enumerate(files):
        src_path = os.path.join(src_dir, filename)
        img = Image.open(src_path)
        
        # Calculate scaling to fit width while keeping aspect ratio
        img.thumbnail((target_w, target_h), Image.Resampling.LANCZOS)
        
        # Create a white background of target size
        new_img = Image.new("RGB", (target_w, target_h), (255, 255, 255))
        
        # Center the thumbnail
        offset = ((target_w - img.width) // 2, (target_h - img.height) // 2)
        new_img.paste(img, offset)
        
        # Apply a light sharpening filter to help text readability at small size
        new_img = new_img.filter(ImageFilter.SHARPEN)
        
        # Convert to a format suitable for the device (JPG with high quality)
        dst_path = os.path.join(dst_dir, f"p{i}.jpg")
        new_img.save(dst_path, "JPEG", quality=95, optimize=True)
        
        if i % 20 == 0:
            print(f"Processed {i}/{len(files)}...")

    print("Done!")

if __name__ == "__main__":
    src = "/home/truonglangquan/idk-code/idk-test/data-pdf/"
    dst = "/home/truonglangquan/idk-code/idk-test/idk-pdf/data/"
    convert_images(src, dst)
