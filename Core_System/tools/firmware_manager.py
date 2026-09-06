import os
import subprocess
import configparser
import sys
from pathlib import Path

class FirmwareTool:
    def __init__(self):
        self.workspace = os.getcwd()
        self.projects = []
        self.output_dir = os.path.join(self.workspace, "idk-bin")
        self.selected_projects = []

    def scan_projects(self):
        print("🔍 Đang quét các dự án PlatformIO...")
        self.projects = []
        for item in os.listdir(self.workspace):
            path = os.path.join(self.workspace, item)
            if os.path.isdir(path) and os.path.exists(os.path.join(path, "platformio.ini")):
                self.projects.append(item)
        self.projects.sort()

    def get_envs(self, project_path):
        config = configparser.ConfigParser()
        config.read(os.path.join(project_path, "platformio.ini"))
        envs = [section.split(":")[1] for section in config.sections() if section.startswith("env:")]
        return envs

    def select_projects(self):
        print("\n--- Danh sách dự án ---")
        for i, proj in enumerate(self.projects):
            print(f"{i+1}. {proj}")
        
        choices = input("\nChọn STT dự án (ví dụ: 1,3,5 hoặc 'all'): ").strip()
        if choices.lower() == 'all':
            selected_indices = range(len(self.projects))
        else:
            try:
                selected_indices = [int(x.strip()) - 1 for x in choices.split(",")]
            except ValueError:
                print("❌ Lựa chọn không hợp lệ.")
                return False

        self.selected_projects = []
        for idx in selected_indices:
            if 0 <= idx < len(self.projects):
                proj_name = self.projects[idx]
                proj_path = os.path.join(self.workspace, proj_name)
                envs = self.get_envs(proj_path)
                
                if not envs:
                    print(f"⚠️ Không tìm thấy environment cho {proj_name}")
                    continue
                
                print(f"\n--- Chọn Environment cho {proj_name} ---")
                for i, env in enumerate(envs):
                    print(f"{i+1}. {env}")
                env_choice = input(f"Chọn STT environment (mặc định 1): ").strip()
                env_idx = int(env_choice) - 1 if env_choice else 0
                selected_env = envs[env_idx] if 0 <= env_idx < len(envs) else envs[0]
                
                self.selected_projects.append({
                    "name": proj_name,
                    "path": proj_path,
                    "env": selected_env
                })
        return True

    def run_command(self, cmd, cwd):
        print(f"🚀 Chạy: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=cwd)
        return result.returncode == 0

    def get_offsets(self, project_path, env):
        # Mặc định cho M5StickC Plus 2 hoặc ESP32 tiêu chuẩn
        # Thường bootloader ở 0x1000, partitions ở 0x8000, app ở 0x10000
        # Nếu là S3/C3 có thể khác, nhưng đây là yêu cầu của user
        return {
            "boot": "0x1000",
            "part": "0x8000",
            "app": "0x10000"
        }

    def process(self):
        if not self.selected_projects:
            print("❌ Chưa chọn dự án nào.")
            return

        print(f"\n📁 Thư mục lưu file merge hiện tại: {self.output_dir}")
        change_dir = input("Bạn có muốn đổi thư mục lưu không? (y/N): ").lower()
        if change_dir == 'y':
            new_dir = input("Nhập đường dẫn thư mục mới: ").strip()
            if new_dir:
                self.output_dir = os.path.abspath(new_dir)
        
        os.makedirs(self.output_dir, exist_ok=True)

        print("\n--- Chọn hành động ---")
        print("1. Build")
        print("2. Merge")
        print("3. Flash (Upload)")
        print("4. Copy file Merge")
        print("5. Tất cả (Build -> Merge -> Copy)")
        
        action_choice = input("Chọn STT hành động (ví dụ: 1,2 hoặc 5): ").strip()
        actions = action_choice.split(",")

        for proj in self.selected_projects:
            print(f"\n========== XỬ LÝ: {proj['name']} ({proj['env']}) ==========")
            
            # Build
            if '1' in actions or '5' in actions:
                if not self.run_command(["pio", "run", "-e", proj["env"]], proj["path"]):
                    print(f"❌ Build thất bại cho {proj['name']}")
                    continue

            # Merge
            build_dir = os.path.join(proj["path"], ".pio", "build", proj["env"])
            out_file = os.path.join(self.output_dir, f"idk-{proj['name']}.bin")
            
            if '2' in actions or '5' in actions:
                offsets = self.get_offsets(proj["path"], proj["env"])
                merge_cmd = [
                    "esptool.py", "--chip", "esp32", "merge_bin",
                    "-o", out_file,
                    offsets["boot"], os.path.join(build_dir, "bootloader.bin"),
                    offsets["part"], os.path.join(build_dir, "partitions.bin"),
                    offsets["app"], os.path.join(build_dir, "firmware.bin")
                ]
                if not self.run_command(merge_cmd, proj["path"]):
                    print(f"❌ Merge thất bại cho {proj['name']}")

            # Flash
            if '3' in actions:
                # PlatformIO tự nhận diện port
                if not self.run_command(["pio", "run", "-t", "upload", "-e", proj["env"]], proj["path"]):
                    print(f"❌ Flash thất bại cho {proj['name']}")

            # Copy (Thực tế lệnh Merge đã xuất file ra output_dir, nhưng nếu user muốn copy từ đâu đó khác)
            if '4' in actions or '5' in actions:
                if os.path.exists(out_file):
                    print(f"✅ File đã sẵn sàng tại: {out_file}")
                else:
                    print(f"⚠️ Không tìm thấy file merge để xác nhận copy.")

    def run(self):
        self.scan_projects()
        if not self.projects:
            print("❌ Không tìm thấy dự án PlatformIO nào trong thư mục này.")
            return
        
        if self.select_projects():
            self.process()

if __name__ == "__main__":
    tool = FirmwareTool()
    tool.run()
