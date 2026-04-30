# M5StickVectrex v1.1 - Screen Rotation Update

## 🎮 New Features

### Screen Rotation Support
- **Hold BtnA for 2 seconds** to toggle screen orientation
- **Two modes**: Normal landscape ↔ Landscape 180° (upside down)
- **Left/Right handed support**: Flip the device however is comfortable!
- **Auto-rotating joystick**: Controls automatically adjust to screen orientation

### Why Only 2 Rotations (Not 4)?
The M5StickC Plus2 screen is 240×135 landscape. Portrait modes would require complex framebuffer transformations that cause rendering artifacts. The two landscape modes provide:
- ✅ Perfect image quality
- ✅ Full screen usage  
- ✅ Proper joystick rotation
- ✅ Left-handed or right-handed grip

## 📦 Files

```
M5StickVectrex-v1.1-4WAY_ROTATION-MERGED.bin  (1.2MB) ⭐ For M5Burner
M5StickVectrex-v1.1-4WAY_ROTATION.bin         (1.1MB) Firmware only
```

## 🕹️ Controls Update

### New Control: Screen Rotation
- **Hold BtnA for 2 seconds** - Toggle screen orientation
- Screen clears and game continues in new orientation
- Joystick automatically rotates with display

### All Controls (v1.1)
| Button | Function | Details |
|--------|----------|---------|
| **JoyC Stick** | Movement | Auto-rotates with screen |
| **JoyC Button** | Button 1 | Primary fire/action |
| **BtnA short** | Button 2 | Secondary action |
| **BtnA hold 2s** | System | **Toggle screen rotation** |
| **PWR** | Button 3 | Special action |
| **BtnB short** | Button 4 | Rare games |
| **BtnB hold 1s** | System | Return to menu |

## ⚙️ Technical Details

### Rotation Implementation
- Uses M5.Lcd.setRotation() hardware feature
- Rotations: 1 (normal) ↔ 3 (180°)
- Joystick X/Y values inverted in rotation 3
- Framebuffer stays 240×135 (no reallocation)

### Performance
- Same as v1.0 (no performance impact)
- 12-15 FPS typical gameplay
- Instant rotation switching

## 🔄 Changelog v1.1

### Added
- ✅ Screen rotation toggle (BtnA hold 2s)
- ✅ Landscape 180° support
- ✅ Auto-rotating joystick controls
- ✅ Screen clear on rotation

### Fixed
- ✅ Proper framebuffer dimensions for rotation
- ✅ Joystick mapping in upside-down mode

### Unchanged from v1.0
- ✅ All 71 ROMs still embedded
- ✅ Same performance and compatibility
- ✅ Menu and gameplay unchanged

## 📝 Upgrade from v1.0

Simply flash the new MERGED.bin with M5Burner. All your ROMs are embedded - no data loss!

## 🎯 Use Cases

### Left-Handed Mode
1. Hold device upside down
2. Hold BtnA for 2 seconds
3. Screen flips - now comfortable for left hand!

### Comfortable Grip
Find the orientation that works best for your hands and how you're holding the device.

## 🙏 Credits

- **Version:** 1.1 (November 6, 2025)
- **Developer:** coreymillia
- **AI Assistant:** GitHub Copilot CLI
- **Based on:** v1.0 with rotation enhancements

---

**Enjoy gaming in any orientation!** 🎮

*Made with ❤️ by coreymillia + GitHub Copilot CLI*
