import os
import sys
import subprocess

PROJECTS_TO_VERIFY = [
    "idk-screensaver",
    "idk-geometry-dash",
    "idk-maze",
    "idk-ai",
    "idk-audio"
]

ROOT_DIR = "/home/truonglangquan/idk-code/idk-test"

def run_build(proj, env, parent_folder):
    proj_dir = os.path.join(ROOT_DIR, parent_folder, proj)
    if not os.path.exists(proj_dir):
        print(f"[ERR] Directory {proj_dir} does not exist!")
        return False
        
    print(f"\n--- Building {proj} ({env}) ---")
    cmd = ["pio", "run", "-e", env]
    result = subprocess.run(cmd, cwd=proj_dir, capture_output=True, text=True)
    
    if result.returncode == 0:
        print(f"[OK] Build Succeeded for {proj} ({env})")
        return True
    else:
        print(f"[FAIL] Build FAILED for {proj} ({env})")
        # Print first few lines of error
        lines = result.stderr.split("\n")
        err_lines = [l for l in lines if "error:" in l or "failed" in l.lower()]
        for l in err_lines[:5]:
            print(f"  {l}")
        return False

def main():
    print("=== Port Verification Builder ===")
    
    # Allow passing custom lists
    targets = PROJECTS_TO_VERIFY
    if len(sys.argv) > 1:
        targets = sys.argv[1:]
        
    success = True
    
    for proj in targets:
        # Build StickS3 port
        ok_s3 = run_build(proj, "sticks3", "firmware-sticks3")
        # Build PCBFUN port (only if directory exists)
        pcb_dir = os.path.join(ROOT_DIR, "firmware-pcbfun", proj)
        if os.path.exists(pcb_dir):
            ok_pcb = run_build(proj, "pcbfun_s3", "firmware-pcbfun")
        else:
            print(f"[INFO] Skipping pcbfun_s3 build for {proj} (directory does not exist)")
            ok_pcb = True
        
        if not ok_s3 or not ok_pcb:
            success = False
            
    if success:
        print("\n[SUCCESS] All verified targets compiled successfully!")
        sys.exit(0)
    else:
        print("\n[ERR] Some compilation checks failed.")
        sys.exit(1)

if __name__ == "__main__":
    main()
