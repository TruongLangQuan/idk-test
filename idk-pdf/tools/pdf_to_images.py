import fitz  # PyMuPDF
from PIL import Image
import sys
import os

def convert(pdf_path, out_dir, max_pages=10, width=600):
    os.makedirs(out_dir, exist_ok=True)
    doc = fitz.open(pdf_path)
    
    num_pages = min(len(doc), max_pages)
    print(f"Converting {num_pages} pages from {pdf_path}...")
    
    for i in range(num_pages):
        page = doc.load_page(i)
        # Calculate zoom factor to match target width
        zoom = width / page.rect.width
        mat = fitz.Matrix(zoom, zoom)
        
        pix = page.get_pixmap(matrix=mat, alpha=False)
        img = Image.frombytes("RGB", [pix.width, pix.height], pix.samples)
        
        out_path = os.path.join(out_dir, f"p{i}.jpg")
        # Save with medium quality to save space on SPIFFS
        img.save(out_path, "JPEG", quality=60, optimize=True)
        print(f"Saved {out_path} ({os.path.getsize(out_path)/1024:.1f} KB) - {pix.width}x{pix.height}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python pdf_to_images.py <pdf_path> <out_dir> [max_pages]")
        sys.exit(1)
    
    max_p = 10
    if len(sys.argv) > 3:
        max_p = int(sys.argv[3])
        
    convert(sys.argv[1], sys.argv[2], max_pages=max_p)
