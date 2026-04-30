# KILLER Portal BS1 v1.1

## Description
WiFi Portal Killer designed to detect and neutralize evil captive portals by overwhelming them with spam requests until they crash or become unusable.

## Features
- **Automatic Detection**: Scans for and targets ALL open WiFi networks
- **High-Speed Spamming**: Sends ~200 spam messages per minute to portal login pages
- **Portal Crashing**: Aggressive spam rate can crash poorly designed portals
- **Real-time Display**: Shows live attack status and target information
- **Button Controls**: Simple navigation with M5StickC Plus2 buttons

## How It Works
1. Scans WiFi environment for open networks
2. Connects to each open network sequentially 
3. Sends POST requests to common captive portal endpoints (192.168.4.1)
4. Spams with fake credentials: "Caught Ya Slippin" / "Ya Damn Fool"
5. Attempts to overwhelm and crash the portal system
6. Moves to next target automatically

## Usage
1. Select "COUNTER ATTACK" from main menu
2. Device automatically scans and finds targets
3. Begins systematic portal attacks
4. Monitor progress on screen
5. Press B button to exit anytime

## WARNING
- **Educational/Defensive Use Only**
- Only test on networks you own or have explicit permission
- Check local laws regarding WiFi security testing
- Tool is designed to protect users from evil portals

## Version History
- v1.1: Working portal killer with button controls and crash capability
- Target detection improved to find ANY open network
- Spam rate optimized for maximum effectiveness

## Hardware Required
- M5StickC Plus2 device
- No additional hardware needed