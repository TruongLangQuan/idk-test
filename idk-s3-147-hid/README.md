# 🎮 idk-s3-147-hid (USB OTG Macro Pad & controller)

A highly visual, premium **USB HID Controller & Macro Pad** designed specifically for the **PCBFun ESP32-S3 1.47-inch Display Board** (ST7789, 172x320 resolution). 

This firmware utilizes the ESP32-S3's native USB OTG peripheral to act as a plug-and-play **USB Keyboard, Mouse, and Media Controller** without needing any drivers on your PC.

---

## 🌟 Key Features

1. **📺 Beautiful Widescreen Glassmorphic Dashboard**
   * Uses double-buffered canvas rendering via `LovyanGFX` for zero flicker.
   * Sleek dark mode styling with electric cyan status elements.
   * Real-time **Input Monitor**: An on-screen D-pad and push button visualizer that lights up in green instantly when you click any button on your physical board!
   * Shows recently sent actions in active green text (e.g., `Sent: VOLUME UP`, `Sent: COPY`).

2. **🕹️ Four Multi-Functional Controller Modes**
   * **Media Controller Mode**: Seamlessly control music, movies, or browser volume.
   * **Mouse Emulator Mode**: Turn the 5-way D-pad into a smooth, analog-feeling joystick to move your mouse pointer on PC, with Left/Right click buttons!
   * **Slides Presenter Mode**: Navigate presentations (PowerPoint/Google Slides) with Arrow Keys, Page Up/Down, F5 (Start) and Esc (Exit).
   * **Quick Macros Mode**: Trigger standard OS shortcuts instantly: PC Lock (`Win+L`), Snipping Tool/Screenshot (`Win+Shift+S`), Copy (`Ctrl+C`), Paste (`Ctrl+V`), Undo (`Ctrl+Z`), and Run Prompt (`Win+R`).

---

## 🕹️ Controls & Pin Mapping

### 1. Mode Switching
* **BTN B (GP7)**: Click to cycle instantly through all 4 HID Modes (`Media -> Mouse -> Slides -> Macros`).

### 2. D-pad & Button Mapping Table
| Button | Media Mode | Mouse Mode | Presentation Mode | Quick Macros Mode |
|---|---|---|---|---|
| **UP (GP1)** | Volume Up | Move Pointer UP | Page Up | Lock Workstation (`Win+L`) |
| **DOWN (GP2)** | Volume Down | Move Pointer DOWN | Page Down | Take Screenshot (`Win+Shift+S`) |
| **LEFT (GP3)** | Previous Track | Move Pointer LEFT | Left Arrow | Copy (`Ctrl+C`) |
| **RIGHT (GP4)** | Next Track | Move Pointer RIGHT | Right Arrow | Paste (`Ctrl+V`) |
| **CENTER (GP5)** | Play / Pause | Left Click | Start Slideshow (`F5`) | Undo (`Ctrl+Z`) |
| **BTN A (GP6)** | Mute Toggle | Right Click | Exit Slideshow (`ESC`) | Open Run Box (`Win+R`) |
| **BTN B (GP7)** | *Next Mode* | *Next Mode* | *Next Mode* | *Next Mode* |

---

## 🚀 How to Build & Flash

Connect your **PCBFun ESP32-S3 1.47-inch Board** to your PC using the **USB OTG / USB** port.

Open a terminal and run the following PlatformIO command:
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-s3-147-hid && pio run -e esp32s3 -t upload
```

*PlatformIO will automatically compile the project under the standard ESP32-S3 framework, pull the LovyanGFX display library, and flash the HID Controller directly over USB. Once flashed, your PC will immediately recognize the board as a standard Keyboard/Mouse/Consumer Controller!*
