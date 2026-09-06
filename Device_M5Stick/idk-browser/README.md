# 🌐 idk-browser (M5Stick S3 Web Browser)

An interactive, resource-efficient HTML web browser designed specifically for the **M5Stick S3** (ESP32-S3) utilizing the **5-Way Tactile Switch** for sleek 2D navigation and scrolling.

---

## 🌟 Key Features

1. **📶 Interactive Wi-Fi Connection Manager**
   * Real-time local network scanning with visual signal strength bars.
   * Scrolling network selector using the 5-Way switch.
   * Custom **2D Grid Keyboard Overlay** (supports lower/uppercase CAPS, Space, Delete, and Back).

2. **🌐 Real HTTP & HTTPS Client Engine**
   * Capable of downloading real web pages over standard Wi-Fi.
   * Bypasses SSL certificate limitations gracefully using secure client overrides.
   * Elegant fetching screen showing micro-animations.

3. **📜 Smart HTML Tag Stripper & Text Renderer**
   * Custom parser that extracts title, strips structural script/style blocks, and processes line spacing.
   * Auto-formats HTML headers and generates nice bullet-points for list items (`<li>`).
   * Intelligent line-wrapping to prevent words clipping off the 135x240 display boundary.

4. **🎮 Dual Rendering Modes (Toggleable)**
   * **Web Render**: Nice formatted, easy-to-read text layouts.
   * **HTML Source**: Toggles viewing the raw downloaded HTML markup with special tag-coloring (excellent for developer inspect!).

---

## 🕹️ Controls (5-Way Tactile Switch & Pins)

The pins are mapped following the standard non-inverted D-pad setup:
- **GP1 (Physical UP)**: Moves UP in Wi-Fi list, Keyboard grid, and Web page viewer.
- **GP2 (Physical DOWN)**: Moves DOWN in Wi-Fi list, Keyboard grid, and Web page viewer.
- **GP3 (Physical LEFT)**: Moves LEFT in keyboard grid, and acts as **BACK/Exit** to go to the previous screen.
- **GP8 (Physical RIGHT)**: Moves RIGHT in keyboard grid, and **toggles raw HTML mode** in the web viewer.
- **GP43 (Physical CENTER)**: Nút Chọn / SELECT. Clocks or presses current selected character/URL/SSID.

---

## 🚀 How to Build & Flash

Open your terminal and run:
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-browser && pio run -e sticks3 -t upload
```
*PlatformIO will automatically download the necessary M5Unified libraries, compile the project, and upload the browser directly to your M5Stick S3 via USB.*
